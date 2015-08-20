//----------------------------------------------------------------------------
// uniforms
//----------------------------------------------------------------------------
uniform vec2 u_destDimensions;
uniform float u_near;
uniform float u_far;
uniform float u_tanHalfFovy;
uniform float u_aspectRatio;
// min max shadow
uniform sampler2D u_minDepth;
uniform sampler2D u_maxDepth;
//----------------------------------------------------------------------------
// varying
//----------------------------------------------------------------------------
varying vec4 v_viewPos;
//----------------------------------------------------------------------------
// constants
//----------------------------------------------------------------------------
const float PI = 3.14159265358979323846264;
//----------------------------------------------------------------------------
vec2 disc[] = vec2[](
	vec2( -0.08017,  0.33915 ),
	vec2(  0.30062, -0.92673 ),
	vec2(  0.94818,  0.08675 ),
	vec2( -0.89223, -0.40924 ),
	vec2( -0.85619,  0.50641 ),
	vec2(  0.39917,  0.91403 ),
	vec2(  0.31172, -0.22781 ),
	vec2( -0.22715, -0.59419 ),
	vec2( -0.37202,  0.83208 ),
	vec2( -0.55927,  0.02883 ),
	vec2(  0.82987,  0.53371 ),
	vec2(  0.79505, -0.58505 ),
	vec2(  0.41486,  0.29644 ),
	vec2( -0.99355,  0.10314 ),
	vec2( -0.13521, -0.10352 ),
	vec2( -0.63228, -0.73680 ),
	vec2( -0.00696,  0.70388 ),
	vec2(  0.23451, -0.57765 ),
	vec2( -0.04103, -0.94567 ),
	vec2( -0.49879,  0.42438 )
);
//----------------------------------------------------------------------------
void main() {
	vec2 coord = gl_FragCoord.xy / u_destDimensions;

	float f = 1. / u_tanHalfFovy;

	float mind = 1.;
	for ( int i = 0; i < 20; ++i ) {
		float r = fract( sin( dot( coord, vec2( 12.9898, 98.133 ) * ( i + 1 ) ) ) * 43758.53 );
		mat2 m = mat2( cos( r * PI * 2 ), -sin( r * PI * 2 ), sin( r * PI * 2 ), cos( r * PI * 2 ) );

		vec3 offset = vec3( m * disc[ i ], 0. ) / 2.;
		vec3 pos = v_viewPos.xyz + offset;
		float l = length( offset );

		vec2 uv = .5 - vec2( pos.x / u_aspectRatio, pos.y ) * ( .5 * f / pos.z );
		float z = ( u_near + pos.z ) / ( u_near - u_far );

		float minz = texture2D( u_minDepth, uv ).r;
		float maxz = texture2D( u_maxDepth, uv ).r;

		if ( minz > 0. && maxz < 1. ) {
			float dz = max( max( z - maxz, minz - z ), 0. ) * ( u_far - u_near );

			mind = min( mind, sqrt( dz * dz + l * l ) * 2. );
		}
	}

	gl_FragDepth = mind;
}
//----------------------------------------------------------------------------
