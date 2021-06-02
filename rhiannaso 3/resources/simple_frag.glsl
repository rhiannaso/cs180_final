#version 330 core 

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

float angle = 45;

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
    //vec3 D = vec3(0, 0, 1);
    float dist = sqrt(pow(lightDir.x, 2) + pow(lightDir.y, 2) + pow(lightDir.z, 2));
    //float denom = 0 + (1*dist) + (0*pow(dist, 2));
    //float IL = dot(D, light)/denom;
    //float IL = 1/denom;
	float dC = max(0, dot(normal, light));
	vec3 V = -1*EPos;
    vec3 H = normalize(lightDir + V);
    float NH = (normal.x*H.x) + (normal.y*H.y) + (normal.z*H.z);
    float NHPow = pow(NH, MatShine);

	//color = vec4((MatAmb*IL) + (dist*((dC*MatDif*IL) + (NHPow*MatSpec*IL))), 1.0);
    color = vec4(MatAmb + (dC*MatDif) + (NHPow*MatSpec), 1.0);
}
