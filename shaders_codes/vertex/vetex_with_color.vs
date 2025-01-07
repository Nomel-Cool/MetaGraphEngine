#version 330 core
layout (location = 0) in vec3 aPos;// pos 0 is for pos data.
layout (location = 1) in vec3 aColor;// pos 1 is for color data.
layout (location = 2) in vec2 aTexCoord;// pos 2 is for texture ccordinates.
layout (location = 3) in vec3 aNormVec; // �ö�����X�����ϵķ�������Damn���޷Ǽ�ʹ(1,0,0)��(-1,0,0)��)

uniform mat3 normModelMatrix; // ������������
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 colorPos;
out vec3 vertexColor;
out vec2 TexCoord;

// ���ڼ��� ���������
out vec3 Norm;
out vec3 FragPos; // ����Ƭ����ɫ�������ص���������

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0); // (x,y,z,w), w �������ӫĻ�ľ�������͸�ӳ�������������� MVP ����ʹ���������˲ü��ռ䣨������NDC��
   //gl_Position = vec4(aPos.x + x_displacement, -aPos.y, aPos.z, 1.0); // ���������������µߵ��ˣ���ס����Χ��ԭ��ģ�ֻ��Ҫ��y�෴������
   //vertexColor = aColor;
   TexCoord = aTexCoord;
   // ���������任������ռ䣨��������������model����ִ���˲��ȱ����ţ������������ٺϺ������Ƭ������ת��model���������õ�����������������ģ�ͽ��б任�������ĸ��棨ʹ�Դ�ֱ����Ƭ��
   Norm = normModelMatrix * aNormVec;
   FragPos = vec3(model * vec4(aPos, 1.0)); // ע������ʹ��vec3��w������������ΪƬ�β���Ҫ��������б任�����Ѿ����͸�ӳ�����
}