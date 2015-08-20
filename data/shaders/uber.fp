//----------------------------------------------------------------------------
// packed floats
//----------------------------------------------------------------------------
uniform vec3 rgbaToFloat;

float unpackFloat1( vec4 value ) {
	vec4 shift = vec4( 1., 1. / rgbaToFloat.x, 1. / rgbaToFloat.x / rgbaToFloat.y, 1. / rgbaToFloat.x / rgbaToFloat.y / rgbaToFloat.z );
	return dot( value, shift );
}

float unpackFloat( vec4 value ) {
	vec3 shift = vec3( 1., 1. / 256., 1. / ( 256. * 256. ) );
	return dot( value.xyz, shift ) * exp2( value.w * 256. - 128. );
}
//----------------------------------------------------------------------------
// shadow setup
//----------------------------------------------------------------------------
#if defined( RGBA_DEPTH )
#define SHADOW_2D sampler2D
float shadowSample2D( sampler2D map, vec3 coords ) {
	float z = unpackFloat1( texture2D( map, coords.xy ) );
	if ( coords.z < z ) {
		return 1.;
	}
	return 0.;
}
#else
#define SHADOW_2D sampler2DShadow
float shadowSample2D( sampler2DShadow map, vec3 coords ) {
	return shadow2D( map, coords ).r;
}
#endif

#if defined( FLOAT_TEXTURES )
float floatSample2D( sampler2D map, vec2 coords ) {
	return texture2D( map, coords ).r;
}
#else
float floatSample2D( sampler2D map, vec2 coords ) {
	return unpackFloat( texture2D( map, coords ) );
}
#endif
//----------------------------------------------------------------------------
// uniforms
//----------------------------------------------------------------------------
uniform vec2 u_destDimensions;
uniform float u_near;
uniform float u_far;
uniform vec4 Color;
uniform float Specularity;
uniform float Hardness;
// texture maps
uniform sampler2D ColorMap;
uniform sampler2D NormalMap;
uniform sampler2D AlphaMap;
uniform sampler2D SpecularMap;
uniform sampler2D Reflection;
uniform sampler2D Refraction;
uniform sampler2D LightMap;
// clip planes
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane2;
uniform vec4 clipPlane3;
// amount of reflection
uniform float u_reflect;
// amount of refraction
uniform float u_refract;
// far z clip plane
uniform float zFar;
// projected texture
struct ProjectedTexture {
	mat4 texMat;
	float alpha;
	float beta;
};
// projected kaleidoscope
struct ProjectedKaleidoscope {
	mat4 texMat;
	float segmentCentralAngle;
	float alpha;
	float beta;
	vec2 uv0;
	vec2 uv1;
	vec2 uv2;
};
// spotlight projected textures
uniform ProjectedTexture spotlightProjectedTexture0;
uniform ProjectedTexture spotlightProjectedTexture1;
uniform ProjectedTexture spotlightProjectedTexture2;
uniform sampler2D spotlightProjectedTextureMap0;
uniform sampler2D spotlightProjectedTextureMap1;
uniform sampler2D spotlightProjectedTextureMap2;

// spotlights
struct Spotlight {
	vec3 point;
	vec3 normal;
	vec3 color;
	float sine;
	float distance;
};
uniform Spotlight spot0;
uniform Spotlight spot1;
uniform Spotlight spot2;

