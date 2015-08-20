// varying
//----------------------------------------------------------------------------
varying vec2 v_uv;
//----------------------------------------------------------------------------
// uniforms
//----------------------------------------------------------------------------
uniform vec2 SrcDimensions;
uniform sampler2D Source;
uniform float u_blurSize;
//----------------------------------------------------------------------------
void main()
{
	float blurSize = u_blurSize / SrcDimensions.y;

	vec3 sum = vec3( 0. );
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y - 7.0 * blurSize ) ).rgb * 0.0122583126287;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y - 6.0 * blurSize ) ).rgb * 0.0228821835736;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y - 5.0 * blurSize ) ).rgb * 0.0386136847805;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y - 4.0 * blurSize ) ).rgb * 0.0590562237819;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y - 3.0 * blurSize ) ).rgb * 0.0820225330304;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y - 2.0 * blurSize ) ).rgb * 0.103607410144;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y - 1.0 * blurSize ) ).rgb * 0.119148521665;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y - 0.0 * blurSize ) ).rgb * 0.124822260792;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y + 1.0 * blurSize ) ).rgb * 0.119148521665;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y + 2.0 * blurSize ) ).rgb * 0.103607410144;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y + 3.0 * blurSize ) ).rgb * 0.0820225330304;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y + 4.0 * blurSize ) ).rgb * 0.0590562237819;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y + 5.0 * blurSize ) ).rgb * 0.0386136847805;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y + 6.0 * blurSize ) ).rgb * 0.0228821835736;
	sum += texture2D( Source, vec2( v_uv.x, v_uv.y + 7.0 * blurSize ) ).rgb * 0.0122583126287;

	gl_FragColor = vec4( sum, 1. );
}
//----------------------------------------------------------------------------
