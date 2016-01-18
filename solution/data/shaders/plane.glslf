#version 330

const int numberOfLights = 3;

in vec2 texCoord;
in vec3 vertexNormal;
in vec3 cameraSpacePosition;
in vec4 lightPos[numberOfLights];

out vec4 outputColor;

uniform vec2 shadowTexSize;

uniform sampler2D colorTexture;
uniform sampler2DShadow shadowTexture[numberOfLights];

layout(std140) uniform;

uniform Material
{
	vec4 specularColor;
	float specularShininess;
	float reflectivity;
} mtl;

struct PerLight
{
	vec4 cameraSpaceLightPos;
	vec4 lightIntensity;
};

uniform Light
{
	vec4 ambientIntensity;
	float lightAttenuation;
	PerLight lights[numberOfLights];
} lgt;

float calcAttenuation(in vec3 cameraSpaceLightPos, out vec3 lightDirection)
{
	vec3 lightDifference =  cameraSpaceLightPos - cameraSpacePosition;
	float lightDistanceSqr = dot(lightDifference, lightDifference);
	lightDirection = lightDifference * inversesqrt(lightDistanceSqr);
	
	return (1 / ( 1.0 + lgt.lightAttenuation * lightDistanceSqr));
}

vec4 computeLighting(in PerLight lightData, in vec4 diffuseColor)
{
	vec3 lightDir;
	vec4 lightIntensity;

	float atten = calcAttenuation(lightData.cameraSpaceLightPos.xyz, lightDir);
	lightIntensity = atten * lightData.lightIntensity;
	
	vec3 surfaceNormal = normalize(vertexNormal);
	float cosAngIncidence = clamp(dot(surfaceNormal, lightDir), 0.0, 1.0);
	
	vec4 lighting = diffuseColor * lightIntensity * cosAngIncidence;
	
	if (mtl.specularShininess != 0.0)
	{
		vec3 viewDirection = normalize(-cameraSpacePosition);
		vec3 halfAngle = normalize(lightDir + viewDirection);
		float angleNormalHalf = acos(dot(halfAngle, surfaceNormal));
		float exponent = angleNormalHalf / mtl.specularShininess;
		exponent = -(exponent * exponent);
		float gaussianTerm = exp(exponent);

		gaussianTerm = cosAngIncidence != 0.0 ? gaussianTerm : 0.0;
		lighting += mtl.specularColor * lightIntensity * gaussianTerm;
	}
	
	return lighting;
}

float calcShadowFactor(vec4 lightSpacePos, sampler2DShadow shadowTex)
{
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * projCoords.x + 0.5;
    UVCoords.y = 0.5 * projCoords.y + 0.5;

	// HACK
	if (UVCoords.x <= 0.0 || UVCoords.x >= 1.0 || 
		UVCoords.y <= 0.0 || UVCoords.y >= 1.0)
		return 1.0;

	float z = 0.5 * projCoords.z + 0.5;
	float xOffset = 1.0 / shadowTexSize.x;
    float yOffset = 1.0 / shadowTexSize.y;
 
	float factor = 0.0;
	const int vb = 1;
    for (int y = -vb ; y <= vb ; y++)
        for (int x = -vb ; x <= vb ; x++) 
		{
            vec2 Offsets = vec2(x * xOffset, y * yOffset);
            vec3 UVC = vec3(UVCoords + Offsets, z + 0.00001);
            factor += texture(shadowTex, UVC);
        }

	float divFactor = vb * 2.0 + 1.0;
	divFactor = 2.0 * divFactor * divFactor;
	return (0.5 + (factor / divFactor));
}

void main()
{
	vec4 diffuseColor = texture(colorTexture, texCoord);
	vec4 accumLighting = diffuseColor * lgt.ambientIntensity;

	accumLighting += computeLighting(lgt.lights[0], diffuseColor) *
	calcShadowFactor(lightPos[0], shadowTexture[0]);
	accumLighting += computeLighting(lgt.lights[1], diffuseColor) *
	calcShadowFactor(lightPos[1], shadowTexture[1]);
	accumLighting += computeLighting(lgt.lights[2], diffuseColor) *
	calcShadowFactor(lightPos[2], shadowTexture[2]);

	outputColor = accumLighting;
}
