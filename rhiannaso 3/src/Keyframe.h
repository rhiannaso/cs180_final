
#pragma once

#include <string>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

class Program;


class Keyframe
{

public:
    Keyframe(float start, float end, float t);
	virtual ~Keyframe();
    float returnStart() { return startAngle; }
    float returnEnd() { return endAngle; }
    void setTimeSince(float t) { startTime = t; }
    void resetCurrTime() { currTime = 0; }
    float interpolate();
    void update(float deltaTime);
    bool isDone();

private:
    float startAngle;
    float endAngle;
    float duration; // duration of keyframe animation
    float startTime;
    float currTime = 0;
    bool done = false;
};
