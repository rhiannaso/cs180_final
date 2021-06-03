
#ifndef KEYFRAME_H
#define KEYFRAME_H

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
    float interpolate(float currTime);
    void setStart(float t) { startTime = t; }
    bool isDone() { return done; }

private:
    float startAngle;
    float endAngle;
    float duration; // duration of keyframe animation
    float startTime;
    float done;
};

#endif // KEYFRAME_H
