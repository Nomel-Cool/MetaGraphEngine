#version 330 core
layout (location = 0) in vec3 aPos;// pos 0 is for pos data.
layout (location = 1) in vec3 aColor;// pos 1 is for color data.
layout (location = 2) in vec2 aTexCoord;// pos 2 is for texture ccordinates.
layout (location = 3) in vec3 aNormVec; // 该顶点在X分量上的法向量（Damn，无非即使(1,0,0)或(-1,0,0)嘛)

uniform mat3 normModelMatrix; // 用于修正法线
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 colorPos;
out vec3 vertexColor;
out vec2 TexCoord;

// 用于计算 漫反射光照
out vec3 Norm;
out vec3 FragPos; // 告诉片段着色器，像素的世界坐标

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0); // (x,y,z,w), w 是相机到荧幕的距离用于透视除法，这里乘上了 MVP 矩阵使得它进入了裁剪空间（后续变NDC）
   //gl_Position = vec4(aPos.x + x_displacement, -aPos.y, aPos.z, 1.0); // 这样让三角形上下颠倒了，记住它是围绕原点的，只需要置y相反数即可
   //vertexColor = aColor;
   TexCoord = aTexCoord;
   // 将法向量变换到世界空间（修正法向量），model矩阵执行了不等比缩放，法向量将不再合乎这个面片，逆再转置model矩阵再作用到法向量可以修正对模型进行变换后法向量的跟随（使仍垂直于面片）
   Norm = normModelMatrix * aNormVec;
   FragPos = vec3(model * vec4(aPos, 1.0)); // 注意这里使用vec3把w分量消除，因为片段不需要对坐标进行变换，（已经完成透视除法）
}