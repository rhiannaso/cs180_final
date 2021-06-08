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
in vec3 camDir;
in vec3 moonDir;
//position of the vertex in camera space
in vec3 EPos;

float a = 1;
float b = 0;
float c = 0;

float a2 = 0;
float b2 = 0.5;
float c2 = 0;

//float a3 = 0.5;
//float b3 = 0;
//float c3 = 0;

//vec3 I0 = vec3(0.8, 0.8, 0.5);
vec3 I0 = vec3(1, 1, 1);

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
    //float dist = sqrt(pow((lightDir.x - EPos.x), 2) + pow((lightDir.y - EPos.y), 2) + pow((lightDir.z - EPos.z), 2));
    //float denom = a + (b*dist) + (c*pow(dist, 2));
    float denom = a + (b*length(lightDir)) + (c*pow(length(lightDir), 2));
    vec3 IL = (dot(normalize(D), light)*I0)/denom;
    //float IL = 1/denom;

    vec3 cam = normalize(camDir);
    vec3 moon = normalize(moonDir);

	float dC = max(0, dot(normal, light));
	vec3 V = -1*EPos;
    vec3 H = normalize(lightDir + V);
    float NH = (normal.x*H.x) + (normal.y*H.y) + (normal.z*H.z);
    float NHPow = pow(NH, MatShine);

    float dC2 = max(0, dot(normal, cam));
    vec3 H2 = normalize(camDir + V);
    float NH2 = (normal.x*H2.x) + (normal.y*H2.y) + (normal.z*H2.z);
    float NHPow2 = pow(NH2, MatShine);
    float dist2 = sqrt(pow((camDir.x - EPos.x), 2) + pow((camDir.y - EPos.y), 2) + pow((camDir.z - EPos.z), 2));
    float denom2 = a2 + (b2*dist2) + (c2*pow(dist2, 2));

    float dC3 = max(0, dot(normal, moon));
    vec3 H3 = normalize(moonDir + V);
    float NH3 = (normal.x*H3.x) + (normal.y*H3.y) + (normal.z*H3.z);
    float NHPow3 = pow(NH3, MatShine);
    float dist3 = sqrt(pow((moonDir.x - EPos.x), 2) + pow((moonDir.y - EPos.y), 2) + pow((moonDir.z - EPos.z), 2));
    //float denom3 = a3 + (b3*dist3) + (c3*pow(dist3, 2));
    float denom3 = 1;

    //color = vec4((MatAmb) + ((1/denom)*((dC*MatDif*IL) + (NHPow*MatSpec*IL))) + ((1/denom2)*((dC2*MatDif) + (NHPow2*MatSpec))), 1.0);
    //color = vec4((MatAmb) + ((1/denom)*((dC*MatDif*IL) + (NHPow*MatSpec*IL))) + ((1/denom2)*((dC2*MatDif) + (NHPow2*MatSpec))) + ((1/denom3)*((dC3*MatDif) + (NHPow3*MatSpec))), 1.0);
    //color = vec4((MatAmb*IL) + ((1/denom)*((dC*MatDif*IL) + (NHPow*MatSpec*IL))) + ((1/denom2)*((dC2*MatDif) + (NHPow2*MatSpec))) + ((1/denom3)*((dC3*MatDif) + (NHPow3*MatSpec))), 1.0);
	//color = vec4((MatAmb*IL) + ((1/denom)*((dC*MatDif*IL) + (NHPow*MatSpec*IL))), 1.0);
    //color = vec4(MatAmb + (dC*MatDif) + (NHPow*MatSpec), 1.0);
    color = vec4((MatAmb*I0) + (dC3*MatDif*I0) + (NHPow3*MatSpec*I0), 1.0);
}
