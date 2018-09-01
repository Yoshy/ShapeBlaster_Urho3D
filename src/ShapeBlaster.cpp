#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/RigidBody2D.h>
#include <Urho3D/Urho2D/CollisionBox2D.h>
#include <Urho3D/Urho2D/PhysicsEvents2D.h>
#include <Urho3D/Urho2D/ParticleEffect2D.h>
#include <Urho3D/Urho2D/ParticleEmitter2D.h>
#include <Urho3D/Urho2D/Urho2DEvents.h>
#include <Urho3D/Audio/Audio.h>

#include "ShapeBlaster.h"
#include "GameDefs.h"
#include "ScreenInhabitant.h"
#include "PlayerHunter.h"
#include "PlayerEvader.h"
#include "Pulsar.h"
#include "Gravitator.h"
#include "MathUtils.h"
#include "TimerEvents.h"

using namespace Urho3D;

ShapeBlaster::ShapeBlaster(Context * context) : Application(context), isShooting_(false), highScore_(100), score_(0), lives_(NUM_LIVES), gameOver_(false)
{
	context->RegisterFactory<ScreenInhabitant>();
	context->RegisterFactory<PlayerHunter>();
	context->RegisterFactory<PlayerEvader>();
	context->RegisterFactory<Pulsar>();
	context->RegisterFactory<Gravitator>();
	context->RegisterSubsystem(new TimerEvents(context));
}

void ShapeBlaster::Setup()
{
	engineParameters_[EP_FULL_SCREEN] = false;
	engineParameters_[EP_WINDOW_WIDTH] = WINDOW_WIDTH;
	engineParameters_[EP_WINDOW_HEIGHT] = WINDOW_HEIGHT;
	engineParameters_[EP_WINDOW_RESIZABLE] = true;

	halfHeight_ = WINDOW_HEIGHT * 0.5f * PIXEL_SIZE;
	halfWidth_ = WINDOW_WIDTH * 0.5f * PIXEL_SIZE;
}

void ShapeBlaster::Start()
{
	// Setup window
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Graphics* graphics = GetSubsystem<Graphics>();
	Image* icon = cache->GetResource<Image>("Textures/UrhoIcon.png");
	graphics->SetWindowIcon(icon);
	graphics->SetWindowTitle("Shape Blaster");

	// Create scene
	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>();
	scene_->CreateComponent<DebugRenderer>();
	PhysicsWorld2D* phw = scene_->CreateComponent<PhysicsWorld2D>();
	phw->SetGravity(Vector2()); // There is no gravity in space

	cameraNode_ = scene_->CreateChild("Camera");
	cameraNode_->SetPosition(Vector3(0.0f, 0.0f, -1.0f));
	Camera* camera{ cameraNode_->CreateComponent<Camera>() };
	camera->SetOrthographic(true);
	camera->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);

	gameOverText_ = GetSubsystem<UI>()->GetRoot()->CreateChild<Text>();
	gameOverText_->SetHorizontalAlignment(HA_CENTER);
	gameOverText_->SetVerticalAlignment(VA_CENTER);
	gameOverText_->SetTextAlignment(HA_CENTER);
	gameOverText_->SetText("Game over.\nPress ESC to exit\nPress SPACE to restart");
	gameOverText_->SetFont(GetSubsystem<ResourceCache>()->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);

	// Setup audio
	musicSound_ = cache->GetResource<Sound>("audio/music.ogg");
	musicSound_->SetLooped(true);
	defeatSound_ = cache->GetResource<Sound>("audio/defeat.ogg");
	musicNode_ = new Node(context_);
	musicSource_ = musicNode_->CreateComponent<SoundSource>();
	musicSource_->SetSoundType(SOUND_MUSIC);
	GetSubsystem<Audio>()->SetMasterGain(SOUND_MUSIC, 1.0);
	GetSubsystem<Audio>()->SetMasterGain(SOUND_EFFECT, 0.6);

	// Setup viewport & post-render effects
	Renderer* renderer{ GetSubsystem<Renderer>() };
	SharedPtr<Viewport> viewport{ new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()) };

	SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
	effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
	effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
	// Make the bloom mixing parameter more pronounced
	effectRenderPath->SetShaderParameter("BloomMix", Vector2(1.5f, 1.5f));
	effectRenderPath->SetEnabled("Bloom", true);
	effectRenderPath->SetEnabled("FXAA2", true);
	viewport->SetRenderPath(effectRenderPath);
	renderer->SetViewport(0, viewport);

	// Setup UI
	ConstructUI();

	// Subscribe to events
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ShapeBlaster, HandleKeyDown));
	SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(ShapeBlaster, HandleMouseButtons));
	SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(ShapeBlaster, HandleMouseButtons));
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ShapeBlaster, HandleUpdate));
	SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(ShapeBlaster, HandlePostRenderUpdate));
	SubscribeToEvent(E_NODEBEGINCONTACT2D, URHO3D_HANDLER(ShapeBlaster, HandleNodeBeginContact2D));
	SubscribeToEvent(E_PARTICLESDURATION, URHO3D_HANDLER(ShapeBlaster, HandleParticlesDuration));

	SubscribeToEvent(E_TIMEREVENT, URHO3D_HANDLER(ShapeBlaster, HandleTimerEvent));
	
	TimerEvents* timerEvents = GetSubsystem<TimerEvents>();
	timerEvents->AddTimerEvent(WANDERER_RESPAWN_TIMER, WANDERER_SPAWN_INTERVAL, 0);
	timerEvents->AddTimerEvent(SEEKER_RESPAWN_TIMER, SEEKER_SPAWN_INTERVAL, 0);
	timerEvents->AddTimerEvent(BLACKHOLE_RESPAWN_TIMER, BLACKHOLE_SPAWN_INTERVAL, 0);
	timerEvents->AddTimerEvent(RECYCLE_DEPLETED_PARTICLE_EMITTERS_TIMER, 1.0f, 0);
	gameOver_ = true;
}

