#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTanget;
layout(location = 4) in vec3 aBitanget;
layout(location = 5) in vec2 aUV;

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

layout(push_constant) uniform Push{
	mat4 modelMatrix;
	mat4 normalMatrix;
} uPush;

layout(location = 0) out vec3 vColor;
layout(location = 1) out vec2 vUV;
layout(location = 2) out vec3 vPositionWorld;
layout(location = 3) out mat3 vTBN;

void main()
{
	vec4 positionWorld = uPush.modelMatrix * vec4(aPos, 1.0);

	vec3 normalWorld = normalize(mat3(uPush.normalMatrix) * aNormal);
	vec3 tangetWorld = normalize(mat3(uPush.normalMatrix) * aTanget);
	vec3 bitangetWorld = normalize(mat3(uPush.normalMatrix) * aBitanget);
	vTBN = mat3(tangetWorld, bitangetWorld, normalWorld);

	vColor = aColor;
	vUV = aUV;
	vPositionWorld = positionWorld.xyz;
	gl_Position = uUbo.projection * uUbo.view * positionWorld;
}