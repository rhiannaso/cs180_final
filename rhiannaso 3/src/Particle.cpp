//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle(vec3 start) :
	charge(1.0f),
	m(1.0f),
	d(0.0f),
	x(start),
	v(0.0f, 0.0f, 0.0f),
	lifespan(1.0f),
	tEnd(0.0f),
	scale(1.0f),
	color(1.0f, 1.0f, 1.0f, 1.0f)
{
}

Particle::~Particle()
{
}

void Particle::load(vec3 start, float t)
{
	// Random initialization
	rebirth(t, start);
}

/* all particles born at the origin */
void Particle::rebirth(float t, vec3 start)
{
	charge = randFloat(0.0f, 1.0f) < 0.5 ? -1.0f : 1.0f;	
	m = 1.0f;
  	d = randFloat(0.0f, 0.02f);
	x = start;
	v.x = randFloat(-0.25f, 0.25f);
	v.y = randFloat(0.1f, 0.5f);
	v.z = randFloat(-0.25f, 0.25f);
    lifespan = randFloat(2.0f, 4.0f);
	tEnd = t + lifespan;
	scale = randFloat(0.2, 1.0f);
    color.r = 0.05;
   	color.g = 0.3;
   	color.b = 0.1;
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3 &g, const vec3 start)
{
    v += (0.1f*g);
    x += (0.1f*v);
    color.a = (tEnd-t)/lifespan;
}