struct PerspectiveShadow {
	mat3 viewToLight;
	mat4 texMat;
};
uniform PerspectiveShadow spotShadow0;
uniform PerspectiveShadow spotShadow1;
uniform PerspectiveShadow spotShadow2;
uniform SHADOW_2D spotShadowMap0;
uniform SHADOW_2D spotShadowMap1;
uniform SHADOW_2D spotShadowMap2;
// sunlight projected textures
uniform ProjectedTexture sunlightProjectedTexture0;
uniform ProjectedTexture sunlightProjectedTexture1;
uniform ProjectedTexture sunlightProjectedTexture2;
uniform sampler2D sunlightProjectedTextureMap0;
uniform sampler2D sunlightProjectedTextureMap1;
uniform sampler2D sunlightProjectedTextureMap2;
// sunlight projected kaleidoscopes
uniform ProjectedKaleidoscope sunlightProjectedKaleidoscope;
uniform sampler2D sunlightProjectedKaleidoscopeMap;
// sunlight
struct OrthoShadow {
	mat4 texMat;
	float pixelWidth;
};
uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform mat3 sunViewToLight;   // rotate from view space to sunlight space
uniform OrthoShadow sunShadow0;
uniform OrthoShadow sunShadow1;
uniform OrthoShadow sunShadow2;
uniform SHADOW_2D sunShadowMap0;
uniform SHADOW_2D sunShadowMap1;
uniform SHADOW_2D sunShadowMap2;
// environment map
uniform mat3 u_viewToEnvmap;       // rotate from view space to envmap space
uniform vec3 u_envmapViewPosition; // envmap position in view space
uniform samplerCube u_cubeEnvmap;
uniform float u_fadeEnvmap;
uniform float u_alphaEnvmap;
uniform sampler2D HemiEnvmap;
// irradiance volume
uniform vec3 u_irradianceVolumeSize;
uniform vec3 u_irradianceVolumeExtents;
uniform mat4 u_irradianceVolumeTransform; // view space to irradiance space
uniform sampler3D u_irradianceVolume0;
uniform sampler3D u_irradianceVolume1;
uniform sampler3D u_irradianceVolume2;
uniform sampler3D u_irradianceVolume3;
uniform sampler3D u_irradianceVolume4;
uniform sampler3D u_irradianceVolume5;
uniform sampler3D u_irradianceVolume6;
// distance fade
uniform float u_fadeNear;
uniform float u_fadeFar;
uniform vec4 u_fadeColor;
// lightmap
uniform vec2 u_lightMapSize;
uniform vec2 u_lightMapTexelSize;
// offset z
uniform sampler2D u_offsetMap;
uniform float u_offsetScale;
// lighting scale
uniform float u_lightScale;
// intensity texture
uniform sampler2D u_intensityTexture;
//----------------------------------------------------------------------------
// varying
//----------------------------------------------------------------------------
varying vec3 v_position;
varying vec3 v_normal; // view space normal
varying vec4 v_color;
varying vec3 v_binormal;
varying vec2 v_coluv;
varying vec2 v_normuv;
varying vec2 v_alphauv;
varying vec2 v_specuv;
varying vec2 v_lightuv;
varying vec2 v_offsetuv;
varying float v_vertexao;
// reflection space texture coordinates
varying vec3 v_reflectionCoord;
varying float V_clip[ 2 ];
//----------------------------------------------------------------------------
// constants
//----------------------------------------------------------------------------
const float PI = 3.14159265358979323846264;
// for irradiance
const float K0 = 0.8862269254527579; // sqrt( pi / 4 )
const float K1 = 1.0233267079464885; // sqrt( pi / 3 )
const float K2 = 0.8580855308097833; // sqrt( 15 * pi / 64 )
const float K3 = 0.2477079561003757; // sqrt( 5 * pi / 256 )
const float K4 = 0.42904276540489167; // sqrt( 15 * pi / 256 )
//----------------------------------------------------------------------------
// vector inside unit cube
//----------------------------------------------------------------------------
bool inUnit( vec3 v ) {
	return all( greaterThan( v, vec3( 0., 0., 0. ) ) ) && all( lessThan( v, vec3( 1., 1., 1. ) ) );
}
//----------------------------------------------------------------------------
// clip
//----------------------------------------------------------------------------
bool clip( vec3 p ) {
#ifdef CLIP_PLANE0
	if ( dot( p, clipPlane0.xyz ) + clipPlane0.w < 0. ) {
		return true;
	}
#endif
#ifdef CLIP_PLANE1
	if ( dot( p, clipPlane1.xyz ) + clipPlane1.w < 0. ) {
		return true;
	}
#endif
#ifdef CLIP_PLANE2
	if ( dot( p, clipPlane2.xyz ) + clipPlane2.w < 0. ) {
		return true;
	}
#endif
#ifdef CLIP_PLANE3
	if ( dot( p, clipPlane3.xyz ) + clipPlane3.w < 0. ) {
		return true;
	}
#endif
	return false;
}
//----------------------------------------------------------------------------
// alpha
//----------------------------------------------------------------------------
float GetAlpha() {
#ifdef ALPHAMAP
	return texture2D( AlphaMap, v_alphauv ).g;
#else
	return 1.;
#endif
}
//----------------------------------------------------------------------------
// color
//----------------------------------------------------------------------------
vec3 GetTangentNormal();

vec2 reflectionUV() {
	return ( v_reflectionCoord.xy + GetTangentNormal().xy * GetAlpha() ) / v_reflectionCoord.z;
}

float reflectionValue( vec3 v, vec3 n ) {
#if defined( MIRROR ) && defined( REFRACTION )
	float f = u_reflect + ( 1. - u_reflect ) * pow( 1. - dot( v, n ), 5. );
	return clamp( f, 0., 1. );
#else
	return u_reflect;
#endif
}

