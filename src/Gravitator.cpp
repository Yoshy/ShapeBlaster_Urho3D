#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/OctreeQuery.h>
#include <Urho3D/Math/Sphere.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/RigidBody2D.h>
#include "Gravitator.h"
#include "GameDefs.h"
#include "MathUtils.h"

using namespace Urho3D;

Gravitator::Gravitator(Context* context) : LogicComponent(context)
{
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void Gravitator::Update(float timeStep)
{
	Vector2 myPos{ node_->GetPosition2D() };
	PODVector<Node*> nodes;

	// FIXME: Replace brute force approach to some kind of spatial querying
	GetScene()->GetChildrenWithComponent<RigidBody2D>(nodes);
	for (Node* node : nodes)
	{
		if (node->GetNameHash() == StringHash{ BLACKHOLE_NODE_NAME })
			continue;
		Vector2 otherPos = node->GetPosition2D();
		Vector2 gravDir = (myPos- otherPos);
		float distSqr = gravDir.LengthSquared();
		gravDir.Normalize();
		if (node->GetNameHash() == StringHash{ "Bullet" })
			gravDir = -10.0f * gravDir;
		if (distSqr < 2.5f * 2.5f)
		{
			RigidBody2D* body{ node->GetComponent<RigidBody2D>() };
			Vector2 curVel = body->GetLinearVelocity();
			Vector2 newVel = (gravDir * 0.01f) + curVel;
			node->SetRotation2D(GetFullAngle(newVel));
			body->SetLinearVelocity(newVel);
		}
	}
}
