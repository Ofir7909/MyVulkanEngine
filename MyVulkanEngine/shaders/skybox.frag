#version 450

layout(location = 0) in vec3 vUVW;

struct PointLight{
	vec4 position;
	vec4 color;
};

struct DirectionalLight
{
	vec4 direction;
	vec4 color;
};

layout(set=0, binding=0) uniform GlobalUbo{
	mat4 view;
	mat4 projection;
	mat4 inverseView;
	vec4 ambientLightColor;
	PointLight pointLights[10];
	DirectionalLight directionalLights[10];
	int numPointLights;
	int numDirectionalLights;
} uUbo;

layout(set=0, binding=1) uniform samplerCube uSkybox;

layout(location = 0) out vec4 oColor;

const float PI = 3.14159265358979;

void main()
{
	oColor = vec4(textureLod(uSkybox, vUVW, 0));
}