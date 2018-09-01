#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Gravitator : public LogicComponent
{
	URHO3D_OBJECT(Gravitator, LogicComponent);

public:
	Gravitator(Context* context);
	/// Handle scene update. Called by LogicComponent base class.
	virtual void Update(float timeStep);
};
