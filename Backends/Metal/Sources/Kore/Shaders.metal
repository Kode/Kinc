#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct uniforms_vertex {
	matrix_float4x4 projectionMatrix;
};

struct uniforms_fragment {
	//sampler2D tex;
};

struct vertex_in {
	packed_float3 vertexPosition;
	packed_float2 texPosition;
	packed_float4 vertexColor;
};

struct fragment_in {
	float4 position [[position]];
	float2 texCoord;
	float4 color;
};

vertex fragment_in kore_vertex(device vertex_in* vertices [[buffer(0)]],
							   constant uniforms_vertex& uniforms [[buffer(1)]],
							   unsigned int vid [[vertex_id]]) {
	fragment_in out;
	out.position = uniforms.projectionMatrix * float4(float3(vertices[vid].vertexPosition), 1.0);
	//float3 pos = float3(vertices[vid].vertexPosition);
	//out.position.x = pos.x / 1000.0;
	//out.position.y = pos.y / 1000.0;
	out.position.z = 0.5f;
	out.texCoord = vertices[vid].texPosition;
	out.color = vertices[vid].vertexColor;
	return out;
}

fragment float4 kore_fragment(constant uniforms_fragment& uniforms [[buffer(0)]], fragment_in in [[stage_in]], texture2d<float> tex [[texture(0)]], sampler texSampler [[sampler(0)]]) {
	float4 texcolor = tex.sample(texSampler, in.texCoord);
	//half4 texcolor = half4(1, 0, 0, 1); //texture2D(tex, texCoord) * color;
	texcolor.rgb *= in.color.a;
	return texcolor;
	//return float4(1.0, 0.0, 0.0, 1.0);
}
