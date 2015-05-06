@surfaceShader

uniform sampler2D unif_DiffuseMap;
varying vec2 var_TexCoord;

uniform float unif_Gloss;
uniform float unif_Shininess;

varying vec3 var_Normal;

uniform sampler2D mask_texture;
uniform int mask_enabled;
uniform float unif_Saturation;

SurfaceData getSurfaceData(void)
{
	SurfaceData sd;

	vec3 greyscale = vec3(0.299, 0.587, 0.114);
	float grey = dot(texture2D(unif_DiffuseMap, var_TexCoord).rgb, greyscale);

	// diffuse component (avoiding)
	sd.albedo = vec4(0, 0, 0, 1);
	// saturation
	sd.emissive = mix(vec4(grey,grey,grey,1),texture2D(unif_DiffuseMap, var_TexCoord), unif_Saturation);
	// greyscale
//  	sd.emissive = vec4(grey,grey,grey,1);
	if (mask_enabled > 0) sd.emissive[3] = mix(0.0, 1.0, texture2D(mask_texture, var_TexCoord)[0]);
	sd.shininess = unif_Shininess;
	sd.gloss = unif_Gloss;
	sd.normal = var_Normal;

	return sd;
}