vec4 GetColor( vec3 v, vec3 viewSpaceNormal ) {
#if defined( COLORMAP ) || defined( MIRROR ) || defined( REFRACTION )
	vec4 color = vec4( 0. );
#else
	vec4 color = vec4( 1. );
#endif

#if defined( COLORMAP )
	color = texture2D( ColorMap, v_coluv );
#endif

#if defined ( VERTEX_COLOR )
	color = color * v_color;
#endif

#if defined( COLOR )
	color = color * Color;
#endif

	return color;
}
//----------------------------------------------------------------------------
// face normal
// vsp - view space position
//----------------------------------------------------------------------------
vec3 GetFaceNormal( vec3 vsp ) {
	vec3 faceNormal = normalize( cross( dFdx( vsp ), dFdy( vsp ) ) );
	/* make sure normal pointing right way */
	faceNormal *= -sign( dot( faceNormal, vsp ) );
	return faceNormal;
}
//----------------------------------------------------------------------------
// normal
// vsp - view space position
//----------------------------------------------------------------------------
vec3 GetNormal( vec3 vsp ) {
#if defined( NORMALMAP )
	vec3 n = texture2D( NormalMap, v_normuv ).xyz;
	n = n * 2. - 1.;
	/************************************************************************/
	/* opengl texture coordinates are origin bottom left                    */
	/* gimp texture coordinates are origin top left                         */
	/* so gimp and opengl have y in opposite directions                     */
	/* for texture coordinates this is fixed in the exporter                */
	/* but for normals this is fixed here                                   */
	/*                                                                      */
	/* could invert Y during normal map generation in gimp, but one day     */
	/* i'd forget and be left wondering why the normals were wrong          */
	/************************************************************************/
	n.y = -n.y;

	mat3 tangentMatrix;
	tangentMatrix[2] = normalize( v_normal );
	tangentMatrix[1] = normalize( v_binormal );
	tangentMatrix[0] = cross( tangentMatrix[ 1 ], tangentMatrix[ 2 ] );

	return normalize( tangentMatrix * n );
#elif defined( NORMAL )
	return normalize( v_normal );
#else
	return GetFaceNormal( vsp );
#endif
}
//----------------------------------------------------------------------------
// tangent space normal
//----------------------------------------------------------------------------
vec3 GetTangentNormal() {
#if defined( NORMALMAP )
	vec3 n = texture2D( NormalMap, v_normuv ).xyz;
	n = n * 2. - 1.;
	n.y = -n.y;
	return normalize( n );
#else
	return vec3( 0., 0., 1. );
#endif
}
//----------------------------------------------------------------------------
// diffuse lambert
//----------------------------------------------------------------------------
float Lambert( vec3 n, vec3 lv ) {
	return max( 0.0, dot( lv, n ) );
}
//----------------------------------------------------------------------------
// specularity
//----------------------------------------------------------------------------
float GetSpecularity() {
#if defined( SPECULARMAP )
	return texture2D( SpecularMap, v_specuv ).x;
#elif defined( SPECULAR )
	return Specularity;
#else
	return 0.;
#endif
}
//----------------------------------------------------------------------------
// phong
//
// spec - specularity
// n    - surface normal
// lv   - light direction
//
//----------------------------------------------------------------------------
float Phong( float spec, vec3 n, vec3 lv ) {
	vec3 ev = normalize( -v_position );
	vec3 rv = normalize( reflect( -lv, n ) );

	return spec * pow( max( dot( rv, ev ), 0.0 ), Hardness );
}
//----------------------------------------------------------------------------
// shadow map
//----------------------------------------------------------------------------
float ShadowMap( vec3 p, PerspectiveShadow shadow, SHADOW_2D shadowmap ) {
	float intensity = 0.;

	vec3 faceNormal = GetFaceNormal( p );
	faceNormal = shadow.viewToLight * faceNormal;

	vec4 tc = shadow.texMat * vec4( p, 1. );
	tc.xy /= tc.w;
	if ( inUnit( tc.xyz ) ) {
		intensity = shadowSample2D( shadowmap, tc.xyz );
	}

	return intensity;
}
//----------------------------------------------------------------------------
// kaleidoscope
//----------------------------------------------------------------------------
void kaleidoscope( in vec3 p, in ProjectedKaleidoscope pk, in sampler2D pkMap, inout vec3 color, inout vec3 intensity ) {
	vec4 tc = pk.texMat * vec4( p, 1. );
	tc.xyz = tc.xyz / tc.w;
	float l = length( vec2( tc.x - .5, tc.y - .5 ) );
	if ( l < 1 ) {
		float angle = atan( tc.y - .5, tc.x - .5 );
		if ( angle < 0. ) angle += 2. * PI;
		while ( angle > 2. * pk.segmentCentralAngle ) angle -= 2. * pk.segmentCentralAngle;
		if ( angle > pk.segmentCentralAngle ) angle = 2. * pk.segmentCentralAngle - angle;

		float t = angle / pk.segmentCentralAngle;

		tc.xy = pk.uv0 + ( 1 - t ) * ( pk.uv1 - pk.uv0 ) * l + t * ( pk.uv2 - pk.uv0 ) * l;

		vec3 col = texture2D( pkMap, tc.xy ).xyz;
		color *= 1. + pk.alpha * ( col - 1. );
		intensity += col * pk.beta;
	}
}
//----------------------------------------------------------------------------
// projected texture
//----------------------------------------------------------------------------
void projectedTexture( in vec3 p, in ProjectedTexture pt, in sampler2D map,
		inout vec3 color, inout vec3 intensity )
{
	vec4 tc = pt.texMat * vec4( p, 1. );
	tc.xyz = tc.xyz / tc.w;
	if ( inUnit( tc.xyz ) ) {
		vec3 col = texture2D( map, tc.xy ).xyz;
		color *= 1. + pt.alpha * ( col - 1. );
		intensity += col * pt.beta;
	}
}
//----------------------------------------------------------------------------
// pcss
//----------------------------------------------------------------------------
//float PcssShadows( vec3 p, OrthoShadow shadow, float lightSize,
//		int blkrSamples, int pcfSamples )
float PcssShadows( vec3 p, OrthoShadow shadow, SHADOW_2D shadowmap )
{
	float lightSize = 16. / shadow.pixelWidth;
	int blkrSamples = 9;
	int pcfSamples = 9;
	float intensity = 0.;

	vec4 uv = shadow.texMat * vec4( p, 1. );
	uv.xyz /= uv.w;
	if ( inUnit( uv.xyz ) ) {
		float x = 1.;
		float blkrStep = 2. * lightSize * x / blkrSamples;

		vec3 coord = uv.xyz - vec3( lightSize * x, lightSize * x, 0. );

		float sum = 0.;
		float count = 0.;
		for ( int i = 0; i < blkrSamples; ++i ) {
			for ( int j = 0; j < blkrSamples; ++j ) {
				float z = shadowSample2D( shadowmap, coord.xyz + vec3( i, j, 0. ) * blkrStep );
				if ( z < coord.z ) {
					sum += z;
					count += 1.;
				}
			}
		}

		float pcfSize = 0.;
		if ( count > 0. ) {
			pcfSize = ( uv.z * count / sum - 1. ) / 4.;
		}
		pcfSize = max( 4. / shadow.pixelWidth, pcfSize );

		intensity = 0.;
		float pcfStep = 2 * pcfSize / pcfSamples;
		coord = uv.xyz - vec3( pcfSize, pcfSize, 0. );

		for ( int j = 0; j < pcfSamples; ++j ) {
			for ( int i = 0; i < pcfSamples; ++i ) {
				float z = shadowSample2D( shadowmap, coord.xyz + vec3( i, j, 0. ) * pcfStep );
				if ( z > coord.z ) {
					intensity += 1.;
				}
			}
		}
		intensity /= pcfSamples * pcfSamples;
	}

	return intensity;
}
//----------------------------------------------------------------------------
// cascade shadows
//----------------------------------------------------------------------------
#ifdef SHADOW
float CascadeShadows( in vec3 p, in OrthoShadow shadow, SHADOW_2D shadowmap )
{
	float intensity = 0.;

	vec4 tc = shadow.texMat * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		intensity = shadowSample2D( shadowmap, tc.xyz );
	}
	return intensity;
}
#endif
//----------------------------------------------------------------------------
// cascade 2 shadows
//----------------------------------------------------------------------------
#if defined( CASCADE_SHADOWS ) && defined( CASCADE2 )
float CascadeShadows( vec3 p, OrthoShadow shadow0, SHADOW_2D shadow0map,
		OrthoShadow shadow1, SHADOW_2D shadow1map )
{
	float intensity = 0.;

	vec4 tc = shadow0.texMat * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		intensity = shadowSample2D( shadow0map, tc.xyz );
	}
	else {
		tc = shadow1.texMat * vec4( p, 1. );
		if ( inUnit( tc.xyz ) ) {
			intensity = shadowSample2D( shadow1map, tc.xyz );
		}
	}

	return intensity;
}
#endif
//----------------------------------------------------------------------------
// cascade 3 shadows
//----------------------------------------------------------------------------
#if defined( CASCADE_SHADOWS ) && defined( CASCADE3 )
float CascadeShadows( vec3 p, OrthoShadow shadow0, SHADOW_2D shadow0map,
		OrthoShadow shadow1, SHADOW_2D shadow1map,
		OrthoShadow shadow2, SHADOW_2D shadow2map )
{
	float intensity = 0.;

	vec4 tc = shadow0.texMat * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		intensity = shadowSample2D( shadow0map, tc.xyz );
	}
	else {
		tc = shadow1.texMat * vec4( p, 1. );
		if ( inUnit( tc.xyz ) ) {
			intensity = shadowSample2D( shadow1map, tc.xyz );
		}
		else {
			tc = shadow2.texMat * vec4( p, 1. );
			if ( inUnit( tc.xyz ) ) {
				intensity = shadowSample2D( shadow2map, tc.xyz );
			}
		}
	}

	return intensity;
}
#endif
//----------------------------------------------------------------------------
// cascade region
//----------------------------------------------------------------------------
#if defined( CASCADE_REGIONS )
vec4 CascadeRegions( vec3 p, mat4 texMat0 )
{
	vec4 color = vec4( 0., 0., 0., 1. );

	vec4 tc = texMat0 * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		color.r = 1.;
	}

	return color;
}
#endif
//----------------------------------------------------------------------------
// cascade 2 regions
//----------------------------------------------------------------------------
#if defined( CASCADE_REGIONS ) && defined( CASCADE2 )
vec4 CascadeRegions( vec3 p, mat4 texMat0, mat4 texMat1 )
{
	vec4 color = vec4( 0., 0., 0., 1. );

	vec4 tc = texMat0 * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		color.r = 1.;
	}
	else {
		tc = texMat1 * vec4( p, 1. );
		if ( inUnit( tc.xyz ) ) {
			color.g = 1.;
		}
	}

	return color;
}
#endif
//----------------------------------------------------------------------------
// cascade 3 regions
//----------------------------------------------------------------------------
#if defined( CASCADE_REGIONS ) && defined( CASCADE3 )
vec4 CascadeRegions( vec3 p, mat4 texMat0, mat4 texMat1, mat4 texMat2 )
{
	vec4 color = vec4( 0., 0., 0., 1. );

	vec4 tc = texMat0 * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		color.r = 1.;
	}
	else {
		tc = texMat1 * vec4( p, 1. );
		if ( inUnit( tc.xyz ) ) {
			color.g = 1.;
		}
		else {
			tc = texMat2 * vec4( p, 1. );
			if ( inUnit( tc.xyz ) ) {
				color.b = 1.;
			}
		}
	}

	return color;
}
#endif
//----------------------------------------------------------------------------
// pcf shadow
//----------------------------------------------------------------------------
#ifdef CASCADE_PCF_SHADOWS
float pcfSample( in OrthoShadow shadow, in SHADOW_2D shadowmap, in vec3 tc, in float nz ) {
	float intensity = 0.;
	vec3 offset;

	float r = 3. * nz;
	float s = ( r * 2. + 1. ) / 7.;

	offset.z = 0;

	offset.y = -r / shadow.pixelWidth;
	for ( int j = 0; j < 7; ++j ) {
		offset.x = -r / shadow.pixelWidth;
		for ( int i = 0; i < 7; ++i ) {
			intensity += shadowSample2D( shadowmap, tc + offset );
			offset.x += s / shadow.pixelWidth;
		}
		offset.y += s / shadow.pixelWidth;
	}

	return intensity * 0.02;
}
#endif

