struct vs_input_t
{
    float3 localPosition : POSITION;
    float4 color         : COLOR;
    float2 uv            : TEXCOORD;
}; 

//----------------------------------------------------------------------------------------------------------------------
struct v2p_t
{
    float4 position : SV_Position; 
    float4 color    : COLOR;
    float2 uv       : TEXCOORD;
};

//----------------------------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
    float4x4 ProjectionMatrix;
    float4x4 ViewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
cbuffer SimpleMinerGameConstants : register(b8)
{
    float4  c_indoorLightColor;        // gameCamera-to-clip ( includes gameCamera->screenCamera axis swaps )
 //   float4  c_outdoorLightColor;       // gameCamera-to-clip ( includes gameCamera->screenCamera axis swaps )
//    float4  c_fogColor;              // Fog color to blend in
//    float4  c_fogStartDistance;      // World units away where fog begins    (0%)
//    float4  c_fogEndDistance;        // World units away where fog maxes out (100%)
//    float4  c_dummyPadding1;
//    float4  c_dummyPadding2;
}

//----------------------------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
    float4x4 ModelMatrix;
    float4 ModelColor;
}

//----------------------------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);

//----------------------------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);

//----------------------------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
    float4 localPosition = float4( input.localPosition, 1 );
    float4 worldPosition = mul( ModelMatrix, localPosition );
    float4 viewPosition  = mul( ViewMatrix, worldPosition );    
    float4 clipPosition  = mul( ProjectionMatrix, viewPosition );

    v2p_t v2p;
    v2p.position = clipPosition;
    v2p.color = input.color;
    v2p.uv = input.uv;
    return v2p;
}

//----------------------------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
//    float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
//    float4 vertexColor  = input.color;
//    float4 modelColor   = ModelColor;
//    float4 color        = ( textureColor * vertexColor * modelColor );
//    clip( color.a - 0.01f );
//    color.g = 1;
//    return float4( color );

    float4 textureColor		        = diffuseTexture.Sample(diffuseSampler, input.uv);      // Get texture color
    float4 rgbEncodedData	        = input.color;                                          // rgbEncodedData is the color with lighting values passed in, ( outdoorLightInfluence, indoorLightInfluence, 127, 1 )
    float4 modelColor		        = ModelColor;
    float indoorLightBrightness		= rgbEncodedData.g;                                     // indoorLightBrightness "scales" the "brightness" of the texture color          
    float3 indoorLightTint			= indoorLightBrightness * c_indoorLightColor.rgb;       // LOW indoorLightBrightness will make the color DARK and HIGH indoorLightBrightness will make the color BRIGHT
    
    // Combine colors
    float4 tint		                = float4( indoorLightTint, 1 ) * textureColor;
    float4 color	                = tint * modelColor;
    return float4(color);
}