void ShapeBlaster::Stop()
{
}

void ShapeBlaster::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	float timeStep{ eventData[Update::P_TIMESTEP].GetFloat() };
	MovePlayer(timeStep);
	if (isShooting_)
		ShootBullets(timeStep);
}

void ShapeBlaster::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	int key = eventData[KeyDown::P_KEY].GetInt();
	if (key == KEY_ESCAPE)
	{
		engine_->Exit();
	}
	if (key == KEY_SPACE)
	{
		Node* player = scene_->GetChild(PLAYER_NODE_NAME);
		if (!player)
			SpawnPlayer();
		if (gameOver_)
			RestartGame();
	}
}

void ShapeBlaster::HandleMouseButtons(StringHash eventType, VariantMap& eventData)
{
	if (eventType == E_MOUSEBUTTONDOWN && eventData[MouseButtonDown::P_BUTTON].GetInt() == MOUSEB_LEFT)
		isShooting_ = true;
	if (eventType == E_MOUSEBUTTONUP && eventData[MouseButtonUp::P_BUTTON].GetInt() == MOUSEB_LEFT)
		isShooting_ = false;
}

void ShapeBlaster::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	//DebugRenderer* dr = scene_->GetComponent<DebugRenderer>();
	//scene_->GetComponent<PhysicsWorld2D>()->DrawDebugGeometry(dr, false);
}

