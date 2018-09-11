#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>

using namespace Urho3D;

class ShapeBlaster : public Application
{
	URHO3D_OBJECT(ShapeBlaster, Application);
public:
	explicit ShapeBlaster(Context * context);
	void Setup() final;
	void Start() final;
	void Stop() final;
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleMouseButtons(StringHash eventType, VariantMap& eventData);
	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
	void HandleNodeBeginContact2D(StringHash eventType, VariantMap& eventData);
	void HandleTimerEvent(StringHash eventType, VariantMap& eventData);
	void HandleParticlesDuration(StringHash eventType, VariantMap& eventData);

private:
	SharedPtr<Scene> scene_;
	SharedPtr<Node> cameraNode_;
	SharedPtr<Text> gameOverText_;
	SharedPtr<Node> musicNode_;
	SharedPtr<SoundSource> musicSource_;
	SharedPtr<Sound> musicSound_;
	SharedPtr<Sound> defeatSound_;
	float halfWidth_;
	float halfHeight_;
	bool isShooting_;
	float shootingCooldownRemain_;
	PODVector<Node*> depletedParticleEmitters_;


	unsigned int highScore_;
	unsigned int score_;
	unsigned int lives_;
	bool gameOver_;

	void ConstructUI();
	void RestartGame();
	void MovePlayer(float timeStep);
	Vector2 GetAimDirection();
	Vector2 GetNormal(Vector2 v) { return Vector2(v.y_, -v.x_); };
	Vector2 GetRandomSpawnCoordinates();
	void ShootBullets(float timeStep);
	void SpawnPlayer();
	void SpawnWanderer();
	void SpawnSeeker();
	void SpawnBlackHole();
	void SpawnFourParticleEmitters(Vector2 pos, String pexName);
	void SpawnParticleEmitter(Node* node, String pexName);
	void KillPlayer();
	void CollectWanderer(Node* w);
	Node* ConstructEntity(const char* name,	const Vector2& position, const Vector2& direction, const char* spriteName,
		const float linearVelocity, const float angularVelocity, const int collisionCategory, const int collisionMask);
	void UpdateUI();
	void RemoveNodesWithComponent(StringHash type);
	void PlaySound(String soundName);
};
