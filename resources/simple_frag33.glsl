#version 330 core
in vec3 fragNor;
out vec4 color;
in vec3 pos;
void main()
{
    vec3 normal = normalize(fragNor);
    // Map normal in the range [-1, 1] to color in range [0, 1];
    vec3 Ncolor = 0.5*normal + 0.5;
    color = vec4(Ncolor, 1.0);
    
    vec3 lp = vec3(100,100,100);
    vec3 ld = normalize(lp - pos);
    float light = dot(ld,normal);
    color = vec4(0.658824,0.658824,0.658824,1) * light;
    color.a=1;
    
//    color.a=0.3;

}
