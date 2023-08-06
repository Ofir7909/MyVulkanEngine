#version 450

layout(location = 0) in vec2 vOffset;

layout(push_constant) uniform Push{
	vec4 position;
	vec4 color;
	float radius;
} uPush;

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

layout(location = 0) out vec4 oColor;

const float PI = 3.14159265358979;
void main()
{
	float dis = length(vOffset);
	if(dis >= 1.0){discard;}

	float alpha = (cos(dis * PI) + 1.0) * 0.5;

	oColor = vec4(uPush.color.xyz + alpha, alpha);
}