#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class ScreenInhabitant : public LogicComponent
{
	URHO3D_OBJECT(ScreenInhabitant, LogicComponent);

public:
	explicit ScreenInhabitant(Context* context);
	void DelayedStart() final;
	/// Handle scene update. Called by LogicComponent base class.
	void Update(float timeStep) final;

private:
	float halfWidth_;
	float halfHeight_;
	bool IsOutOfScreen();
};
