#include "TimerEvents.h"
#include <Urho3D/Core/CoreEvents.h>

using namespace Urho3D;

TimerEvents::TimerEvents(Context* context) : Object(context)
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(TimerEvents, Update));
}

TimerEvents::~TimerEvents()
{
	for (PODVector<ETimer*>::Iterator i = timers_.Begin(); i != timers_.End(); ++i)
		delete *i;
	timers_.Clear();
}

void TimerEvents::AddTimerEvent(StringHash id, float timeInterval, unsigned int repeats)
{
	ETimer* et{ new ETimer() };
	et->id = id.Value();
	et->timeInterval = timeInterval;
	et->timeRemain = timeInterval;
	et->numRepeats = repeats;
	timers_.Push(et);
}

void TimerEvents::Update(StringHash eventType, VariantMap& eventData)
{
	for (PODVector<ETimer*>::Iterator i = timers_.Begin(); i != timers_.End(); ++i)
	{
		ETimer& et = **i;
		et.timeRemain = et.timeRemain - eventData[Update::P_TIMESTEP].GetFloat();
		if (et.timeRemain <= 0)
		{
			VariantMap& eventData = GetEventDataMap();
			eventData[TimerEvent::P_TIMER_ID] = et.id;
			SendEvent(E_TIMEREVENT, eventData);
			et.timeRemain = et.timeInterval;
			--et.numRepeats;
			if (et.numRepeats == 0) // That was last repeat, delete timer
			{
				delete *i;
				i = timers_.Erase(i);
				continue;
			}
			if (et.numRepeats < 0) // When numRepeats is set to zero, timer will repeat forever
				et.numRepeats = 0;
		}

	}
}
