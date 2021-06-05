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
//in vec3 moonDir;
//position of the vertex in camera space
in vec3 EPos;

float a = 0.9;
float b = 0;
float c = 0;

vec3 I0 = vec3(0.8, 0.8, 0.3);

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
    //float dist = sqrt(pow((lightDir.x - EPos.x), 2) + pow((lightDir.y - EPos.y), 2) + pow((lightDir.z - EPos.z), 2));
    //float denom = a + (b*dist) + (c*pow(dist, 2));
    float denom = a + (b*length(lightDir)) + (c*pow(length(lightDir), 2));
    vec3 IL = (dot(normalize(D), light)*I0)/denom;
    //float IL = 1/denom;

    //vec3 moon = normalize(moonDir);

	float dC = max(0, dot(normal, light));
	vec3 V = -1*EPos;
    vec3 H = normalize(lightDir + V);
    float NH = (normal.x*H.x) + (normal.y*H.y) + (normal.z*H.z);
    float NHPow = pow(NH, MatShine);

    //float dC2 = max(0, dot(normal, moon));
    //vec3 H2 = normalize(moonDir + V);
    //float NH2 = (normal.x*H2.x) + (normal.y*H2.y) + (normal.z*H2.z);
    //float NHPow2 = pow(NH2, MatShine);

    //color = vec4((MatAmb*IL) + ((1/denom)*((dC*MatDif*IL) + (NHPow*MatSpec*IL))) + (dC2*MatDif) + (NHPow2*MatSpec), 1.0);
	color = vec4((MatAmb*IL) + ((1/denom)*((dC*MatDif*IL) + (NHPow*MatSpec*IL))), 1.0);
    //color = vec4(MatAmb + (dC*MatDif) + (NHPow*MatSpec), 1.0);
}
