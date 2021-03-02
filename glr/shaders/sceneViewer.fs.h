"#version 330 core\n \
 \n \
/* \n \
* \n \
* \n \
RESERVED \n \
* \n \
* \n \
*/ \n \
uniform vec3 cameraPos; \n \
 \n \
uniform vec3 Ka; \n \
uniform vec3 Kd; \n \
uniform vec3 Ks; \n \
uniform vec3 Ke; \n \
uniform float Ns; \n \
 \n \
uniform sampler2D texture1; \n \
uniform int textureAssigned; \n \
 \n \
uniform vec3 ambientColor; \n \
uniform float ambientI; \n \
 \n \
uniform vec3 dirLightDir[50]; \n \
uniform vec3 dirLightColor[50]; \n \
uniform float dirLightI[50]; \n \
uniform float dirLightSpec[50]; \n \
uniform int numDirLight; \n \
\n \
uniform vec3 pointLightPos[50];\n \
uniform vec3 pointLightColor[50];\n \
uniform float pointLightI[50];\n \
uniform float pointLightSpec[50];\n \
uniform int numPointLight;\n \
\n \
uniform vec3 spotLightPos[50];\n \
uniform vec3 spotLightDir[50];\n \
uniform vec3 spotLightColor[50];\n \
uniform float spotLightI[50];\n \
uniform float spotLightSpec[50];\n \
uniform float spotLightCutoff[50];\n \
uniform int numSpotLight;\n \
/*\n \
*\n \
*\n \
END RESERVED\n \
*\n \
*\n \
*/\n \
\n \
in vec3 FragPos;\n \
in vec3 Norm;\n \
in vec2 TexCoord;\n \
\n \
out vec4 FragColor;\n \
\n \
\n \
void main()\n \
{\n \
    // texture mapping\n \
    vec3 texColor = vec3(texture(texture1, TexCoord));\n \
    vec3 diffuseBase;\n \
    (textureAssigned == 0) ? (diffuseBase = Kd) : (diffuseBase = vec3(texColor));\n \
\n \
\n \
    vec3 diffuseColor = vec3(0.0,0.0,0.0);\n \
    vec3 specColor = vec3(0.0,0.0,0.0);\n \
\n \
    // directional lighting\n \
    for (int i=0; i<numDirLight; i++)\n \
    {\n \
        vec3 lightDir = dirLightDir[i];\n \
        vec3 lightColor = dirLightColor[i];\n \
        float I = dirLightI[i];\n \
        // diffuse\n \
        float diff = max(dot(normalize(Norm), normalize(-lightDir)), 0.0);\n \
        diffuseColor += I * diff * lightColor * diffuseBase;\n \
\n \
        // specular\n \
        vec3 viewDir    = normalize(FragPos - cameraPos);\n \
        vec3 halfwayDir = normalize(-lightDir - viewDir);\n \
        float dotProd = max(dot(normalize(Norm), normalize(halfwayDir)), 0.0);\n \
        (dotProd < 1e-10) ? (dotProd = 1e-10) : (dotProd = dotProd);\n \
        float spec = dirLightSpec[i] * pow(dotProd, Ns);\n \
\n \
        specColor += I * spec * lightColor * Ks;\n \
    }\n \
\n \
    // point lighting\n \
    for (int i=0; i<numPointLight; i++)\n \
    {\n \
        vec3 lightDir = FragPos - pointLightPos[i];\n \
        vec3 lightColor = pointLightColor[i];\n \
        float I = pointLightI[i]/( pow( length(lightDir), 2.0 ) );\n \
        // diffuse\n \
        float diff = max(dot(normalize(Norm), normalize(-lightDir)), 0.0);\n \
        diffuseColor += I * diff * lightColor * diffuseBase;\n \
\n \
        // specular\n \
        vec3 viewDir    = normalize(FragPos - cameraPos);\n \
        vec3 halfwayDir = normalize(-lightDir - viewDir);\n \
        float dotProd = max(dot(normalize(Norm), normalize(halfwayDir)), 0.0);\n \
        (dotProd < 1e-10) ? (dotProd = 1e-10) : (dotProd = dotProd);\n \
        float spec = pointLightSpec[i] * pow(dotProd, Ns);\n \
\n \
        specColor += I * spec * lightColor * Ks;\n \
    }\n \
\n \
    // spot lighting\n \
    for (int i=0; i<numSpotLight; i++)\n \
    {\n \
        vec3 lightDir = FragPos - spotLightPos[i];\n \
        vec3 lightColor = spotLightColor[i];\n \
        float I = spotLightI[i]/( pow( length(lightDir), 2.0 ) );\n \
        \n \
        float cutoff = cos(spotLightCutoff[i]);\n \
        bool isLit = (cutoff < dot(normalize(lightDir), spotLightDir[i]));\n \
        float softLightScale = cos( radians(90.)/spotLightCutoff[i] * acos(max(dot(normalize(lightDir), spotLightDir[i]), 0.0)) );\n \
        \n \
        // diffuse\n \
        float diff = max(dot(normalize(Norm), normalize(-lightDir)), 0.0);\n \
        diff *= softLightScale;\n \
        (isLit) ? (diffuseColor += I * diff * lightColor * diffuseBase) : (diffuseColor += 0);\n \
\n \
        // specular\n \
        vec3 viewDir    = normalize(FragPos - cameraPos);\n \
        vec3 halfwayDir = normalize(-lightDir - viewDir);\n \
        float dotProd = max(dot(normalize(Norm), normalize(halfwayDir)), 0.0);\n \
        (dotProd < 1e-10) ? (dotProd = 1e-10) : (dotProd = dotProd);\n \
        float spec = spotLightSpec[i] * pow(dotProd, Ns);\n \
        spec *= softLightScale;\n \
\n \
        (isLit) ? (specColor += I * spec * lightColor * Ks) : (specColor += 0);\n \
    }\n \
    \n \
    FragColor = vec4( ambientI * ambientColor * Ka * diffuseBase + specColor + diffuseColor + Ke, 1.0);\n \
} "
