Texture2DMS<float>  Input;
sampler Sampler;

float4 main(float4 pos : SV_POSITION,
    float2 tex : TEXCOORD) : SV_TARGET
{
    uint width, height, sampleCount;
    Input.GetDimensions(width, height, sampleCount);
    int2 coord = int2(width * tex.x, height * tex.y);

    // This is a terrible resolve and shouldn't be used for anything
    // where we care to maintain the result
    return Input.Load(coord, sampleCount / 2);
}