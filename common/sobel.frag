// sobel-based edge detection. WIP
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

	float Brightness = 1.0;

	const int NUM = 9;
// 	const float threshold = 0.05;
	const float threshold = 0.02;

	vec2 c[NUM];

	const float s = 0.000244140625; // 1 / 4096
// 	const float s = 0.00048828125; // 1 / 2048
// 	const float s = 0.0009765625; // 1 / 1024
// 	const float s = 0.001953125; // 1/ 512
// 	const float s = 0.0078125; // 1/128

	c[0] = vec2(-s, s);
	c[1] = vec2( 0.00 , s);
	c[2] = vec2( s, s);
	c[3] = vec2(-s, 0.00 );
	c[4] = vec2( 0.0, 0.0 );
	c[5] = vec2( s, s );
	c[6] = vec2(-s,-s);
	c[7] = vec2( 0.00 , -s);
	c[8] = vec2( s,-s);

	vec3 col[NUM];
	int i;

	for (i = 0; i < NUM; i++) {
		col[i] = texture2D(unif_DiffuseMap, var_TexCoord + c[i]);
	}

	vec3 rgb2lum = vec3(0.30, 0.59, 0.11);
	float lum[NUM];
	for (i = 0; i < NUM; i++) {
		lum[i] = dot(col[i].xyz, rgb2lum);
	}

	float x = lum[2]+ lum[8]+2*lum[5]-lum[0]-2*lum[3]-lum[6];
	float y = lum[6]+2*lum[7]+ lum[8]-lum[0]-2*lum[1]-lum[2];

 	float edgeX =(x*x + y*y < threshold) ? 0.0 : 1.0;
	vec3 edge = vec3(edgeX,edgeX,edgeX);
// 	vec3 edge = vec3(1,1,1);


	sd.albedo = vec4(0, 0, 0, 1);
// 	sd.emissive = texture2D(unif_DiffuseMap, var_TexCoord) * gl_Color;
// 	sd.emissive = vec4(Brightness * col[5].xyz * edge, 1.0);

	// outline
	sd.emissive = vec4(Brightness * edge, 1.0);

	// outline + texture
// 	sd.emissive = (0.5 * texture2D(unif_DiffuseMap, var_TexCoord) * gl_Color) + vec4(Brightness * edge, 1.0);

	// like a wireframe?
// 	sd.emissive = texture2D(unif_DiffuseMap, var_TexCoord) * gl_Color * vec4(Brightness * edge, 1.0);

	// line above object
// 	if (length(sd.emissive) <= 1.0) {
// 		sd.emissive = texture2D(unif_DiffuseMap, var_TexCoord) * gl_Color;
// 	}

	if (mask_enabled) {
		sd.emissive[3] = mix(0.0, 1.0, texture2D(mask_texture, var_TexCoord)[0]);
	}
	sd.shininess = unif_Shininess;
	sd.gloss = unif_Gloss;
	sd.normal = var_Normal;
	
	return sd;
}
