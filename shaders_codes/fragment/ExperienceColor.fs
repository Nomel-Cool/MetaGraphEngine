#version 330 core
//in vec3 colorPos;// Same name & type in vertexShaderSource, it is corresponding. \n"
in vec2 TexCoord; // ����ȡ����
in vec3 vertexColor;
// ������ ĳ�ֲ��ʵ� ������ǿ�ȡ�
struct Material {
    //vec3 ambient;    // ��Ϊ��������ɫ�ڼ�����������¶�������������ɫ������ȥ��
    //vec3 diffuse;    // ���������
    sampler2D diffuse; // ʹ�ò��ʵĶ���������ɫ�������������
    sampler2D specular;   // �������
    float shininess; // �����
};
struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Norm;
in vec3 FragPos;

out vec4 FragColor;

//uniform vec3 objectColor;
//uniform vec3 lightColor;
//uniform vec3 lightPos;
uniform vec3 viewPos; // ������꣨������ռ䣩
uniform Material material;
uniform Light light;

//uniform vec4 globalFragColor;
//uniform sampler2D globalTexture1; // sampler2D ΪƬ����ɫ���Դ��������ͣ�������һ��������ʱ��������Ĭ�ϰ� GL_TEXTURE0 �Լ��� ��Ӧ���ʶ�����Ҫ CPU ���ֶ� Set
//uniform sampler2D globalTexture2; // sampler2D ΪƬ����ɫ���Դ���������
//uniform float texture_ratio;
void main()
{
   //FragColor = vec4(colorPos,1.0);
   //FragColor = texture(globalTexture, TexCoord) * vec4(colorPos, 1.0); // ʹ�� texture ����ʾƬ��ʹ�ò��ʲ��������Ƿ����˷�
   //FragColor = mix(texture(globalTexture1, TexCoord), texture(globalTexture2, vec2(1.0 - TexCoord.x, TexCoord.y)), texture_ratio); // 0.2 �᷵�� 80% �ĵ�һ��������ɫ�� 20% �ĵڶ���������ɫ����������������Ļ��ɫ
   //FragColor = vec4(lightColor * objectColor, 1.0);
   
   // ���������ϵĹ���ͼ��ѧ��ֱ�������屾��ġ���ɫ�����壨��Ȼ��ɫ�ǹⷴ��󿴵��ģ��Ǻ���ģ������Ǽ����ͼ��ѧYou Know����
   // ��ȫ�ֹ��գ��� �����ڱ�����Ĺ⡱ ����һ������ϵ����ֻҪ���Ǻ�ɫ�Ķ���΢��
   //float ambientStrength = 0.3;
   //vec3 ambient = material.ambient * light.ambient;
    //vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord)); // ʹ�ò�����ɫ���������ܵĻ�������ɫ
    vec3 ambient = light.ambient * vertexColor; // ʹ�ò�����ɫ���������ܵĻ�������ɫ

    // ����������thetaֵ
    vec3 norm = normalize(Norm);
    vec3 lightDir = normalize(light.position - FragPos); // ����ָ���Դ�ķ�������
    float diff = max(dot(norm, lightDir), 0.0); // ����нǳ��� 90�㣬����thetaΪ��������û�ж��壬����ȡ0��ɫ��
    //vec3 diffuse = light.diffuse * (diff * material.diffuse) * lightColor; // ��Դ���µ��������ٳ��ϲ��ʵ�������ǿ��
    vec3 diffuse = light.diffuse * diff * vertexColor; // ȥ���̶�����⣬��ɫ��ȡΪ������ɫ

    // ����ǿ�ȣ��߹����ȣ�
    //float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos); // �õ������嵽����ķ�������
    vec3 reflectDir = reflect(-lightDir, norm); // ���߷���ȡ����reflect����Ҫ���һ�������Ǵӹ�Դָ��Ƭ��λ�õ�����������lightDir��ǰ�����෴
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // ���32�Ǹ߹�ķ����(Shininess)��һ������ķ����Խ�ߣ�����������Խǿ��ɢ���Խ�٣��߹��ͻ�ԽС
    vec3 specular = light.specular * spec * vertexColor;

    // ���ս���
    //vec3 result = (ambient + diffuse + specular) * objectColor;
    vec3 result = ambient + diffuse + specular; // ��ʹ��������ɫ
    FragColor = vec4(result, 1.0);
    // FragColor = vec4(Norm, 1.0); // ���ԣ�����������Ϊ��ɫ�����������ɫ��ͬ��˵��Norm������ȷ
}

// �򻯲��Բ�����
//void main()
//{
//    FragColor = texture(material.diffuse, TexCoord); // ֱ����ʾ������ɫ
//}