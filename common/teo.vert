@vertexShader

uniform vec2 unif_TextureTiling;
varying vec2 var_TexCoord;

varying vec3 var_EyePos;

///////////////////////////////////////////////////////////////////////////////////////////////////
void setupSurfaceData(vec4 eyeSpacePosition)
{
	var_TexCoord = gl_MultiTexCoord0.xy; //* unif_TextureTiling;

	var_EyePos = (gl_ModelViewMatrix * gl_Vertex).xyz;
}