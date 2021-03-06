#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

in vec4 vertex_color;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define PLANE      0
#define PLAYER_COW 1
#define COW2       2
#define COW3       3
#define FENCE      4
#define PIG        5
#define PORTAO     6
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D GrassTexture;
uniform sampler2D CowTextureBrownWhite;
uniform sampler2D CowTextureBlackWhite;
uniform sampler2D CowTextureBrownWhite2;
uniform sampler2D Fence;
uniform sampler2D PigTexture;
uniform sampler2D Portao;


// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

vec2 planeProjectionXY();

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    bool only_lambert = false;

    vec4 Kd = vec4(0.0,0.0,0.0,1.0);

    vec4 Ka = vec4(0.1,0.1,0.1,1.0);
    vec4 Ia = vec4(1.0,1.0,1.0,1.0);

    vec4 Ks = vec4(0.0,0.0,0.0,1.0);
    vec4 I = vec4(0.0,0.0,0.0,1.0);
    float q = 0;
    if ( object_id == PLAYER_COW)
    {
        vec2 aux = planeProjectionXY();
        U = aux.x;
        V = aux.y;
        //Kd = texture(CowTextureBrownWhite, vec2(U,V));
        Kd = texture(CowTextureBlackWhite, vec2(U,V));
        Ks = vec4(0.2,0.25,0.2,1.0);
        I = vec4(1.0,1.0,1.0,1.0);
        q = 30;
    }
    else if ( object_id == COW3)
    {
        vec2 aux = planeProjectionXY();
        U = aux.x;
        V = aux.y;
        Kd = texture(CowTextureBrownWhite2, vec2(U,V));
        Ks = vec4(0.2,0.25,0.2,1.0);
        I = vec4(1.0,1.0,1.0,1.0);
        q = 30;
    }
    else if (object_id == PIG)
    {
        vec2 aux = planeProjectionXY();
        U = aux.x;
        V = aux.y;
        Kd = texture(PigTexture, vec2(U,V));
        Ks = vec4(0.5,0.5,0.5,1.0);
        I = vec4(1.0,1.0,1.0,1.0);
        q = 10;
    }

    vec4 h = normalize(v+l);
    vec4 lambert = Kd*I*(max(0,dot(n,l)) + 0.05);
    vec4 ambient = Ka*Ia;
    vec4 blinn_phong_specular = Ks*I*pow(max(0,dot(n,h)),q);

    if(only_lambert)
        color = lambert;
    else
        color = lambert + ambient + blinn_phong_specular;

    if ( object_id == COW2)
    {
        color = vertex_color;
    }
    else if ( object_id == PLANE ){
        U = texcoords.x;
        V = texcoords.y;
        color = texture(GrassTexture, vec2(U,V));
    }
    else if( object_id == FENCE){
        U = texcoords.x;
        V = texcoords.y;
        color = texture(Fence, vec2(U,V));
    }
    else if(object_id == PORTAO){
        U = texcoords.x;
        V = texcoords.y;
        color = texture(Portao, vec2(U,V));
    }

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    //color = pow(color, vec4(1.0,1.0,1.0,1.0)/2.2);
}

vec2 planeProjectionXY()
{
    float minx = bbox_min.x;
    float maxx = bbox_max.x;

    float miny = bbox_min.y;
    float maxy = bbox_max.y;

    float minz = bbox_min.z;
    float maxz = bbox_max.z;

    float px = position_model.x;
    float py = position_model.y;

    float U = (px - minx) / (maxx - minx);
    float V = (py - miny) / (maxy - miny);

    return vec2(U,V);
}
