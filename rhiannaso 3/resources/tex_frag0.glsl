#version 330 core

uniform sampler2D Texture0;
uniform float MatShine;

uniform int flip;
uniform vec3 D;

vec3 MatAmb;
vec3 MatDif;
vec3 MatSpec;

float a = 1;
float b = 0;
float c = 0;

float a2 = 0;
float b2 = 0.5;
float c2 = 0;

in vec2 vTexCoord;

out vec4 Outcolor;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
in vec3 moonDir;
//position of the vertex in camera space
in vec3 EPos;

vec3 I0 = vec3(0.8, 0.8, 0.3);
//vec3 I0 = vec3(1, 1, 1);

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);

  vec3 normal = normalize(fragNor);
  if (flip < 1)
  	normal *= -1.0f;
  vec3 light = normalize(lightDir);
  float dC = max(0, dot(normal, light));
  float dist = sqrt(pow((lightDir.x - EPos.x), 2) + pow((lightDir.y - EPos.y), 2) + pow((lightDir.z - EPos.z), 2));
  float denom = a + (b*dist) + (c*pow(dist, 2));
  vec3 IL = (dot(normalize(D), light)*I0)/denom;
  //float IL = 1/denom;

  vec3 moon = normalize(moonDir);
  float dC2 = max(0, dot(normal, moon));
  float dist2 = sqrt(pow((moonDir.x - EPos.x), 2) + pow((moonDir.y - EPos.y), 2) + pow((moonDir.z - EPos.z), 2));
  float denom2 = a2 + (b2*dist2) + (c2*pow(dist2, 2));

  vec3 V = -1*EPos;
  vec3 H = normalize(lightDir + V);
  float NH = max(0, dot(normal, H));
  float NHPow = pow(NH, MatShine);
  if (texColor0.a < 0.1) {
      discard;
  }

  vec3 H2 = normalize(moonDir + V);
  float NH2 = max(0, dot(normal, H2));
  float NHPow2 = pow(NH2, MatShine);

  MatAmb = (0.3*texColor0).xyz;
  MatDif = (0.7*texColor0).xyz;
  MatSpec = (0.7*texColor0).xyz;

  //Outcolor = vec4(MatAmb + (dC*MatDif) + (NHPow*MatSpec), 1.0);
  Outcolor = vec4((MatAmb*IL) + ((1/denom)*((dC*MatDif*IL) + (NHPow*MatSpec*IL))) + ((1/denom2)*((dC2*MatDif) + (NHPow2*MatSpec))), 1.0);
  //Outcolor = vec4((MatAmb*IL) + ((1/denom)*((dC*MatDif*IL) + (NHPow*MatSpec*IL))), 1.0);

  //to confirm texture coordinates
  //Outcolor = vec4(vTexCoord.x, vTexCoord.y, 0, 0);
}

