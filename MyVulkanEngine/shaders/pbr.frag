#version 450

#define MAX_REFLECTION_LOD 10.0

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vPositionWorld;
layout(location = 3) in mat3 vTBN;

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

layout(set=1, binding=0) uniform MaterialParams
{
	vec4 albedo;
	vec4 emission;
	vec2 uvScale;
	float roughness;
	float metallic;
} uMaterialParams;

layout(set=1, binding=1) uniform sampler2D albedoTexture;
layout(set=1, binding=2) uniform sampler2D armTexture;
layout(set=1, binding=3) uniform sampler2D normalTexture;


layout(push_constant) uniform Push{
	mat4 modelMatrix;
	mat4 normalMatrix;
} uPush;


layout(location = 0) out vec4 oColor;

const float PI = 3.14159265358979;


// GGX/Trowbridge-Reitz Normal distribution function
// alpha = roughness ^ 2;
float D(float alpha, vec3 N, vec3 H)
{
	float nDotH = dot(N, H);
	float alpha2 = alpha * alpha;
	
	float temp = 1 + nDotH * nDotH * (alpha2 - 1);
	float denom = PI * temp * temp;
	denom = max(denom, 0.00001);

	return alpha2 / denom;
}

// Schlick-Beckmann Geometry shadowing function
float G1(float k, vec3 N, vec3 X)
{
	float nDotX = max(dot(N, X), 0.0);
	float denom = nDotX * (1-k) + k;
	denom = max(denom, 0.00001);

	return nDotX / denom;
}

// Smith model
float G_ibl(float roughness, vec3 N, vec3 L, vec3 V)
{
	float k = roughness * roughness / 2.0;
	return G1(k, N, L) * G1(k, N, V);
}
float G_direct(float roughness, vec3 N, vec3 L, vec3 V)
{
	float k = (roughness + 1) * (roughness + 1) / 8.0;
	return G1(k, N, L) * G1(k, N, V);
}

// Fresnel-Schlick Function
// F0 = base reflectivity
vec3 F(vec3 F0, vec3 V, vec3 H)
{
	float vDotH = max(dot(V, H), 0.0);
	return F0 + (vec3(1.0) - F0) * pow(1-vDotH, 5);
}

void main()
{
	vec3 cameraPosWorld = uUbo.inverseView[3].xyz;

	vec2 scaledUV = vUV * uMaterialParams.uvScale;
	vec3 albedo = uMaterialParams.albedo.rgb * texture(albedoTexture, scaledUV).rgb;
	float ao = uMaterialParams.roughness * texture(armTexture, scaledUV).r;
	float roughness = uMaterialParams.roughness * texture(armTexture, scaledUV).g;
	float metallic = uMaterialParams.metallic * texture(armTexture, scaledUV).b;
	vec3 normalTexture = texture(normalTexture, scaledUV).xyz * 2.0 - 1.0;

	vec3 N = normalize(vTBN * normalTexture);
	vec3 V = normalize(cameraPosWorld - vPositionWorld);

	//vec3 ambientLight = uUbo.ambientLightColor.xyz * uUbo.ambientLightColor.w * ao;
	vec3 ambientLight = textureLod(uSkybox, N, MAX_REFLECTION_LOD).rgb * ao;
	vec3 outLight = ambientLight * albedo;
	
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	vec3 R = reflect(-V, N);

	//for each point light
	for(int i = 0; i < uUbo.numPointLights; i++)
	{
		PointLight light = uUbo.pointLights[i];
		vec3 lightColor = light.color.xyz * light.color.w;

		vec3 L = light.position.xyz - vPositionWorld;
		float attenuation = 1.0 / dot(L,L);
		L = normalize(L);
		vec3 H = normalize(V + L);

		// PBR
		vec3 ks = F(F0, V, H);
		vec3 kd = (vec3(1.0) - ks) * (1.0 - metallic);

		vec3 lambert = albedo / PI;
		float alpha = roughness * roughness;

		vec3 cookTorranceNom = D(alpha, N, H) * G_direct(roughness, N, L, V) * ks;
		float cookTorranceDenom = 4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0);
		cookTorranceDenom = max(cookTorranceDenom, 0.00001);
		vec3 cookTorrance = cookTorranceNom / cookTorranceDenom;

		vec3 BRDF = kd * lambert + cookTorrance;
		outLight += BRDF * lightColor * attenuation * max(dot(L, N), 0);
	}

	//for each directional light
	for(int i = 0; i < uUbo.numDirectionalLights; i++)
	{
		DirectionalLight light = uUbo.directionalLights[i];
		vec3 lightColor = light.color.xyz * light.color.w;

		vec3 L = light.direction.xyz;
		float attenuation = 1.0;
		L = normalize(L);
		vec3 H = normalize(V + L);

		// PBR
		vec3 ks = F(F0, V, H);
		vec3 kd = (vec3(1.0) - ks) * (1.0 - metallic);

		vec3 lambert = albedo / PI;
		float alpha = roughness * roughness;

		vec3 cookTorranceNom = D(alpha, N, H) * G_direct(roughness, N, L, V) * ks;
		float cookTorranceDenom = 4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0);
		cookTorranceDenom = max(cookTorranceDenom, 0.00001);
		vec3 cookTorrance = cookTorranceNom / cookTorranceDenom;

		vec3 BRDF = kd * lambert + cookTorrance;
		outLight += BRDF * lightColor * attenuation * max(dot(L, N), 0);
	}

	outLight += uMaterialParams.emission.xyz + uMaterialParams.emission.w;

	// HDR Mapping
	outLight = outLight / (outLight + vec3(1.0));

	oColor = vec4(outLight, 1.0);
}