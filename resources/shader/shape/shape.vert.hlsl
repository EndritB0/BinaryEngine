cbuffer CameraBuffer : register(b0, space1)
{
	float4x4 u_ViewProjection;
};

struct VertexInput
{
	float2 Position         : TEXCOORD0;
	float2 LocalPosition    : TEXCOORD1;
	float2 HalfSize         : TEXCOORD2;
	float4 Color            : TEXCOORD3;
	float  OutlineThickness : TEXCOORD4;
	uint   Type             : TEXCOORD5;
};

struct VertexOutput
{
	float4 Color                           : TEXCOORD0;
	float2 LocalPosition                   : TEXCOORD1;
	nointerpolation float2 HalfSize        : TEXCOORD2;
	nointerpolation float OutlineThickness : TEXCOORD3;
	nointerpolation uint Type              : TEXCOORD4;
	float4 Position                        : SV_Position;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;

	float4 clip = mul(u_ViewProjection, float4(input.Position, 0.0f, 1.0f));

	output.Position = float4(clip.xy, 0.0f, clip.w);
	output.LocalPosition = input.LocalPosition;
	output.HalfSize = input.HalfSize;
	output.Color = input.Color;
	output.OutlineThickness = input.OutlineThickness;
	output.Type = input.Type;

	return output;
}