void ShapeBlaster::HandleNodeBeginContact2D(StringHash eventType, VariantMap& eventData)
{
	Node* myNode{ static_cast<RigidBody2D*>(eventData[NodeBeginContact2D::P_BODY].GetPtr())->GetNode() };
	const String& myName{ myNode->GetName() };
	Node* other = (Node*)(eventData[NodeBeginContact2D::P_OTHERNODE].GetPtr());
	if (myName == String{ PLAYER_NODE_NAME })
	{
		if (other->GetName() == String{ SEEKER_NODE_NAME } && other->GetVar(VAR_SEEKER_INACTIVITY_TIMER).GetFloat() <= 0)
			KillPlayer();
		if (other->GetName() == String{ WANDERER_NODE_NAME })
			CollectWanderer(other);
		if (other->GetName() == String{ BLACKHOLE_NODE_NAME })
			KillPlayer();
	}
	if (myName == String{ BULLET_NODE_NAME })
	{
		if (other->GetName() == String{ WANDERER_NODE_NAME })
		{
			PlaySound("audio/explosion-01.wav");
			myNode->Remove();
			other->Remove();
			SpawnFourParticleEmitters(other->GetPosition2D(), "Urho2D/ExplodeBlue.pex");
		}
		if (other->GetName() == String{ SEEKER_NODE_NAME } && other->GetVar(VAR_SEEKER_INACTIVITY_TIMER).GetFloat() <= 0)
		{
			PlaySound("audio/explosion-02.wav");
			myNode->Remove();
			other->Remove();
			SpawnFourParticleEmitters(other->GetPosition2D(), "Urho2D/ExplodeYel.pex");
		}
		if (other->GetName() == String{ BLACKHOLE_NODE_NAME })
		{
			myNode->Remove();
			unsigned int HP{ other->GetVar(VAR_BLACKHOLE_HP).GetUInt() };
			HP--;
			if (HP == 0)
			{
				PlaySound("audio/explosion-04.wav");
				SpawnFourParticleEmitters(other->GetPosition2D(), "Urho2D/ExplodeGreen.pex");
				other->Remove();
			}
			other->SetVar(VAR_BLACKHOLE_HP, HP);
		}
	}
}

void ShapeBlaster::HandleTimerEvent(StringHash eventType, VariantMap& eventData)
{
	StringHash id{ eventData[TimerEvent::P_TIMER_ID].GetUInt() };
	if (id == WANDERER_RESPAWN_TIMER)
		SpawnWanderer();
	if (id == SEEKER_RESPAWN_TIMER && scene_->GetChild(PLAYER_NODE_NAME)) // Spawn seekers only when player is spawned
		SpawnSeeker();
	if (id == BLACKHOLE_RESPAWN_TIMER && scene_->GetChild(PLAYER_NODE_NAME)) // Spawn black holes only when player is spawned
		SpawnBlackHole();
	if (id == RECYCLE_DEPLETED_PARTICLE_EMITTERS_TIMER)
	{
		for (unsigned int i = 0; i < depletedParticleEmitters_.Size(); ++i)
		{
			Node* n = depletedParticleEmitters_[i];
			Variant v = n->GetVar(StringHash("canRemove")); // Check if node is flagged for removing
			if (v == Variant::EMPTY)
			{
				// Set a flag on the node. On the next timer tick this node will be removed.
				n->SetVar(StringHash("canRemove"), true);
			}
			else
			{
				// As the timer interval vastly greater than the particles life, removing it will not be visible to the player (all particles of this emitter already faded to void)
				n->Remove();
				depletedParticleEmitters_.Erase(i); // FIXME: Maybe we need to use something more performant than PODVector
			}
		}
	}
}

void ShapeBlaster::HandleParticlesDuration(StringHash eventType, VariantMap& eventData)
{
	Node* node = (Node*)eventData[ParticlesDuration::P_NODE].GetPtr();
	depletedParticleEmitters_.Push(node);
}

void ShapeBlaster::ConstructUI()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();

	// Set mouse cursor
	Image* cursorImage{ cache->GetResource<Image>("sprites/Pointer.png") };
	SharedPtr<Cursor> cursor{ new Cursor(context_) };
	cursor->DefineShape(Urho3D::CursorShape::CS_NORMAL, cursorImage, IntRect(0, 0, cursorImage->GetWidth(), cursorImage->GetHeight()), IntVector2(1, 1));
	cursor->SetPosition((int)halfWidth_, (int)halfHeight_);
	ui->SetCursor(cursor);

	Window* rWin = new Window(context_);
	ui->GetRoot()->AddChild(rWin);
	rWin->SetAlignment(HA_RIGHT, VA_TOP);
	rWin->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	rWin->SetName("Window");
	rWin->SetColor(Color(0.f, 0.f, 0.f, 0.f));

	Text* scoreText{ rWin->CreateChild<Text>(UI_SCORES_TEXT) };
	scoreText->SetText(String("Score: ") + String(score_));
	scoreText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
	scoreText->SetHorizontalAlignment(HA_LEFT);
	scoreText->SetVerticalAlignment(VA_CENTER);

	Text* highScoreText{ rWin->CreateChild<Text>(UI_HSCORES_TEXT) };
	highScoreText->SetText(String("High score: ") + String(highScore_));
	highScoreText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
	highScoreText->SetHorizontalAlignment(HA_LEFT);
	highScoreText->SetVerticalAlignment(VA_CENTER);

	Window* lWin = new Window(context_);
	ui->GetRoot()->AddChild(lWin);
	lWin->SetAlignment(HA_LEFT, VA_TOP);
	lWin->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	lWin->SetName("Window");
	lWin->SetColor(Color(0.f, 0.f, 0.f, 0.f));

	Text* livesText{ lWin->CreateChild<Text>(UI_LIVES_TEXT) };
	livesText->SetText(String("Lives: ") + String(lives_));
	livesText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
	livesText->SetHorizontalAlignment(HA_CENTER);
	livesText->SetVerticalAlignment(VA_CENTER);
}

