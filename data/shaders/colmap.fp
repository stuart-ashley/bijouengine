//----------------------------------------------------------------------------
// uniforms
//----------------------------------------------------------------------------
uniform sampler2D Source;
uniform vec4 color;
//----------------------------------------------------------------------------
// varying
//----------------------------------------------------------------------------
varying vec2 v_uv;
//----------------------------------------------------------------------------
void main()
{
	vec4 col = texture2D( Source, v_uv );
	gl_FragColor = color * col;
}
//----------------------------------------------------------------------------
