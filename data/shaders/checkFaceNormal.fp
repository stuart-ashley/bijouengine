//----------------------------------------------------------------------------
// varying
//----------------------------------------------------------------------------
varying vec3 v_position;
//----------------------------------------------------------------------------
void main()
{
	vec3 n = normalize( cross( dFdx( v_position ), dFdy( v_position ) ) );
	gl_FragColor = vec4( n, 1 );
}
//----------------------------------------------------------------------------