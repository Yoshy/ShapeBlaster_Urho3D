#pragma once

#include <Urho3D/Core/Object.h>

using namespace Urho3D;

struct ETimer
{
	unsigned int id;
	float timeInterval;
	float timeRemain;
	int numRepeats;
};

URHO3D_EVENT(E_TIMEREVENT, TimerEvent)
{
	URHO3D_PARAM(P_TIMER_ID, TimerID);      // StringHash
};

class TimerEvents : public Object
{
	URHO3D_OBJECT(TimerEvents, Object);

public:
	explicit TimerEvents(Context* context);
	~TimerEvents() final;
	void AddTimerEvent(StringHash id, float timeInterval, unsigned int repeats);

private:
	PODVector<ETimer*> timers_;

	void Update(StringHash eventType, VariantMap& eventData);
};