void ShapeBlaster::RestartGame()
{
	isShooting_ = false;
	if (score_ > highScore_)
	{
		highScore_ = score_;
	}
	score_ = 0;
	UpdateUI();
	lives_ = NUM_LIVES;
	RemoveNodesWithComponent(StringHash("ScreenInhabitant"));
	gameOver_ = false;
	gameOverText_->SetVisible(false);
	musicSource_->Play(musicSound_);
	SpawnPlayer();
}

void ShapeBlaster::MovePlayer(float timeStep)
{
	Node* playerNode = scene_->GetChild(PLAYER_NODE_NAME);
	if (!playerNode)
		return;

	static const Vector2 DIRECTION_N{ 0.0f, 1.0f };
	static const Vector2 DIRECTION_S{ 0.0f, -1.0f };
	static const Vector2 DIRECTION_E{ 1.0f, 0.0f };
	static const Vector2 DIRECTION_W{ -1.0f, -.0f };

	Vector2 playerVelV;
	Vector2 playerVelH;
	bool isMoving{ false };

	Input* input{ GetSubsystem<Input>() };

	if (input->GetKeyDown(KEY_W))
	{
		playerVelV = DIRECTION_N;
		isMoving = true;
	}
		
	if (input->GetKeyDown(KEY_S))
	{
		playerVelV = DIRECTION_S;
		isMoving = true;
	}
		
	if (input->GetKeyDown(KEY_A))
	{
		playerVelH = DIRECTION_W;
		isMoving = true;
	}
	if (input->GetKeyDown(KEY_D))
	{
		playerVelH = DIRECTION_E;
		isMoving = true;
	}
	if (isMoving)
	{
		Vector2 playerVelocity = playerVelV + playerVelH;
		playerNode->GetComponent<RigidBody2D>()->SetLinearVelocity(playerVelocity * PLAYER_MOVE_SPEED);
		playerNode->SetRotation2D(GetFullAngle(playerVelocity));
		playerNode->GetChild("ShipExhaust")->SetEnabled(true);
	}
	else
	{
		// Damp player speed
		Vector2 curVel{ playerNode->GetComponent<RigidBody2D>()->GetLinearVelocity() };
		playerNode->GetComponent<RigidBody2D>()->SetLinearVelocity(curVel * 0.98f);
		// Switch engine off
		playerNode->GetChild("ShipExhaust")->SetEnabled(false);
	}

	Vector2 curPos{ playerNode->GetPosition2D() };
	if (curPos.x_ > halfWidth_) curPos.x_ = halfWidth_;
	if (curPos.x_ < -halfWidth_) curPos.x_ = -halfWidth_;
	if (curPos.y_ > halfHeight_) curPos.y_ = halfHeight_;
	if (curPos.y_ < -halfHeight_) curPos.y_ = -halfHeight_;
	playerNode->SetPosition2D(curPos);
}

Vector2 ShapeBlaster::GetAimDirection()
{
	Node* playerNode = scene_->GetChild(PLAYER_NODE_NAME);
	if (!playerNode)
		return Vector2();

	Input* input{ GetSubsystem<Input>() };
	Vector2 mousePosition{ Vector2(GetSubsystem<UI>()->GetCursorPosition()) }; // viewport coordinates
	mousePosition = Vector2(mousePosition.x_ * PIXEL_SIZE - halfWidth_, -mousePosition.y_ * PIXEL_SIZE + halfHeight_);
	Vector2 playerPosition{ playerNode->GetPosition2D() };
	Vector2 direction{ Vector2(mousePosition) - playerPosition };
	direction.Normalize();
	return direction;
}

