#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class ScreenInhabitant : public LogicComponent
{
	URHO3D_OBJECT(ScreenInhabitant, LogicComponent);

public:
	ScreenInhabitant(Context* context);
	virtual void DelayedStart();
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);

private:
	float halfWidth_;
	float halfHeight_;
	bool IsOutOfScreen();
};
