#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class PlayerHunter : public LogicComponent
{
	URHO3D_OBJECT(PlayerHunter, LogicComponent);

public:
	explicit PlayerHunter(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	void Update(float timeStep) final;

private:
	Color fadeColor(Color from, Color to, float amount);
	float fadeComponent(float from, float to, float amount);
};
