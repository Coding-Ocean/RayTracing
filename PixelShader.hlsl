Texture2D<float4> Texture : register(t0); //テクスチャ0番
SamplerState Sampler : register(s0); //サンプラ0番
void main(
    in float4 i_pos : SV_POSITION,
    in float2 i_uv : TEXCOORD,
    out float4 o_col : SV_TARGET)
{
    o_col = float4(Texture.Sample(Sampler, i_uv));
}
