#version 400

layout(location=0) in vec4 VertexPos;
layout(location=1) in vec4 VertexNormal;
layout(location=2) in vec2 VertexTexcoord;

out vec3 Position;
out vec3 Normal;
out vec2 Texcoord;

uniform mat4 ModelMat;
uniform mat4 ModelViewProjMat;
uniform vec3 Scaling;

void main()
{

    vec4 scaledVertexPos = vec4(VertexPos.xyz * Scaling.xyz,1);
    vec4 scaledNormal = normalize(vec4(VertexNormal.xyz/Scaling.xyz,1));


    Position = (ModelMat * scaledVertexPos).xyz;
    Normal = (ModelMat * vec4(scaledNormal.xyz,0)).xyz;
    Texcoord = VertexTexcoord;
    gl_Position = ModelViewProjMat * scaledVertexPos;
}
