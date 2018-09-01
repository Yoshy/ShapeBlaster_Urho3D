#include <Urho3D/Math/Vector2.h>

using namespace Urho3D;

float GetFullAngle(Vector2 v)
{
	if (v.y_ >= 0)
	{
		return v.Angle(Vector2(1.0f, 0.0f));
	}
	else {
		return 360 - v.Angle(Vector2(1.0f, 0.0f));
	}
}

Vector2 GetDirectionFromAngle(float angle)
{
	return Vector2(Cos(angle), Sin(angle));
}
