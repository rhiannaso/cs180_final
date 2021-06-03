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
    startTime = 0;
    done = false;
}

// void Keyframe::init(float start, float end, float t) {
//     startAngle = start;
// 	endAngle = end;
// 	duration = t;
//     startTime = 0;
//     done = false;
// }

float Keyframe::interpolate(float currTime) {
    cout << "Start in func: " << startTime << endl;
    cout << "Curr time in func: " << currTime << endl;
    float percentDone = (currTime-startTime) / duration;
    //cout << percentDone << endl;
    float diff = endAngle - startAngle;
    if (percentDone > 1) {
        done = true;
    }
    return startAngle + (percentDone*diff);
}

Keyframe::~Keyframe()
{
}