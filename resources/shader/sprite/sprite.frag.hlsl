Texture2D u_Texture : register(t0, space2);
SamplerState u_Sampler : register(s0, space2);

struct FragmentInput
{
	float4 Color    : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
};

float4 main(FragmentInput input) : SV_Target
{
	return u_Texture.Sample(u_Sampler, input.TexCoord) * input.Color;
}
