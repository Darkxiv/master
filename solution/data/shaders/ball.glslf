#version 330

in vec2 texCoord;
in vec3 cameraNormal;
in vec3 worldNormal;
in vec3 cameraSpacePos;
in vec3 worldSpacePos;

in vec3 lightingNormal;
in vec3 lightingEyeVec;
in vec3 lightingPos;

out vec4 outputColor;

uniform vec3 camPos;
uniform sampler2D colorTexture;
uniform samplerCube skybox;

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

const int numberOfLights = 3;

uniform Light
{
	vec4 ambientIntensity;
	float lightAttenuation;
	PerLight lights[numberOfLights];
} lgt;

float calcAttenuation(in vec3 cameraSpaceLightPos, out vec3 lightDirection)
{
	vec3 lightDifference =  cameraSpaceLightPos - cameraSpacePos;
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
	
	vec3 surfaceNormal = normalize(cameraNormal);
	float cosAngIncidence = clamp(dot(surfaceNormal, lightDir), 0.0, 1.0);
	
	vec4 lighting = diffuseColor * lightIntensity * cosAngIncidence;
	
	if (mtl.specularShininess != 0.0)
	{
		vec3 viewDirection = normalize(-cameraSpacePos);
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

vec4 computeIBL(vec4 surfColor)
{
	vec3 ln = normalize(lightingNormal);
	vec3 lv = normalize(lightingEyeVec);
	float vdn = dot(lv, ln);
	
	float kr = 1.5;
	float krMin = 0.05 * kr;

	float fres = krMin + (kr - krMin) * pow((1.0 - abs(vdn)), 5.0); // according to GPU gems
	
	vec3 reflVect = normalize(reflect(lv, ln));

	float b = -2.0 * dot(reflVect, lightingPos);
	float c = dot(lightingPos, lightingPos) - 1.0;
	float discrim = b * b - 4.0 * c;
	bool hasIntersects = false;
	vec4 reflColor = vec4(1.0, 0.0, 0.0, 1.0);
	if (discrim > 0)
		hasIntersects = ((abs(sqrt(discrim) - b) / 2.0) > 0.00001);

	if (hasIntersects) 
	{
		float nearT = 1.0;
		reflVect = nearT * reflVect - lightingPos;
		reflColor = fres * texture(skybox, reflVect);
	}
	vec4 result = mix(surfColor, reflColor, mtl.reflectivity);
	return result;
}

void main()
{
	vec4 diffuseColor = texture(colorTexture, texCoord);
	vec4 accumLighting = diffuseColor * lgt.ambientIntensity;

	for(int light = 0; light < numberOfLights; light++)
		accumLighting += computeLighting(lgt.lights[light], diffuseColor);
	
	outputColor = computeIBL(accumLighting);
}
