#include "Keyframe.h"
#include <iostream>
#include <cassert>
#include <cstdlib>

#include "GLSL.h"
#include "Program.h"

using namespace std;

Keyframe::Keyframe(float start, float end, float t, string name) {
    //cout << name << endl;
	startAngle = start;
	endAngle = end;
	duration = t;
    startTime = 0;
    done = false;
    part = (char*)malloc(name.length());
    name.copy(part, name.length(), 0);
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
    cout << "PART: " << part << endl;
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