#if defined( CASCADE_PCF_SHADOWS ) && !defined( CASCADE2 ) && !defined( CASCADE3 )
float CascadePcfShadows( vec3 p, float nz,
		OrthoShadow shadow, SHADOW_2D shadowmap )
{
//	return PcssShadows( p, shadow );
	float intensity = 0.;

	vec4 tc = shadow.texMat * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		intensity = pcfSample( shadow, shadowmap, tc.xyz, nz );
	}
	return intensity;
}
#endif
//----------------------------------------------------------------------------
// cascade 2 shadows pcf
//----------------------------------------------------------------------------
#if defined( CASCADE_PCF_SHADOWS ) && defined( CASCADE2 )
float CascadePcfShadows( vec3 p, float nz,
		OrthoShadow shadow0, SHADOW_2D shadow0map,
		OrthoShadow shadow1, SHADOW_2D shadow1map )
{
	float intensity = 0.;

	vec4 tc = shadow0.texMat * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		intensity = pcfSample( shadow0, shadow0map, tc.xyz, nz );
	}
	else {
		tc = shadow1.texMat * vec4( p, 1. );
		if ( inUnit( tc.xyz ) ) {
			intensity = shadowSample2D( shadow1map, tc.xyz );
		}
	}

	return intensity;
}
#endif
//----------------------------------------------------------------------------
// cascade 3 shadows pcf
//----------------------------------------------------------------------------
#if defined( CASCADE_PCF_SHADOWS ) && defined( CASCADE3 )
float CascadePcfShadows( vec3 p, float nz,
		OrthoShadow shadow0, SHADOW_2D shadow0map,
		OrthoShadow shadow1, SHADOW_2D shadow1map,
		OrthoShadow shadow2, SHADOW_2D shadow2map )
{
	float intensity = 0.;

	vec4 tc = shadow0.texMat * vec4( p, 1. );
	if ( inUnit( tc.xyz ) ) {
		intensity = pcfSample( shadow0, shadow0map, tc.xyz, nz );
	}
	else {
		tc = shadow1.texMat * vec4( p, 1. );
		if ( inUnit( tc.xyz ) ) {
			intensity = shadowSample2D( shadow1map, tc.xyz );
		}
		else {
			tc = shadow2.texMat * vec4( p, 1. );
			if ( inUnit( tc.xyz ) ) {
				intensity = shadowSample2D( shadow2map, tc.xyz );
			}
		}
	}

	return intensity;
}
#endif
//----------------------------------------------------------------------------
// spotlight
//
// p      - fragment position in view space
// n      - fragment normal in view space
//----------------------------------------------------------------------------
#if defined( SPOTLIGHT0 ) || defined( SPOTLIGHT1 ) || defined( SPOTLIGHT2 )
vec3 spotlight( vec3 p, vec3 n, Spotlight spot, PerspectiveShadow spotShadow,
		SHADOW_2D spotShadowmap )
{
	vec3 lv = normalize( p - spot.point );

	vec3 intensity = vec3( 0. );

	vec3 color = spot.color;

	// distance from 'point' to 'pos' along 'normal'
	float d = dot( p - spot.point, spot.normal );
	// length from 'pos' to nearest point on 'normal'
	float l = length( spot.point + d * spot.normal - p );
	float b = clamp( 1. - pow( l / ( d * spot.sine ), 2. ), 0., 1. );

#ifdef SPOTLIGHT_PROJECTED_TEXTURE0
	projectedTexture( p, spotlightProjectedTexture0,
			spotlightProjectedTextureMap0, color, intensity );
#endif

#ifdef SPOTLIGHT_PROJECTED_TEXTURE1
	projectedTexture( p, spotlightProjectedTexture1,
			spotlightProjectedTextureMap1, color, intensity );
#endif

#ifdef SPOTLIGHT_PROJECTED_TEXTURE2
	projectedTexture( p, spotlightProjectedTexture2,
			spotlightProjectedTextureMap2, color, intensity );
#endif

#ifdef LAMBERT
	intensity += color * b * Lambert( n, -lv );
#endif

#ifdef PHONG
	float specularity = GetSpecularity();
	intensity += color * b * Phong( specularity, n, -lv );
#endif

	return intensity * ShadowMap( p, spotShadow, spotShadowmap );
}
#endif // defined( SPOTLIGHT0 ) || defined( SPOTLIGHT1 ) || defined( SPOTLIGHT2 )
//----------------------------------------------------------------------------
// sunlight
//
// p         - fragment position in view space
// n         - fragment normal in view space
// direction - direction of sunlight in view space
// color     - color of sunlight
//----------------------------------------------------------------------------
#ifdef SUNLIGHT
vec3 doSunlight( vec3 p, vec3 n, vec3 direction, vec3 color )
{
	vec3 intensity = vec3( 0. );

#ifdef SUNLIGHT_PROJECTED_TEXTURE0
	projectedTexture( p, sunlightProjectedTexture0,
			sunlightProjectedTextureMap0, color, intensity );
#endif

#ifdef SUNLIGHT_PROJECTED_TEXTURE1
	projectedTexture( p, sunlightProjectedTexture1,
			sunlightProjectedTextureMap1, color, intensity );
#endif

#ifdef SUNLIGHT_PROJECTED_TEXTURE2
	projectedTexture( p, sunlightProjectedTexture2,
			sunlightProjectedTextureMap2, color, intensity );
#endif

#ifdef SUNLIGHT_PROJECTED_KALEIDOSCOPE0
	kaleidoscope( p, sunlightProjectedKaleidoscope, sunlightProjectedKaleidoscopeMap, color, intensity );
#endif

#ifdef LAMBERT
	intensity += color * Lambert( n, -direction );
#endif

#ifdef PHONG
	float specularity = GetSpecularity();
	intensity += color * Phong( specularity, n, -direction );
#endif

	return intensity;
}

