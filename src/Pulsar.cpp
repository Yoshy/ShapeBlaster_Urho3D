#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Urho2D/RigidBody2D.h>
#include "Pulsar.h"
#include "GameDefs.h"
#include "MathUtils.h"

using namespace Urho3D;

Pulsar::Pulsar(Context* context) : LogicComponent(context)
{
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void Pulsar::Update(float timeStep)
{
	float t = 1.0f + 0.1f * Sin(GetSubsystem<Time>()->GetElapsedTime() * 300.0f);
	node_->SetScale2D(t, t);
}
