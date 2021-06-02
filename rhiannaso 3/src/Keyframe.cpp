#include "Keyframe.h"
#include <iostream>
#include <cassert>

#include "GLSL.h"
#include "Program.h"

using namespace std;

Keyframe::Keyframe(float start, float end, float t) {
	startAngle = start;
	endAngle = end;
	duration = t;
}

void Keyframe::update(float deltaTime) {
    currTime += deltaTime;
}

float Keyframe::interpolate() {
    float percentDone = (currTime-startTime) / duration;
    float diff = endAngle - startAngle;
    if (percentDone > 1) {
        done = true;
    } 
    return startAngle + (percentDone*diff);
}

// float Keyframe::interpolate(float deltaTime)
// {
//     // scale the deltatime by the duration
//     // so that it works with the parameterized 
//     // Bezier functions, which expext 0<=t<=1
//     startTime += deltaTime / duration;
//     float percentDone = startTime;
//     float diff = endAngle - startAngle;
//     // don't overshoot 1
//     if(startTime > 1)
//     {
//         startTime = 1;
//         done = true;
//     }
//     return startAngle + (percentDone*diff);
// }

bool Keyframe::isDone()
{
    return done;
}

Keyframe::~Keyframe()
{
}