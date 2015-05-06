@surfaceShader 

uniform sampler2D unif_DiffuseMap;
varying vec2 var_TexCoord;

uniform float unif_Gloss;
uniform float unif_Shininess;

varying vec3 var_Normal;

uniform sampler2D mask_texture;
uniform int mask_enabled;

SurfaceData getSurfaceData(void)
{
	SurfaceData sd;
	// diffuse component (avoiding)
	sd.albedo = vec4(0, 0, 0, 1);
	sd.emissive = texture2D(unif_DiffuseMap, var_TexCoord) * gl_Color;
	if (mask_enabled > 0) {
		sd.emissive = 0.25 * texture2D(mask_texture, var_TexCoord) + texture2D(unif_DiffuseMap, var_TexCoord);
		sd.emissive[3] = mix(0.0, 1.0, texture2D(mask_texture, var_TexCoord)[0]);
	}
	sd.shininess = unif_Shininess;
	sd.gloss = unif_Gloss;
	sd.normal = var_Normal;
	
	return sd;
}
