
#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <string>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

class Program;


class Keyframe
{

public:
    Keyframe();
    Keyframe(float start, float end, float t, std::string part);
	virtual ~Keyframe();
    // void init(float start, float end, float t);
    float returnStart() { return startAngle; }
    float returnEnd() { return endAngle; }
    float returnDur() { return duration; }
    char* returnPart() { return part; }
    float interpolate(float currTime);
    void setStart(float t) { startTime = t; }
    bool isDone() { return done; }
    void resetDone() { done = false; }

private:
    float startAngle;
    float endAngle;
    float duration; // duration of keyframe animation
    float startTime;
    float done;
    char* part;
};

#endif // KEYFRAME_H
