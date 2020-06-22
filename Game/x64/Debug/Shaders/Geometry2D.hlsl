struct VertexIn{
    float3 Pos : POSITION;
    float4 Color : COLOR;
};

struct VertexOut{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

cbuffer Proj : register(b0){
    float4x4 worldProj;
};

VertexOut VS(VertexIn input){

    VertexOut output;
    output.Pos = mul( worldProj , float4(input.Pos,1.f));
    output.Color = input.Color;

    return output;
}

float4 PS(VertexOut input) : SV_TARGET{
    //return mul(worldProj , float4(0,0,0,-0.1));
    return input.Color;
}