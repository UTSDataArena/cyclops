// Applies a texture to the emissive channel instead of the diffuse channel
// and a rim lighting outline

@surfaceShader 

// The diffuse texture
uniform sampler2D unif_DiffuseMap;
varying vec2 var_TexCoord;

uniform float unif_Shininess;
uniform float unif_Gloss;
uniform vec3 unif_CamPos;

uniform vec4 unif_Outline_Color;
uniform float unif_Outline_Width;

varying vec3 var_Normal;
varying vec3 var_EyePos;

///////////////////////////////////////////////////////////////////////////////////////////////////
SurfaceData getSurfaceData(void)
{
	SurfaceData sd;
	sd.shininess = unif_Shininess;
	sd.gloss = unif_Gloss;
	sd.normal = var_Normal;
   	sd.albedo = vec4(0, 0, 0, 1);

	vec3 norm = normalize(var_Normal).xyz;
 	vec3 viewDir = normalize(unif_CamPos - var_EyePos).xyz;
 	float d = dot(viewDir, norm);
	float m = smoothstep(unif_Outline_Width, min(1.0, 0.05 + unif_Outline_Width), d);
	sd.emissive = mix(unif_Outline_Color, texture2D(unif_DiffuseMap, var_TexCoord) * gl_FrontMaterial.diffuse, m);

	return sd;
}