Vector2 ShapeBlaster::GetRandomSpawnCoordinates()
{
	Node* playerNode = scene_->GetChild(PLAYER_NODE_NAME);
	Vector2 playerPosition;
	if (playerNode)
	{
		playerPosition = playerNode->GetPosition2D();
	}
	Vector2 spawnPosition;
	while (true)
	{
		spawnPosition.x_ = Random(halfWidth_ * 2.f) - halfWidth_;
		spawnPosition.y_ = Random(halfHeight_ * 2.f) - halfHeight_;
		if (!playerNode)
			return spawnPosition;
		Vector2 distanceToPlayer{ spawnPosition - playerPosition };
		if (distanceToPlayer.Length() > halfWidth_ / 2)
			break;
	}
	return spawnPosition;
}

void ShapeBlaster::ShootBullets(float timeStep)
{
	Node* playerNode = scene_->GetChild(PLAYER_NODE_NAME);
	if (!playerNode)
		return;

	if (shootingCooldownRemain_ > 0)
	{
		shootingCooldownRemain_ -= timeStep;
		shootingCooldownRemain_ = Clamp(shootingCooldownRemain_, 0.0f, SHOOT_COOLDOWN);
		return;
	}
	shootingCooldownRemain_ = SHOOT_COOLDOWN;

	Vector2 aim{ GetAimDirection() };
	Vector2 offsetN{ GetNormal(aim) * PIXEL_SIZE * 10.0f };
	Vector2 offsetP{ aim * PIXEL_SIZE * 40.0f };
	Vector2 position = playerNode->GetPosition2D();
	
	// Slightly randomize shoot direction
	float redirAngle = (Random() * SHOOT_GROUPING_SIZE) - (SHOOT_GROUPING_SIZE / 2.0f);
	aim.x_ = aim.x_ * Cos(redirAngle) - aim.y_ * Sin(redirAngle);
	aim.y_ = aim.x_ * Sin(redirAngle) + aim.y_ * Cos(redirAngle);

	Node* bulletNode = ConstructEntity(BULLET_NODE_NAME, position, aim, "sprites/Bullet.png", BULLET_MOVE_SPEED, 0, 0x0001, 0xFFFF);
	bulletNode->Translate2D(offsetN + offsetP, TS_WORLD);
	bulletNode = ConstructEntity(BULLET_NODE_NAME, position, aim, "sprites/Bullet.png", BULLET_MOVE_SPEED, 0, 0x0001, 0xFFFF);
	bulletNode->Translate2D(-offsetN + offsetP, TS_WORLD);
	PlaySound("audio/shoot-01.wav");
}

void ShapeBlaster::SpawnPlayer()
{
	if (gameOver_)
		return;
	PlaySound("audio/spawn-01.wav");
	Vector2 playerPos{ Vector2(0.0f, 0.0f) };
	PODVector<Node*> seekers = scene_->GetChildrenWithComponent(StringHash("PlayerHunter"));
	for (auto i = seekers.Begin(); i != seekers.End(); i++)
	{
		Vector2 seekerPos{ (*i)->GetPosition2D() };
		Vector2 safeDistance{ seekerPos - playerPos };
		if (safeDistance.Length() < WANDERER_EVADE_RADIUS * 2)
			(*i)->Remove();
	}
	Node* playerNode = ConstructEntity(PLAYER_NODE_NAME, playerPos, Vector2(1.0f, 0.0f), "sprites/Player.png", 0, 0, 0x0001, 0xFFFF);
	ParticleEffect2D* particleEffect{ GetSubsystem<ResourceCache>()->GetResource<ParticleEffect2D>("Urho2D/shipExhaust.pex") };
	if (particleEffect)
	{
		Node* exhaustNode = playerNode->CreateChild("ShipExhaust");
		exhaustNode->SetPosition2D(Vector2(-0.25f, 0.0f));
		ParticleEmitter2D* particleEmitter{ exhaustNode->CreateComponent<ParticleEmitter2D>() };
		particleEmitter->SetEffect(particleEffect);
		exhaustNode->SetEnabled(false);
	}
}

