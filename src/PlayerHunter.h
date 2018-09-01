#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class PlayerHunter : public LogicComponent
{
	URHO3D_OBJECT(PlayerHunter, LogicComponent);

public:
	PlayerHunter(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);

private:
	Color fadeColor(Color from, Color to, float amount);
	float PlayerHunter::fadeComponent(float from, float to, float amount);
};
