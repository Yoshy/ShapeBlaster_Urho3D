#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Urho2D/RigidBody2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include "PlayerHunter.h"
#include "GameDefs.h"
#include "MathUtils.h"

using namespace Urho3D;

PlayerHunter::PlayerHunter(Context* context) : LogicComponent(context)
{
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void PlayerHunter::Update(float timeStep)
{
	float inactTimer = node_->GetVar(VAR_SEEKER_INACTIVITY_TIMER).GetFloat();
	if (inactTimer > 0)
	{
		inactTimer -= timeStep;
		node_->SetVar(VAR_SEEKER_INACTIVITY_TIMER, inactTimer);

		float fadeAmount = 1 - inactTimer / SEEKER_INACTIVITY_PERIOD;
		Color fromColor = Color::GREEN;
		fromColor.a_ = 0.0f;
		Color c = fadeColor(fromColor, Color::WHITE, fadeAmount);
		node_->GetComponent<StaticSprite2D>()->SetColor(c);
		return;
	}
	RigidBody2D* body = node_->GetComponent<RigidBody2D>();
	Node* playerNode = node_->GetScene()->GetChild(PLAYER_NODE_NAME);
	if (!playerNode)
	{
		if (body)
			body->SetLinearVelocity(Vector2());
		return;
	}
	Vector2 playerPos{ playerNode->GetPosition2D() };
	Vector2 myPos{ node_->GetPosition2D() };
	Vector2 targetDirection{ playerPos - myPos };
	targetDirection.Normalize();

	node_->SetRotation2D(GetFullAngle(targetDirection));
	if (body)
		body->SetLinearVelocity(targetDirection * SEEKER_MOVE_SPEED);
	node_->GetChild("ShipExhaust")->SetEnabled(true);
}

Color PlayerHunter::fadeColor(Color from, Color to, float amount)
{
	float r = fadeComponent(from.r_, to.r_, amount);
	float g = fadeComponent(from.g_, to.g_, amount);
	float b = fadeComponent(from.b_, to.b_, amount);
	float a = fadeComponent(from.a_, to.a_, amount);

	return Color(r, g, b, a);
}

float PlayerHunter::fadeComponent(float from, float to, float amount)
{
	float r;
	if (to < from)
	{
		float dr = (from - to) * amount;
		r = from - dr;
	}
	else
	{
		float dr = (to - from) * amount;
		r = from + dr;
	}
	return r;
}