void ShapeBlaster::SpawnWanderer()
{
	PlaySound("audio/spawn-02.wav");
	Vector2 spawnPosition{ GetRandomSpawnCoordinates() };
	Vector2 spawnDirection{ Vector2{ 0.f, 0.f } - spawnPosition };
	spawnDirection.Normalize();
	Node* seekerNode = ConstructEntity(WANDERER_NODE_NAME, spawnPosition, spawnDirection, "sprites/Wanderer.png", WANDERER_MOVE_SPEED, WANDERER_ROLL_SPEED, 0x0001, 0xFFFF);
	seekerNode->CreateComponent<PlayerEvader>();
}

void ShapeBlaster::SpawnSeeker()
{
	PlaySound("audio/spawn-03.wav");
	Vector2 spawnPosition{ GetRandomSpawnCoordinates() };
	Vector2 spawnDirection{ Vector2{ 0.f, 0.f } - spawnPosition };
	spawnDirection.Normalize();
	Node* seekerNode = ConstructEntity(SEEKER_NODE_NAME, spawnPosition, spawnDirection, "sprites/Seeker.png", 0, 0, 0x0001, 0xFFFF);
	seekerNode->CreateComponent<PlayerHunter>();
	seekerNode->GetComponent<StaticSprite2D>()->SetColor(Color::GREEN);
	seekerNode->SetVar(VAR_SEEKER_INACTIVITY_TIMER, SEEKER_INACTIVITY_PERIOD);
	ParticleEffect2D* particleEffect{ GetSubsystem<ResourceCache>()->GetResource<ParticleEffect2D>("Urho2D/shipExhaust.pex") };
	if (particleEffect)
	{
		Node* exhaustNode = seekerNode->CreateChild("ShipExhaust");
		exhaustNode->SetPosition2D(Vector2(-0.25f, 0.0f));
		ParticleEmitter2D* particleEmitter{ exhaustNode->CreateComponent<ParticleEmitter2D>() };
		particleEmitter->SetEffect(particleEffect);
		exhaustNode->SetEnabled(false);
	}
}

void ShapeBlaster::SpawnBlackHole()
{
	PODVector<Node*> holes = scene_->GetChildrenWithComponent(StringHash("Pulsar"));
	if (holes.Size() >= 3) // Do not spawn more than three black holes
		return;
	PlaySound("audio/spawn-04.wav");
	Vector2 spawnPosition{ GetRandomSpawnCoordinates() };
	Vector2 spawnDirection{ Vector2{ 0.f, 0.f } - spawnPosition };
	spawnDirection.Normalize();
	Node* BHNode = ConstructEntity(BLACKHOLE_NODE_NAME, spawnPosition, spawnDirection, "sprites/Black Hole.png", 0, BLACKHOLE_ROLL_SPEED, 0x0001, 0xFFFF);
	BHNode->CreateComponent<Pulsar>();
	BHNode->CreateComponent<Gravitator>();
	BHNode->SetVar(VAR_BLACKHOLE_HP, BLACKHOLE_HP);
	SpawnParticleEmitter(BHNode, "Urho2D/BlackHole2.pex");
}

void ShapeBlaster::SpawnFourParticleEmitters(Vector2 pos, String pexName)
{
	ParticleEffect2D* particleEffect{ GetSubsystem<ResourceCache>()->GetResource<ParticleEffect2D>(pexName) };
	if (particleEffect)
	{
		Node* particlesNode;
		ParticleEmitter2D* particleEmitter;
		for (int i = 0; i < 4; ++i)
		{
			particlesNode = scene_->CreateChild(pexName);
			particlesNode->SetPosition2D(pos + Vector2(Random() * 0.4f - 0.2f, Random() * 0.4f - 0.2f));
			particleEmitter = particlesNode->CreateComponent<ParticleEmitter2D>();
			particleEmitter->SetEffect(particleEffect);
		}
	}
}

void ShapeBlaster::SpawnParticleEmitter(Node* node, String pexName)
{
	ParticleEffect2D* particleEffect{ GetSubsystem<ResourceCache>()->GetResource<ParticleEffect2D>(pexName) };
	if (particleEffect)
	{
		ParticleEmitter2D* particleEmitter;
		particleEmitter = node->CreateComponent<ParticleEmitter2D>();
		particleEmitter->SetEffect(particleEffect);
	}
}

