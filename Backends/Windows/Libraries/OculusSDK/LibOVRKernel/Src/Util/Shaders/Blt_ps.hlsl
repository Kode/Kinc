Texture2D Input;
sampler Sampler;

float4 main(float4 pos : SV_POSITION,
            float2 tex : TEXCOORD) : SV_TARGET
{
    return Input.Sample(Sampler, tex);
}