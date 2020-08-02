Texture2D Input;
sampler Sampler;

float4 main(float4 pos : SV_POSITION,
            float2 tex : TEXCOORD) : SV_TARGET
{
  float4 temp = Input.Sample(Sampler, tex);
  return float4(temp.rrr, 1.0f);
}