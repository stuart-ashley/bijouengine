//----------------------------------------------------------------------------
// packed floats
//----------------------------------------------------------------------------
#ifdef RGBA_DEPTH
uniform vec3 rgbaToFloat;
uniform sampler2D Texture;

vec4 packFloat1( float x ) {
	vec4 shift = vec4( 1., rgbaToFloat.x, rgbaToFloat.x * rgbaToFloat.y, rgbaToFloat.x * rgbaToFloat.y * rgbaToFloat.z );
	vec4 mask = vec4( 1. / rgbaToFloat.x, 1. / rgbaToFloat.y, 1. / rgbaToFloat.z, 0.);
	vec4 res = fract( x * shift );
	res -= res.yzww * mask;
	return res;
}

float unpackFloat1( vec4 value ) {
	vec4 shift = vec4( 1., 1. / rgbaToFloat.x, 1. / rgbaToFloat.x / rgbaToFloat.y, 1. / rgbaToFloat.x / rgbaToFloat.y / rgbaToFloat.z );
	return dot( value, shift );
}

float tap( vec2 coord ) {
	return unpackFloat1( texture2D( Texture, coord ) );
}
#else
uniform sampler2D Texture;

float tap( vec2 coord ) {
	return texture2D( Texture, coord ).r;
}
#endif
//----------------------------------------------------------------------------
varying vec2 v_uv;
uniform vec2 SrcDimensions;
uniform float u_blurSize;
//----------------------------------------------------------------------------
void main()
{
	float blurSize = u_blurSize / SrcDimensions.y;

	float sum = 0.;
	sum += tap( vec2( v_uv.x, v_uv.y - 7.0 * blurSize ) ) * 0.0122583126287;
	sum += tap( vec2( v_uv.x, v_uv.y - 6.0 * blurSize ) ) * 0.0228821835736;
	sum += tap( vec2( v_uv.x, v_uv.y - 5.0 * blurSize ) ) * 0.0386136847805;
	sum += tap( vec2( v_uv.x, v_uv.y - 4.0 * blurSize ) ) * 0.0590562237819;
	sum += tap( vec2( v_uv.x, v_uv.y - 3.0 * blurSize ) ) * 0.0820225330304;
	sum += tap( vec2( v_uv.x, v_uv.y - 2.0 * blurSize ) ) * 0.103607410144;
	sum += tap( vec2( v_uv.x, v_uv.y - 1.0 * blurSize ) ) * 0.119148521665;
	sum += tap( vec2( v_uv.x, v_uv.y - 0.0 * blurSize ) ) * 0.124822260792;
	sum += tap( vec2( v_uv.x, v_uv.y + 1.0 * blurSize ) ) * 0.119148521665;
	sum += tap( vec2( v_uv.x, v_uv.y + 2.0 * blurSize ) ) * 0.103607410144;
	sum += tap( vec2( v_uv.x, v_uv.y + 3.0 * blurSize ) ) * 0.0820225330304;
	sum += tap( vec2( v_uv.x, v_uv.y + 4.0 * blurSize ) ) * 0.0590562237819;
	sum += tap( vec2( v_uv.x, v_uv.y + 5.0 * blurSize ) ) * 0.0386136847805;
	sum += tap( vec2( v_uv.x, v_uv.y + 6.0 * blurSize ) ) * 0.0228821835736;
	sum += tap( vec2( v_uv.x, v_uv.y + 7.0 * blurSize ) ) * 0.0122583126287;

#ifdef RGBA_DEPTH
	gl_FragColor = packFloat1( sum );
#else
	gl_FragDepth = clamp( sum, 0., 1. );
#endif
}
//----------------------------------------------------------------------------
