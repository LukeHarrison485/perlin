#version 460 core
out vec4 FragColor; // Output color of the fragment

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform int numPointLights;
uniform DirLight dirLight;
uniform Material material;
uniform PointLight pointLights[100];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

vec4 texture2DAA(sampler2D tex, vec2 uv) {
    vec2 lod = textureQueryLod(tex, uv);
    vec2 texSize = vec2(textureSize(tex, int(lod.x)));
    vec2 uvTexSpace = uv * texSize;
    vec2 seam = floor(uvTexSpace + 0.5);
    uvTexSpace = (uvTexSpace - seam) / fwidth(uvTexSpace) + seam;
    uvTexSpace = clamp(uvTexSpace, seam - 0.5, seam + 0.5);
    return texture(tex, uvTexSpace / texSize);
}

void main()
{
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    if(numPointLights != 0) {
        for(int i = 0; i < numPointLights; i++) {
            result += CalcPointLight(pointLights[i], norm, FragPos, viewPos);
        }
    }
    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // Use texture2DAA for anti-aliased texture sampling
    vec3 ambient = light.ambient * vec3(texture2DAA(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture2DAA(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture2DAA(material.specular, TexCoords));
    
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    vec3 ambient = light.ambient * vec3(texture2DAA(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture2DAA(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture2DAA(material.specular, TexCoords));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}