//----------------------------------------------------------------------------
// uniforms
//----------------------------------------------------------------------------
uniform vec3 rgbaToFloat;
uniform float u_near;
uniform float u_far;
uniform sampler2D u_alphaMap;
uniform float u_readRadius;
//----------------------------------------------------------------------------
// varying
//----------------------------------------------------------------------------
varying vec4 v_viewPos;
varying vec2 v_alphaUV;
//----------------------------------------------------------------------------
// packed floats
//----------------------------------------------------------------------------
vec4 packFloat1( float x ) {
	vec4 shift = vec4( 1., rgbaToFloat.x, rgbaToFloat.x * rgbaToFloat.y, rgbaToFloat.x * rgbaToFloat.y * rgbaToFloat.z );
	vec4 mask = vec4( 1. / rgbaToFloat.x, 1. / rgbaToFloat.y, 1. / rgbaToFloat.z, 0.);
	vec4 res = fract( x * shift );
	res -= res.yzww * mask;
	return res;
}

vec4 packFloat( in float x ) {
	float l = log2( x );
	float e = ceil( l );
//	if ( e == l ) e += 1.;
	e = max( e, -128. );
	float f = x * exp2( -e );

	vec3 a = vec3( 1., 256., 256. * 256. );
	vec3 b = vec3( 1. / 256., 1. / 256., 0. );
	vec3 r = fract( f * a );
	r -= r.xxy * b;

	return vec4( r, ( e + 128. ) / 256. );
}
//----------------------------------------------------------------------------
vec3 GetFaceNormal( vec3 vsp ) {
	vec3 faceNormal = normalize( cross( dFdx( vsp ), dFdy( vsp ) ) );
	/* make sure normal pointing right way */
	faceNormal *= -sign( dot( faceNormal, vsp ) );
	return faceNormal;
}
//----------------------------------------------------------------------------
void main()
{
#ifdef ALPHAMAP
	if ( texture2D( u_alphaMap, v_alphaUV ).a < .5 ) discard;
#endif

#if defined( ORTHO )
	// offset
	vec3 faceNormal = GetFaceNormal( v_viewPos.xyz );
	float offset = u_readRadius * sqrt( 1. - faceNormal.z * faceNormal.z ) / faceNormal.z;
	// normalized depth
	float z = ( v_viewPos.z  - offset + .5 * ( u_far + u_near ) ) / ( u_near - u_far ) + .5;
#else
	// normalized linear depth
	float z = ( 1. / gl_FragCoord.w - u_near ) / ( u_far - u_near );
#endif

#if defined( RGBA_DEPTH )
	gl_FragColor = packFloat1( z );
#else
	gl_FragDepth = z;
#endif
}
//----------------------------------------------------------------------------