vec3 Sunlight( vec3 p, vec3 n ) {
	vec3 intensity = doSunlight( p, n, sunDirection, sunColor );
	// face normal in light space
	vec3 fn = sunViewToLight * GetFaceNormal( p );

#if defined( CASCADE_PCF_SHADOWS ) && defined( CASCADE3 )
	return intensity * CascadePcfShadows( p, fn.z,
			sunShadow0, sunShadowMap0,
			sunShadow1, sunShadowMap1,
			sunShadow2, sunShadowMap2 );
#elif defined( CASCADE_PCF_SHADOWS ) && defined( CASCADE2 )
	return intensity * CascadePcfShadows( p, fn.z,
			sunShadow0, sunShadowMap0,
			sunShadow1, sunShadowMap1 );
#elif defined( CASCADE_PCF_SHADOWS )
	return intensity * CascadePcfShadows( p, fn.z,
			sunShadow0, sunShadowMap0 );
#elif defined( CASCADE_SHADOWS ) && defined( CASCADE3 )
	return intensity * CascadeShadows( p, sunShadow0, sunShadowMap0,
			sunShadow1, sunShadowMap1, sunShadow2, sunShadowMap2 );
#elif defined( CASCADE_SHADOWS ) && defined( CASCADE2 )
	return intensity * CascadeShadows( p, sunShadow0, sunShadowMap0,
			sunShadow1, sunShadowMap1 );
#elif defined( SHADOW )
	return intensity * CascadeShadows( p, sunShadow0, sunShadowMap0 );
#else
	return intensity;
#endif
}
#endif // SUNLIGHT
//----------------------------------------------------------------------------
// light mapping
//----------------------------------------------------------------------------
#if defined( LIGHTMAP )
vec3 lightMapSample( in vec2 uv ) {
	vec4 rgbe = texture2D( LightMap, uv );
	return rgbe.rgb * pow( 2, rgbe.a * 255. - 127. );
}

