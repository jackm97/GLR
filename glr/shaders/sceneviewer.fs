#version 330 core

/*
*
*
RESERVED
*
*
*/
uniform int useVertColor;

uniform vec3 cameraPos;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 Ke;
uniform float Ns;

uniform sampler2D texture1;
uniform int textureAssigned;

uniform vec3 ambientLightColor;
uniform float ambientI;

uniform vec3 dirLightDir[50];
uniform vec3 dirLightColor[50];
uniform float dirLightI[50];
uniform float dirLightSpec[50];
uniform int numDirLight;

uniform vec3 pointLightPos[50];
uniform vec3 pointLightColor[50];
uniform float pointLightI[50];
uniform float pointLightSpec[50];
uniform int numPointLight;

uniform vec3 spotLightPos[50];
uniform vec3 spotLightDir[50];
uniform vec3 spotLightColor[50];
uniform float spotLightI[50];
uniform float spotLightSpec[50];
uniform float spotLightCutoff[50];
uniform int numSpotLight;
/*
*
*
END RESERVED
*
*
*/

in vec3 FragPos;
in vec3 Norm;
in vec3 VertColor;
in vec2 TexCoord;

out vec4 FragColor;


void main()
{
    // texture mapping
    vec3 texColor = vec3(texture(texture1, TexCoord));
    vec3 diffuseBase;
    (textureAssigned == 0) ? (diffuseBase = Kd) : (diffuseBase = vec3(texColor));

    // vertex coloring?
    (useVertColor == 1) ? (diffuseBase = VertColor) : (diffuseBase = diffuseBase);
    vec3 ambientBase;
    (useVertColor == 0) ? (ambientBase = Ka * diffuseBase) : (ambientBase = diffuseBase);
    vec3 specBase;
    (useVertColor == 0) ? (specBase = Ks) : (specBase = diffuseBase);


    vec3 diffuseColor = vec3(0.0,0.0,0.0);
    vec3 specColor = vec3(0.0,0.0,0.0);
    vec3 ambientColor = vec3(0.0,0.0,0.0);

    // directional lighting
    for (int i=0; i<numDirLight; i++)
    {
        vec3 lightDir = dirLightDir[i];
        vec3 lightColor = dirLightColor[i];
        float I = dirLightI[i];
        // diffuse
        float diff = max(dot(normalize(Norm), normalize(-lightDir)), 0.0);
        diffuseColor += I * diff * lightColor * diffuseBase;

        // specular
        vec3 viewDir    = normalize(FragPos - cameraPos);
        vec3 halfwayDir = normalize(-lightDir - viewDir);
        float dotProd = max(dot(normalize(Norm), normalize(halfwayDir)), 0.0);
        (dotProd < 1e-10) ? (dotProd = 1e-10) : (dotProd = dotProd);
        float spec = dirLightSpec[i] * pow(dotProd, Ns);

        specColor += I * spec * lightColor * specBase;
    }

    // point lighting
    for (int i=0; i<numPointLight; i++)
    {
        vec3 lightDir = FragPos - pointLightPos[i];
        vec3 lightColor = pointLightColor[i];
        float I = pointLightI[i]/( pow( length(lightDir), 2.0 ) );
        // diffuse
        float diff = max(dot(normalize(Norm), normalize(-lightDir)), 0.0);
        diffuseColor += I * diff * lightColor * diffuseBase;

        // specular
        vec3 viewDir    = normalize(FragPos - cameraPos);
        vec3 halfwayDir = normalize(-lightDir - viewDir);
        float dotProd = max(dot(normalize(Norm), normalize(halfwayDir)), 0.0);
        (dotProd < 1e-10) ? (dotProd = 1e-10) : (dotProd = dotProd);
        float spec = pointLightSpec[i] * pow(dotProd, Ns);

        specColor += I * spec * lightColor * specBase;
    }

    // spot lighting
    for (int i=0; i<numSpotLight; i++)
    {
        vec3 lightDir = FragPos - spotLightPos[i];
        vec3 lightColor = spotLightColor[i];
        float I = spotLightI[i]/( pow( length(lightDir), 2.0 ) );
        
        float cutoff = cos(spotLightCutoff[i]);
        bool isLit = (cutoff < dot(normalize(lightDir), spotLightDir[i]));
        float softLightScale = cos( radians(90.)/spotLightCutoff[i] * acos(max(dot(normalize(lightDir), spotLightDir[i]), 0.0)) );
        
        // diffuse
        float diff = max(dot(normalize(Norm), normalize(-lightDir)), 0.0);
        diff *= softLightScale;
        (isLit) ? (diffuseColor += I * diff * lightColor * diffuseBase) : (diffuseColor += 0);

        // specular
        vec3 viewDir    = normalize(FragPos - cameraPos);
        vec3 halfwayDir = normalize(-lightDir - viewDir);
        float dotProd = max(dot(normalize(Norm), normalize(halfwayDir)), 0.0);
        (dotProd < 1e-10) ? (dotProd = 1e-10) : (dotProd = dotProd);
        float spec = spotLightSpec[i] * pow(dotProd, Ns);
        spec *= softLightScale;

        (isLit) ? (specColor += I * spec * lightColor * specBase) : (specColor += 0);
    }

    // ambient lighting
    ambientColor = ambientI * ambientLightColor * ambientBase;
    
    FragColor = vec4( ambientColor + specColor + diffuseColor + Ke, 1.0);
} 