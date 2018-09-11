#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class PlayerEvader : public LogicComponent
{
	URHO3D_OBJECT(PlayerEvader, LogicComponent);

public:
	explicit PlayerEvader(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	void Update(float timeStep) final;
};