vec3 lightMapping() {
	// manual bilinear filter
	vec2 uv = vec2( v_lightuv.x, 1. - v_lightuv.y ) - u_lightMapTexelSize * .5;
	vec3 tl = lightMapSample( uv );
	vec3 tr = lightMapSample( uv + vec2( u_lightMapTexelSize.x, 0 ) );
	vec3 bl = lightMapSample( uv + vec2( 0, u_lightMapTexelSize.y ) );
	vec3 br = lightMapSample( uv + u_lightMapTexelSize );
	vec2 f = fract( uv * u_lightMapSize );
	return mix( mix( tl, tr, f.x ), mix( bl, br, f.x ), f.y );
}
#endif
//----------------------------------------------------------------------------
// envmap
//----------------------------------------------------------------------------
#if !defined( IGNORE_ENVMAP ) && defined( ENVMAP )
// lookup envmap
// p - view space position
// n - view space surface normal
vec4 EnvmapCubeLookup( vec3 p, vec3 n ) {
	vec3 v = normalize( p );   // view direction
	return textureCube( u_cubeEnvmap, u_viewToEnvmap * reflect( v, n ) ) * u_fadeEnvmap;
}

/*vec4 EnvmapHemiLookup() {
	vec3 n = normalize( v_envmapDir );
	n.xy = n.xy * -.17 / n.z;
	vec4 col = texture2D( HemiEnvmap, n.xy * .5 + .5 );
	return col * vec4( .6, .7, 1., 1. );
}*/
#endif
//----------------------------------------------------------------------------
// irradiance volume
//
// p - view space position
// n - view space normal
//----------------------------------------------------------------------------
#if defined( IRRADIANCE_VOLUME ) && defined( LIGHTMAP )
void sampleIrradianceVolume2( in vec3 p, in vec3 n1, in vec3 n2, out vec3 r1, out vec3 r2 )
{
	vec4 t0 = texture3D( u_irradianceVolume0, p );
	vec4 t1 = texture3D( u_irradianceVolume1, p );
	vec4 t2 = texture3D( u_irradianceVolume2, p );

	vec4 t3 = texture3D( u_irradianceVolume3, p );
	vec4 t4 = texture3D( u_irradianceVolume4, p );
	vec4 t5 = texture3D( u_irradianceVolume5, p );
	vec4 t6 = texture3D( u_irradianceVolume6, p );

	float s = pow( 2., t6.w * 255. - 127. + 1. );

	t0 = ( t0 - 127. / 255. ) * s;
	t1 = ( t1 - 127. / 255. ) * s;
	t2 = ( t2 - 127. / 255. ) * s;
	t3 = ( t3 - 127. / 255. ) * s;
	t4 = ( t4 - 127. / 255. ) * s;
	t5 = ( t5 - 127. / 255. ) * s;
	t6 = ( t6 - 127. / 255. ) * s;

	// first
	r1 = vec3( t0.x, t1.x, t2.x ) * K0;

	r1.r += dot( t0.yzw, n1.yzx ) * K1;
	r1.g += dot( t1.yzw, n1.yzx ) * K1;
	r1.b += dot( t2.yzw, n1.yzx ) * K1;

	r1 += t3.xyz * K2 * n1.x * n1.y;
	r1 += t4.xyz * K2 * n1.y * n1.z;
	r1 += t5.xyz * K3 * ( 3. * n1.z * n1.z - 1. );
	r1 += t6.xyz * K2 * n1.z * n1.x;
	r1 += vec3( t3.w, t4.w, t5.w ) * K4 * ( n1.x * n1.x - n1.y * n1.y );

	// second
	r2 = vec3( t0.x, t1.x, t2.x ) * K0;

	r2.r += dot( t0.yzw, n2.yzx ) * K1;
	r2.g += dot( t1.yzw, n2.yzx ) * K1;
	r2.b += dot( t2.yzw, n2.yzx ) * K1;

	r2 += t3.xyz * K2 * n2.x * n2.y;
	r2 += t4.xyz * K2 * n2.y * n2.z;
	r2 += t5.xyz * K3 * ( 3. * n2.z * n2.z - 1. );
	r2 += t6.xyz * K2 * n2.z * n2.x;
	r2 += vec3( t3.w, t4.w, t5.w ) * K4 * ( n2.x * n2.x - n2.y * n2.y );
}

