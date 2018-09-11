#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Pulsar : public LogicComponent
{
	URHO3D_OBJECT(Pulsar, LogicComponent);

public:
	explicit Pulsar(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	void Update(float timeStep) final;
};
