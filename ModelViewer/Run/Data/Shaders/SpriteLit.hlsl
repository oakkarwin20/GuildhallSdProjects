struct vs_input_t
 {
	float3 localPosition : POSITION;
	float4 color         : COLOR;
	float2 uv            : TEXCOORD;
	float3 localTangent  : TANGENT;
	float3 localbiNormal : BINORMAL;
	float3 localNormal   : NORMAL;
 };
 
//----------------------------------------------------------------------------------------------------------------------
 struct v2p_t
 {
	float4 position : SV_Position;
	float4 color    : COLOR;
	float2 uv       : TEXCOORD;
	float4 tangent  : TANGENT;
	float4 biNormal : BINORMAL;
	float4 normal   : NORMAL;
 };
 
//----------------------------------------------------------------------------------------------------------------------
cbuffer LightingConstants : register(b1)
{
	float3 SunDirection;
	float  SunIntensity;
	float  AmbientIntensity;
}

//----------------------------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
    float4x4 ProjectionMatrix;
    float4x4 ViewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
    float4x4 ModelMatrix;
    float4   ModelColor;
}

//----------------------------------------------------------------------------------------------------------------------
Texture2D       diffuseTexture : register(t0);

//----------------------------------------------------------------------------------------------------------------------
SamplerState    diffuseSampler : register(s0);

//----------------------------------------------------------------------------------------------------------------------
 v2p_t VertexMain(vs_input_t input)
 {
    float4 localPosition = float4( input.localPosition, 1 );
    float4 worldPosition = mul(      ModelMatrix, localPosition );
    float4 viewPosition  = mul(       ViewMatrix, worldPosition );    
    float4 clipPosition  = mul( ProjectionMatrix, viewPosition  ); 

    // Compute world Tangent
    float4 localTangent = float4( input.localTangent, 0 );
    float4 worldTangent = mul( ModelMatrix, localTangent );
    
    // Compute world biNormal
    float4 localbiNormal  = float4( input.localbiNormal, 0 );
    float4 worldbiNormal  = mul( ModelMatrix, localbiNormal );

    // Compute world Normal
    float4 localNormal  = float4( input.localNormal, 0 );
    float4 worldNormal  = mul( ModelMatrix, localNormal );

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color    = input.color;
	v2p.uv       = input.uv;
	v2p.tangent  = worldTangent;
	v2p.biNormal = worldbiNormal;
	v2p.normal   = worldNormal;
 	return v2p;
}


//----------------------------------------------------------------------------------------------------------------------
 float4 PixelMain(v2p_t input) : SV_Target0
 {
	float ambient       = AmbientIntensity;
	float directional   = SunIntensity * saturate(dot(normalize(input.normal.xyz), -SunDirection));
	float4 lightColor   = float4((ambient + directional).xxx, 1);
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor  = input.color;
	float4 modelColor   = ModelColor;
	float4 color        = lightColor * textureColor * vertexColor * modelColor;
	clip(color.a - 0.01f);
	return color;


/*
    //----------------------------------------------------------------------------------------------------------------------
    // Compute TBN values only if normalMode
    // Else, compute only normal
    float3 worldNormal;
    if ( normalMode == 0 )
    {
        float3x3 Matrix_TBN     = float3x3( normalize( input.worldTangent.xyz ), normalize( input.worldbiNormal.xyz ), normalize( input.worldNormal.xyz ) );
        float3   tangentNormal  = ( 2.0f * normalTexture.Sample( samplerState, input.uv ).rgb, - 1.0f );
        worldNormal             = mul( tangentNormal, Matrix_TBN );
    }
    else
    {
        worldNormal = normalize( input.worldNormal.xyz );
    }
	//----------------------------------------------------------------------------------------------------------------------
    // Compute specular intensity only if specularMode is on
    float specularIntensity = 0.0f;
    float specularPower     = 0.0f;
    if ( specularMode == 0 )
    {
        float3 specGlossEmit = specGlossEmitTexture.Sample( samplerState, input.uv ).rgb;
        specularIntensity    = specGlossEmit.r;
        specularPower        = 31.0f * specGlossEmit.g + 1.0f;   
    }
    else
    {
        specularIntensity    = specularIntensity;
        specularPower        = specularPower;
    }

    //----------------------------------------------------------------------------------------------------------------------
    float3 worldViewDirection          = normalize( worldEyePosition - input.worldPosition.xyz );
    float3 worldHalfVector             = normalize(    -sunDirection + worldViewDirection );
    float  nDotH                       =  saturate( dot( worldNormal, worldHalfVector ) );
    float  specular                    = pow( nDotH, specularPower ) * specularIntensity;
    
    float  ambient                      = AmbientIntensity;
    float  directional                  = specularIntensity * ( saturate( dot( normalize(worldNormal), -SunDirection ) ) );
    float4 lightColor                   = saturate( float4( (ambient + directional + specular).xxx, 1 ) );
    float4 vertexColor                  = input.color;
    float4 modelColor                   = ModelColor;
    float4 textureColor                 = diffuseTexture.Sample( diffuseSampler, input.uv );
    float4 color                        = lightColor * textureColor * vertexColor * modelColor;
    clip( color.a - 0.01f );
    return color;
*/
 
/*
    float4 normalizedInputNormal       = normalize( input.normal ); 
    float  dotResult                   = dot( normalizedInputNormal.xyz, -SunDirection );
    float  clampedValue                = saturate( dotResult );
    float  directionalLightIntensity   = mul( clampedValue, SunIntensity );
    float  totalLightIntensity         = directionalLightIntensity + AmbientIntensity;
    float  clampedTotalLightIntensity  = saturate( totalLightIntensity );
    float4 lightColor                  = float4( clampedTotalLightIntensity, clampedTotalLightIntensity, clampedTotalLightIntensity, 1 );

    float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
    float4 vertexColor  = input.color;
    float4 modelColor   = ModelColor;
    float4 color        = ( textureColor * vertexColor * modelColor * lightColor );
    clip( color.a - 0.99f );
    return float4( color );
*/
 }