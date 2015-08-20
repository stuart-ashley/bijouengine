#extension GL_EXT_geometry_shader4 : enable
//----------------------------------------------------------------------------
// Input: GL_TRIANGLES
// Output: GL_TRIANGLE_STRIP
// Varying: v_position, V_position
// Varying: v_normal, V_normal
// Varying: v_binormal, V_binormal
// Varying: v_coluv, V_coluv
// Varying: v_normuv, V_normuv
// Varying: v_alphauv, V_alphauv
// Varying: v_specuv, V_specuv
// Varying: v_lightuv, V_lightuv
// Varying: v_irradianceColor, V_irradianceColor
// Varying: v_viewToSampler, V_viewToSampler
// Varying: v_irradianceLength, V_irradianceLength
// Varying: v_irradianceLength4, V_irradianceLength4
// Varying: v_reflectionCoord, V_reflectionCoord
// Varying: v_envmapDir, V_envmapDir
//----------------------------------------------------------------------------
// uniforms
//----------------------------------------------------------------------------
uniform int u_depth;
uniform float u_delta;
//----------------------------------------------------------------------------
// varying
//----------------------------------------------------------------------------
varying in vec3 v_position[ 3 ];
varying in vec3 v_normal[ 3 ];
varying in vec3 v_binormal[ 3 ];
varying in vec2 v_coluv[ 3 ];
varying in vec2 v_normuv[ 3 ];
varying in vec2 v_alphauv[ 3 ];
varying in vec2 v_specuv[ 3 ];
varying in vec2 v_lightuv[ 3 ];
varying in vec3 v_irradianceColor[ 3 ];
varying in mat3 v_viewToSampler[ 3 ];
varying in float v_irradianceLength[ 3 ];
varying in vec4 v_irradianceLength4[ 3 ];
varying in vec3 v_reflectionCoord[ 3 ];
varying in vec3 v_envmapDir[ 3 ];

varying out vec3 V_position;
varying out vec3 V_normal;
varying out vec3 V_binormal;
varying out vec2 V_coluv;
varying out vec2 V_normuv;
varying out vec2 V_alphauv;
varying out vec2 V_specuv;
varying out vec2 V_lightuv;
varying out vec3 V_irradianceColor;
varying out mat3 V_viewToSampler;
varying out float V_irradianceLength;
varying out vec4 V_irradianceLength4;
varying out vec3 V_reflectionCoord;
varying out vec3 V_envmapDir;
varying out float V_clip[ 2 ];
//----------------------------------------------------------------------------
void vertices() {
	for ( int i = 0; i < gl_VerticesIn; ++i ) {
		gl_Position = gl_PositionIn[ i ];
		V_position = v_position[ i ];

#ifdef NORMAL
		V_normal = v_normal[ i ];
#endif

#ifdef ALPHAMAP
		V_alphauv = v_alphauv[ i ];
#endif

#ifdef COLORMAP
		V_coluv = v_coluv[ i ];
#endif

#ifdef NORMALMAP
		V_normuv = v_normuv[ i ];
		V_binormal = v_binormal[ i ];
#	ifdef IRRADIANCE
		V_viewToSampler = v_viewToSampler[ i ];
#	elif defined( IRRADIANCE2 )
		V_viewToSampler = v_viewToSampler[ i ];
		V_irradianceLength = v_irradianceLength[ i ];
#	elif defined( IRRADIANCE4 )
		V_viewToSampler = v_viewToSampler[ i ];
		V_irradianceLength4 = v_irradianceLength4[ i ];
#	endif
#else // NORMALMAP
#	if defined( IRRADIANCE ) || defined( IRRADIANCE2 ) || defined( IRRADIANCE4 )
		V_irradianceColor = v_irradianceColor[ i ];
#	endif
#endif // NORMALMAP

#ifdef SPECULARMAP
		V_specuv = v_specuv[ i ];
#endif

#ifdef LIGHTMAP
		V_lightuv = v_lightuv[ i ];
#endif

#if defined( MIRROR ) || defined( REFRACTION )
		V_reflectionCoord = v_reflectionCoord[ i ];
#endif

#ifdef ENAMP
		V_envmapDir = v_envmapDir[ i ];
#endif

		EmitVertex();
	}
	EndPrimitive();
}

void main() {
#ifdef VOLUME
	// extent
	float minz = gl_PositionIn[ 0 ].z;
	float maxz = gl_PositionIn[ 0 ].z;

	for ( int i = 1; i < gl_VerticesIn; ++i ) {
		minz = min( minz, gl_PositionIn[ i ].z );
		maxz = max( maxz, gl_PositionIn[ i ].z );
	}

	minz = minz * .5 + .5;
	maxz = maxz * .5 + .5;

	// add to layers
	V_clip[1] = 0.;
	for ( gl_Layer = 0; gl_Layer < u_depth - 1; ++gl_Layer ) {
		V_clip[0] = V_clip[1];
		V_clip[1] += u_delta;

		if ( maxz >= V_clip[0] && minz < V_clip[1] ) {
			vertices();
		}
	}

	V_clip[0] = V_clip[1];
	V_clip[1] =  1.;
	if ( maxz >= V_clip[0] ) {
		vertices();
	}
#else
	vertices();
#endif
}
//----------------------------------------------------------------------------