void ShapeBlaster::KillPlayer()
{
	PlaySound("audio/explosion-03.wav");
	Vector2 pos{ scene_->GetChild(PLAYER_NODE_NAME)->GetPosition2D() };
	float offset = 0.3f;
	SpawnFourParticleEmitters(pos + Vector2{ offset, offset }, "Urho2D/ExplodeYel.pex");
	SpawnFourParticleEmitters(pos + Vector2{ -offset, offset }, "Urho2D/ExplodeBlue.pex");
	SpawnFourParticleEmitters(pos + Vector2{ offset, -offset }, "Urho2D/ExplodeGreen.pex");
	SpawnFourParticleEmitters(pos + Vector2{ -offset, -offset }, "Urho2D/ExplodeRed.pex");
	scene_->GetChild(PLAYER_NODE_NAME)->Remove();
	lives_--;
	UpdateUI();
	if (!lives_)
		gameOver_ = true;
	if (gameOver_)
	{
		musicSource_->Play(defeatSound_);
		gameOverText_->SetVisible(true);
	}
}

void ShapeBlaster::CollectWanderer(Node* w)
{
	w->Remove();
	score_ += SCORES_FOR_WANDERER;
	if (score_ > highScore_)
		highScore_ = score_;
	UpdateUI();
}

Node* ShapeBlaster::ConstructEntity(const char* name, const Vector2& position, const Vector2& direction, const char* spriteName,
	const float linearVelocity, const float angularVelocity, const int collisionCategory, const int collisionMask)
{
	Node* node = scene_->CreateChild(name);
	node->SetPosition2D(position);
	node->SetRotation2D(GetFullAngle(direction));

	StaticSprite2D* staticSprite2D{ node->CreateComponent<StaticSprite2D>() };
	Sprite2D* sprite{ GetSubsystem<ResourceCache>()->GetResource<Sprite2D>(spriteName) };
	staticSprite2D->SetSprite(sprite);

	RigidBody2D* body = node->CreateComponent<RigidBody2D>();
	body->SetBodyType(BT_DYNAMIC);
	body->SetLinearVelocity(direction * linearVelocity);
	body->SetAngularVelocity(angularVelocity);
	CollisionBox2D* cb = node->CreateComponent<CollisionBox2D>();
	float sx = staticSprite2D->GetWorldBoundingBox().Size().x_;
	float sy = staticSprite2D->GetWorldBoundingBox().Size().y_;
	cb->SetSize(Vector2(sx, sy));
	cb->SetCategoryBits(collisionCategory);
	cb->SetMaskBits(collisionMask);
	cb->SetTrigger(true);
	node->CreateComponent<ScreenInhabitant>();

	return node;
}

void ShapeBlaster::UpdateUI()
{
	GetSubsystem<UI>()->GetRoot()->GetChildStaticCast<Text>(UI_LIVES_TEXT, true)->SetText(String("Lives: ") + String(lives_));
	GetSubsystem<UI>()->GetRoot()->GetChildStaticCast<Text>(UI_SCORES_TEXT, true)->SetText(String("Score: ") + String(score_));
	GetSubsystem<UI>()->GetRoot()->GetChildStaticCast<Text>(UI_HSCORES_TEXT, true)->SetText(String("High score: ") + String(highScore_));
}

void ShapeBlaster::RemoveNodesWithComponent(StringHash type)
{
	PODVector<Node*> destNodes = scene_->GetChildrenWithComponent(type, true);
	for (auto i = destNodes.Begin(); i != destNodes.End(); i++)
		(*i)->Remove();
}

void ShapeBlaster::PlaySound(String soundName)
{
	Node* node{ scene_->CreateChild(soundName) };
	Sound* snd{ GetSubsystem<ResourceCache>()->GetResource<Sound>(soundName) };
	SoundSource* soundSource{ node->CreateComponent<SoundSource>() };
	soundSource->SetSoundType(SOUND_EFFECT);
	soundSource->Play(snd);
	soundSource->SetAutoRemoveMode(REMOVE_NODE);
}