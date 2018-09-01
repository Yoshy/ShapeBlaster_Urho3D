#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Urho2D/RigidBody2D.h>
#include "PlayerEvader.h"
#include "GameDefs.h"
#include "MathUtils.h"

using namespace Urho3D;

PlayerEvader::PlayerEvader(Context* context) : LogicComponent(context)
{
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void PlayerEvader::Update(float timeStep)
{
	Node* playerNode = node_->GetScene()->GetChild(PLAYER_NODE_NAME);
	Vector2 playerPos;
	if (playerNode)
		playerPos = playerNode->GetPosition2D();

	Vector2 myPos{ node_->GetPosition2D() };
	Vector2 evadeDirection{ myPos - playerPos };
	if (evadeDirection.Length() > WANDERER_EVADE_RADIUS || !playerNode)
		return;

	evadeDirection.Normalize();
	node_->SetRotation2D(GetFullAngle(evadeDirection));
	RigidBody2D* body = node_->GetComponent<RigidBody2D>();
	if (body)
		body->SetLinearVelocity(evadeDirection * WANDERER_MOVE_SPEED);
}
