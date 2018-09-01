#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include <Urho3D/Urho2D/CollisionBox2D.h>
#include "ScreenInhabitant.h"

using namespace Urho3D;

ScreenInhabitant::ScreenInhabitant(Context* context) : LogicComponent(context)
{
	// Only the scene update event is needed: unsubscribe from the rest for optimization
	SetUpdateEventMask(USE_UPDATE);
}

void ScreenInhabitant::DelayedStart()
{
	Graphics* graphics{ GetSubsystem<Graphics>() };
	halfWidth_ = graphics->GetWidth() * PIXEL_SIZE * 0.5f;
	halfHeight_ = graphics->GetHeight() * PIXEL_SIZE * 0.5f;
}

void ScreenInhabitant::Update(float timeStep)
{
	//BoundingBox bbox{ node_->GetComponent<StaticSprite2D>()->GetWorldBoundingBox() };
	//DebugRenderer* dr = node_->GetParent()->GetComponent<DebugRenderer>();
	//dr->AddBoundingBox(bbox, Color::WHITE);

	if (IsOutOfScreen())
		node_->Remove();
}

bool ScreenInhabitant::IsOutOfScreen()
{
	Vector2 position{ node_->GetPosition2D() };
	if (position.x_ > halfWidth_ || position.x_ < -halfWidth_ || position.y_ > halfHeight_ || position.y_ < -halfHeight_)
		return true;
	return false;
}
