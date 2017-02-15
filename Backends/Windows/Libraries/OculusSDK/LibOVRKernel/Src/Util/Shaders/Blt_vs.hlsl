void main(float2 pos : POSITION,
          float2 tex : TEXCOORD,
          out float4 oPos : SV_POSITION,
          out float2 oTex : TEXCOORD)
{
    oPos = float4(pos, 0, 1);
    oTex = tex;
}