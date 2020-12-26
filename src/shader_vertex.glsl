#version 330 core

// Atributos de v�rtice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a fun��o BuildTrianglesAndAddToVirtualScene() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texture_coefficients;

// Matrizes computadas no c�digo C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#define PLANE      0
#define PLAYER_COW 1
#define COW2       2
#define COW3       3
#define FENCE      4
#define PIG        5
#define PORTAO     6
uniform int object_id;
uniform sampler2D CowTextureBlackWhite;
uniform sampler2D CowTextureBrownWhite;

uniform vec4 bbox_min;
uniform vec4 bbox_max;


// Atributos de v�rtice que ser�o gerados como sa�da ("out") pelo Vertex Shader.
// ** Estes ser�o interpolados pelo rasterizador! ** gerando, assim, valores
// para cada fragmento, os quais ser�o recebidos como entrada pelo Fragment
// Shader. Veja o arquivo "shader_fragment.glsl".
out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec2 texcoords;
out vec4 vertex_color;

vec2 planeProjectionXY();

void main()
{
    // A vari�vel gl_Position define a posi��o final de cada v�rtice
    // OBRIGATORIAMENTE em "normalized device coordinates" (NDC), onde cada
    // coeficiente estar� entre -1 e 1 ap�s divis�o por w.
    // Veja slides 144 e 150 do documento "Aula_09_Projecoes.pdf".
    //
    // O c�digo em "main.cpp" define os v�rtices dos modelos em coordenadas
    // locais de cada modelo (array model_coefficients). Abaixo, utilizamos
    // opera��es de modelagem, defini��o da c�mera, e proje��o, para computar
    // as coordenadas finais em NDC (vari�vel gl_Position). Ap�s a execu��o
    // deste Vertex Shader, a placa de v�deo (GPU) far� a divis�o por W. Veja
    // slide 189 do documento "Aula_09_Projecoes.pdf".

    gl_Position = projection * view * model * model_coefficients;

    // Como as vari�veis acima  (tipo vec4) s�o vetores com 4 coeficientes,
    // tamb�m � poss�vel acessar e modificar cada coeficiente de maneira
    // independente. Esses s�o indexados pelos nomes x, y, z, e w (nessa
    // ordem, isto �, 'x' � o primeiro coeficiente, 'y' � o segundo, ...):
    //
    //     gl_Position.x = model_coefficients.x;
    //     gl_Position.y = model_coefficients.y;
    //     gl_Position.z = model_coefficients.z;
    //     gl_Position.w = model_coefficients.w;
    //

    // Agora definimos outros atributos dos v�rtices que ser�o interpolados pelo
    // rasterizador para gerar atributos �nicos para cada fragmento gerado.

    // Posi��o do v�rtice atual no sistema de coordenadas global (World).
    position_world = model * model_coefficients;

    // Posi��o do v�rtice atual no sistema de coordenadas local do modelo.
    position_model = model_coefficients;

    // Normal do v�rtice atual no sistema de coordenadas global (World).
    // Veja slide 107 do documento "Aula_07_Transformacoes_Geometricas_3D.pdf".
    normal = inverse(transpose(model)) * normal_coefficients;
    normal.w = 0.0;


    // Obtemos a posi��o da c�mera utilizando a inversa da matriz que define o
    // sistema de coordenadas da c�mera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
    texcoords = texture_coefficients;

    // O fragmento atual � coberto por um ponto que percente � superf�cie de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posi��o no
    // sistema de coordenadas global (World coordinates). Esta posi��o � obtida
    // atrav�s da interpola��o, feita pelo rasterizador, da posi��o de cada
    // v�rtice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada v�rtice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em rela��o ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da c�mera em rela��o ao ponto atual.
    vec4 v = normalize(camera_position - p);

    float U = 0.0;
    float V = 0.0;

    vec4 Kd = vec4(0.0,0.0,0.0,1.0);

    vec4 Ka = vec4(0.1,0.1,0.1,1.0);
    vec4 Ia = vec4(1.0,1.0,1.0,1.0);

    vec4 Ks = vec4(0.0,0.0,0.0,1.0);
    vec4 I = vec4(0.0,0.0,0.0,1.0);
    float q = 0;
    if ( object_id == COW2 )
    {
        vec2 aux = planeProjectionXY();
        U = aux.x;
        V = aux.y;
        //Kd = texture(CowTextureBlackWhite, vec2(U,V));
        Kd = texture(CowTextureBrownWhite, vec2(U,V));
        Ks = vec4(0.2,0.2,0.2,1.0);
        I = vec4(1.0,1.0,1.0,1.0);
        q = 100;
    }
    vec4 h = normalize(v+l);
    vec4 lambert = Kd*I*(max(0,dot(n,l)) + 0.05);
    vec4 ambient = Ka*Ia;
    vec4 blinn_phong_specular = Ks*I*pow(max(0,dot(n,h)),q);

    vertex_color = lambert + ambient + blinn_phong_specular;
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