void irradianceVolume2( in vec3 p, in vec3 n1, in vec3 n2, out vec3 r1, out vec3 r2 ) {
	vec3 vp = ( u_irradianceVolumeTransform * vec4( p, 1. ) ).xyz;
	vp = vp / u_irradianceVolumeExtents + .5;

	vec3 vn1 = normalize( ( u_irradianceVolumeTransform * vec4( n1, 0. ) ).xyz );
	vec3 vn2 = normalize( ( u_irradianceVolumeTransform * vec4( n2, 0. ) ).xyz );

	vec3 p0 = vp - .5 / u_irradianceVolumeSize;
	vec3 p1 = vp + .5 / u_irradianceVolumeSize;

	vec3 a000, a100, a010, a110, a001, a101, a011, a111;
	vec3 b000, b100, b010, b110, b001, b101, b011, b111;

	sampleIrradianceVolume2( p0, vn1, vn2, a000, b000 );
	sampleIrradianceVolume2( vec3( p1.x, p0.y, p0.z ), vn1, vn2, a100, b100 );
	sampleIrradianceVolume2( vec3( p0.x, p1.y, p0.z ), vn1, vn2, a010, b010 );
	sampleIrradianceVolume2( vec3( p1.x, p1.y, p0.z ), vn1, vn2, a110, b110 );
	sampleIrradianceVolume2( vec3( p0.x, p0.y, p1.z ), vn1, vn2, a001, b001 );
	sampleIrradianceVolume2( vec3( p1.x, p0.y, p1.z ), vn1, vn2, a101, b101 );
	sampleIrradianceVolume2( vec3( p0.x, p1.y, p1.z ), vn1, vn2, a011, b011 );
	sampleIrradianceVolume2( p1, vn1, vn2, a111, b111 );

	vec3 f = fract( p0 * u_irradianceVolumeSize );

	// first
	vec3 ax00 = mix( a000, a100, f.x );
	vec3 ax10 = mix( a010, a110, f.x );
	vec3 ax01 = mix( a001, a101, f.x );
	vec3 ax11 = mix( a011, a111, f.x );

	vec3 axy0 = mix( ax00, ax10, f.y );
	vec3 axy1 = mix( ax01, ax11, f.y );

	r1 = mix( axy0, axy1, f.z );

	// second
	vec3 bx00 = mix( b000, b100, f.x );
	vec3 bx10 = mix( b010, b110, f.x );
	vec3 bx01 = mix( b001, b101, f.x );
	vec3 bx11 = mix( b011, b111, f.x );

	vec3 bxy0 = mix( bx00, bx10, f.y );
	vec3 bxy1 = mix( bx01, bx11, f.y );

	r2 = mix( bxy0, bxy1, f.z );
}
#endif
#if defined( IRRADIANCE_VOLUME )
vec3 sampleIrradianceVolume( in vec3 p, in vec3 n )
{
	vec4 t0 = texture3D( u_irradianceVolume0, p );
	vec4 t1 = texture3D( u_irradianceVolume1, p );
	vec4 t2 = texture3D( u_irradianceVolume2, p );

	vec4 t3 = texture3D( u_irradianceVolume3, p );
	vec4 t4 = texture3D( u_irradianceVolume4, p );
	vec4 t5 = texture3D( u_irradianceVolume5, p );
	vec4 t6 = texture3D( u_irradianceVolume6, p );

	float s = pow( 2., t6.w * 255. - 127. + 1. );

	t0 = ( t0 - 127. / 255. ) * s;
	t1 = ( t1 - 127. / 255. ) * s;
	t2 = ( t2 - 127. / 255. ) * s;
	t3 = ( t3 - 127. / 255. ) * s;
	t4 = ( t4 - 127. / 255. ) * s;
	t5 = ( t5 - 127. / 255. ) * s;
	t6 = ( t6 - 127. / 255. ) * s;

	vec3 col = vec3( t0.x, t1.x, t2.x ) * K0;

	col.r += dot( t0.yzw, n.yzx ) * K1;
	col.g += dot( t1.yzw, n.yzx ) * K1;
	col.b += dot( t2.yzw, n.yzx ) * K1;

	col += t3.xyz * K2 * n.x * n.y;
	col += t4.xyz * K2 * n.y * n.z;
	col += t5.xyz * K3 * ( 3. * n.z * n.z - 1. );
	col += t6.xyz * K2 * n.z * n.x;
	col += vec3( t3.w, t4.w, t5.w ) * K4 * ( n.x * n.x - n.y * n.y );

	return col;
}

