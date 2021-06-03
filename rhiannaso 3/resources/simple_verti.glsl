#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout (location = 3) in mat4 instanceMatrix;
uniform mat4 P;
uniform mat4 V;
uniform vec3 lightPos;

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;
out vec2 vTexCoord;

void main()
{
	gl_Position = P * V * instanceMatrix * vertPos;
	fragNor = (V * instanceMatrix * vec4(vertNor, 0.0)).xyz;
    lightDir = vec3(V*(vec4(lightPos - (instanceMatrix*vertPos).xyz, 0.0)));
	EPos = (V * instanceMatrix * vertPos).xyz;
    
	vTexCoord = vertTex;
}
