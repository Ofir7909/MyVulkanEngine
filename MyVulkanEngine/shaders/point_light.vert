#version 450

const vec2 OFFSETS[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0, -1.0),

	vec2( 1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0,  1.0)
);

layout(push_constant) uniform Push{
	vec4 position;
	vec4 color;
	float radius;
} uPush;

struct PointLight{
	vec4 position;
	vec4 color;
};

layout(set=0, binding=0) uniform GlobalUbo{
	mat4 view;
	mat4 projection;
	mat4 inverseView;
	vec4 ambientLightColor;
	PointLight pointLights[10];
	int numLights;
} uUbo;

layout(location = 0) out vec2 vOffset;

void main()
{
	vOffset = OFFSETS[gl_VertexIndex];
	vec3 cameraRight = {uUbo.view[0][0], uUbo.view[1][0], uUbo.view[2][0]};
	vec3 cameraUp = {uUbo.view[0][1], uUbo.view[1][1], uUbo.view[2][1]};

	vec3 positionWorld = uPush.position.xyz
		+ uPush.radius * vOffset.x * cameraRight
		+ uPush.radius * vOffset.y * cameraUp;

	gl_Position = uUbo.projection * uUbo.view * vec4(positionWorld, 1.0);

}