vec3 irradianceVolume( in vec3 p, in vec3 n ) {
	vec3 vp = ( u_irradianceVolumeTransform * vec4( p, 1. ) ).xyz;
	vec3 vn = ( u_irradianceVolumeTransform * vec4( n, 0. ) ).xyz;
	vp = vp / u_irradianceVolumeExtents + .5;
	vn = normalize( vn );

	vec3 p0 = vp - .5 / u_irradianceVolumeSize;
	vec3 p1 = vp + .5 / u_irradianceVolumeSize;

	vec3 v000 = sampleIrradianceVolume( p0, vn );
	vec3 v100 = sampleIrradianceVolume( vec3( p1.x, p0.y, p0.z ), vn );
	vec3 v010 = sampleIrradianceVolume( vec3( p0.x, p1.y, p0.z ), vn );
	vec3 v110 = sampleIrradianceVolume( vec3( p1.x, p1.y, p0.z ), vn );
	vec3 v001 = sampleIrradianceVolume( vec3( p0.x, p0.y, p1.z ), vn );
	vec3 v101 = sampleIrradianceVolume( vec3( p1.x, p0.y, p1.z ), vn );
	vec3 v011 = sampleIrradianceVolume( vec3( p0.x, p1.y, p1.z ), vn );
	vec3 v111 = sampleIrradianceVolume( p1, vn );

	vec3 f = fract( p0 * u_irradianceVolumeSize );

	vec3 vx00 = mix( v000, v100, f.x );
	vec3 vx10 = mix( v010, v110, f.x );
	vec3 vx01 = mix( v001, v101, f.x );
	vec3 vx11 = mix( v011, v111, f.x );

	vec3 vxy0 = mix( vx00, vx10, f.y );
	vec3 vxy1 = mix( vx01, vx11, f.y );

	vec3 vxyz = mix( vxy0, vxy1, f.z );

	return vxyz;
}
#endif
//----------------------------------------------------------------------------
// main
//----------------------------------------------------------------------------
void main() {
#ifdef VOLUME
	if ( gl_FragCoord.z < V_clip[ 0 ] || gl_FragCoord.z > V_clip[ 1 ] ) discard;
#endif

	if ( clip( v_position ) ) {
		discard;
	}

	vec3 p = v_position;
#ifdef OFFSETZ
	vec3 d = normalize( p );
	p += texture2D( u_offsetMap, v_offsetuv ).r * u_offsetScale * ( d / abs( d.z ) );
	gl_FragDepth = u_far * ( p.z + u_near ) / ( p.z * ( u_far - u_near ) );
#endif

	vec3 n = GetNormal( p );
	vec4 color = GetColor( normalize( -p ), n );

	vec3 intensity = vec3( 0. );

#ifdef SPOTLIGHT0
	intensity += spotlight( p, n, spot0, spotShadow0, spotShadowMap0 );
#endif

#ifdef SPOTLIGHT1
	intensity += spotlight( p, n, spot1, spotShadow1, spotShadowMap1 );
#endif

#ifdef SPOTLIGHT2
	intensity += spotlight( p, n, spot2, spotShadow2, spotShadowMap2 );
#endif

#ifdef SUNLIGHT
	intensity += Sunlight( p, n );
#endif

	// baked lighting cases
	// 1. irradiance volume only
	// 2. lightmap only
	// 3. lightmap & irradiance volume
	//    a. no normal map, lightmap will do
	//    b. normal map, augment lightmap with irradiance volume

#if !defined( IGNORE_IRRADIANCE ) && defined( IRRADIANCE_VOLUME ) && !defined( LIGHTMAP )
	intensity += irradianceVolume( p, n );
#endif

#if defined( LIGHTMAP ) && ( !defined( IRRADIANCE_VOLUME ) || !defined( NORMALMAP ) )
	intensity += lightMapping();
#endif

#if !defined( IGNORE_IRRADIANCE ) && defined( IRRADIANCE_VOLUME ) && defined( LIGHTMAP ) && defined( NORMALMAP )
	vec3 n2 = normalize( v_normal );
	if ( abs( dot( n, n2 ) ) > .99 ) {
		intensity += lightMapping();
	} else {
		vec3 a, b;
		irradianceVolume2( p, n, n2, a, b );
		intensity += a * lightMapping() / b;
	}
#endif

#ifdef SCALE_LIGHTING
	intensity *= u_lightScale;
	float l = dot( intensity, vec3( .299, .587, .114 ) );
	intensity *= sqrt( l ) * sqrt( .5 ) / l;
#endif

	vec4 totalColor = vec4( intensity, 1. ) * color;

#if defined( CASCADE_REGIONS ) && defined( CASCADE3 )
	totalColor *= CascadeRegions( p, sunTexMat0, sunTexMat1, sunTexMat2 );
#elif defined( CASCADE_REGIONS ) && defined( CASCADE2 )
	totalColor *= CascadeRegions( p, sunTexMat0, sunTexMat1 );
#elif defined( CASCADE_REGIONS )
	totalColor *= CascadeRegions( p, sunTexMat0 );
#endif

#if !defined( IGNORE_ENVMAP ) && defined( ENVMAP )
	totalColor = ( 1 - u_reflect ) * totalColor + EnvmapCubeLookup( p, n ) * u_reflect;
#endif

#if defined( MIRROR )
	totalColor += vec4( texture2D( Reflection, reflectionUV() ).xyz * reflectionValue( normalize( -p ), n ), 1. );
#endif

#if defined( REFRACTION )
	totalColor += vec4( texture2D( Refraction, reflectionUV() ).xyz * ( 1 - reflectionValue( normalize( -p ), n ) ), 1. );
#endif

#ifdef VERTEX_AO
	totalColor = vec4( v_vertexao );
#endif

#ifdef UNLIT
	totalColor = color;
#endif

#ifdef DISTANCE_FADE
	float t = ( -v_position.z - u_fadeNear ) / ( u_fadeFar - u_fadeNear );
	t = clamp( t, 0., 1. );
	totalColor = totalColor * ( 1. - t ) + u_fadeColor * t;
#endif

#ifdef WRITE_NORMAL
	totalColor = vec4( n, 1. );
#endif

#ifdef INTENSITY_TEXTURE
	totalColor *= texture2D( u_intensityTexture, gl_FragCoord.xy / u_destDimensions );
#endif

	gl_FragColor = totalColor;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
