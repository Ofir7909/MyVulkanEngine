#version 450

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec3 vPositionWorld;
layout(location = 2) in vec3 vNormalWorld;


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

layout(push_constant) uniform Push{
	mat4 modelMatrix;
	mat4 normalMatrix;
} uPush;


layout(location = 0) out vec4 oColor;


void main()
{
	vec3 ambientLight = uUbo.ambientLightColor.xyz * uUbo.ambientLightColor.w;
	vec3 diffuseLight = ambientLight;
	vec3 specularLight = vec3(0.0);

	vec3 surfaceNormal = normalize(vNormalWorld);

	vec3 cameraPosWorld = uUbo.inverseView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - vPositionWorld);

	for(int i = 0; i < uUbo.numLights; i++)
	{
		PointLight light = uUbo.pointLights[i];
		vec3 directionToLight = light.position.xyz - vPositionWorld;
		vec3 lightColor = light.color.xyz * light.color.w;

		//diffuse
		float attenuation = 1.0 / dot(directionToLight,directionToLight);
		directionToLight = normalize(directionToLight);
		float intensity = max(dot(surfaceNormal, directionToLight), 0) * attenuation;

		diffuseLight += intensity * lightColor;

		//specular
		vec3 halfAngle = normalize(viewDirection + directionToLight);
		float bling = max(dot(halfAngle, surfaceNormal), 0);
		bling = pow(bling, 256);
		specularLight += bling * lightColor * attenuation;
	}

	oColor = vec4(vColor * diffuseLight + vColor * specularLight, 1.0);
}