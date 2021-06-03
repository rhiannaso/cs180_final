#version 330 core 

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;
uniform vec3 D;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

float a = 0;
float b = 2;
float c = 0;

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
    float dist = sqrt(pow((lightDir.x - EPos.x), 2) + pow((lightDir.y - EPos.y), 2) + pow((lightDir.z - EPos.z), 2));
    float denom = a + (b*dist) + (c*pow(dist, 2));
    float IL = dot(D, light)/denom;
    //float IL = 1/denom;

	float dC = max(0, dot(normal, light));
	vec3 V = -1*EPos;
    vec3 H = normalize(lightDir + V);
    float NH = (normal.x*H.x) + (normal.y*H.y) + (normal.z*H.z);
    float NHPow = pow(NH, MatShine);

	color = vec4((MatAmb*IL) + (dist*((dC*MatDif*IL) + (NHPow*MatSpec*IL))), 1.0);
    //color = vec4(MatAmb + (dC*MatDif) + (NHPow*MatSpec), 1.0);
}
