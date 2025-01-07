#version 330 core
//in vec3 colorPos;// Same name & type in vertexShaderSource, it is corresponding. \n"
in vec2 TexCoord; // 纹理取样点
in vec3 vertexColor;
// 定义了 某种材质的 【光照强度】
struct Material {
    //vec3 ambient;    // 因为环境光颜色在几乎所有情况下都等于漫反射颜色，所以去除
    //vec3 diffuse;    // 漫反射光照
    sampler2D diffuse; // 使用材质的多种像素颜色计算漫反射光照
    sampler2D specular;   // 镜面光照
    float shininess; // 反光度
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
uniform vec3 viewPos; // 相机坐标（在世界空间）
uniform Material material;
uniform Light light;

//uniform vec4 globalFragColor;
//uniform sampler2D globalTexture1; // sampler2D 为片段着色器自带采样类型，当多于一个采样器时，并不会默认绑定 GL_TEXTURE0 以及其 对应材质对象，需要 CPU 处手动 Set
//uniform sampler2D globalTexture2; // sampler2D 为片段着色器自带采样类型
//uniform float texture_ratio;
void main()
{
   //FragColor = vec4(colorPos,1.0);
   //FragColor = texture(globalTexture, TexCoord) * vec4(colorPos, 1.0); // 使用 texture 来表示片段使用材质采样，这是分量乘法
   //FragColor = mix(texture(globalTexture1, TexCoord), texture(globalTexture2, vec2(1.0 - TexCoord.x, TexCoord.y)), texture_ratio); // 0.2 会返回 80% 的第一个输入颜色和 20% 的第二个输入颜色，即返回两个纹理的混合色
   //FragColor = vec4(lightColor * objectColor, 1.0);
   
   // 打在物体上的光在图形学内直接由物体本身的“颜色”定义（虽然颜色是光反射后看到的，是后验的，但这是计算机图形学You Know？）
   // 简化全局光照，对 “打在本物体的光” 乘上一个固有系数，只要不是黑色的都有微光
   //float ambientStrength = 0.3;
   //vec3 ambient = material.ambient * light.ambient;
    //vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord)); // 使用材质颜色替代物体接受的环境光颜色
    vec3 ambient = light.ambient * vertexColor; // 使用材质颜色替代物体接受的环境光颜色

    // 计算漫反射theta值
    vec3 norm = normalize(Norm);
    vec3 lightDir = normalize(light.position - FragPos); // 物体指向光源的方向向量
    float diff = max(dot(norm, lightDir), 0.0); // 避免夹角超过 90°，导致theta为负数，它没有定义，所以取0黑色。
    //vec3 diffuse = light.diffuse * (diff * material.diffuse) * lightColor; // 光源导致的漫反射再乘上材质的漫反射强度
    vec3 diffuse = light.diffuse * diff * vertexColor; // 去除固定漫射光，颜色光取为材质颜色

    // 镜面强度（高光亮度）
    //float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos); // 得到从物体到相机的方向向量
    vec3 reflectDir = reflect(-lightDir, norm); // 光线方向取反，reflect函数要求第一个向量是从光源指向片段位置的向量，但是lightDir当前正好相反
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 这个32是高光的反光度(Shininess)。一个物体的反光度越高，反射光的能力越强，散射得越少，高光点就会越小
    vec3 specular = light.specular * spec * vertexColor;

    // 最终结算
    //vec3 result = (ambient + diffuse + specular) * objectColor;
    vec3 result = ambient + diffuse + specular; // 不使用物体颜色
    FragColor = vec4(result, 1.0);
    // FragColor = vec4(Norm, 1.0); // 测试：将法向量作为颜色输出，各面颜色不同，说明Norm接受正确
}

// 简化测试采样器
//void main()
//{
//    FragColor = texture(material.diffuse, TexCoord); // 直接显示纹理颜色
//}