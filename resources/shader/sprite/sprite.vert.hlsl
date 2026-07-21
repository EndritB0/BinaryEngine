cbuffer CameraBuffer : register(b0, space1)
{
	float4x4 u_ViewProjection;
};

struct VertexInput
{
	float3 Position : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
	float4 Color    : TEXCOORD2;
};

struct VertexOutput
{
	float4 Color    : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
	float4 Position : SV_Position;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	
	float4 clip = mul(u_ViewProjection, float4(input.Position, 1.0f));
	
	output.Position = float4(clip.xy, 0.0f, clip.w);
	output.TexCoord = input.TexCoord;
	output.Color = input.Color;

	return output;
}
