#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include <iostream>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>
#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include <irrKlang/irrKlang.h>
//#pragma comment(lib, "irrKlang.lib")

#include "utils.h"
#include "matrices.h"
#include "helpers.h" // Funções de aula

#define M_PI   3.14159265358979323846

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);

void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Funções implementadas
void HandlePressedKeys();
bool CheckBoxBoxCollision(const char* object1,const char* object2, glm::vec3 offset1, glm::vec3 offset2);
bool CheckBoxSphereCollision(const char* object1, const char* object2, glm::vec3 offset1, glm::vec3 offset2, float radius_offset);
bool CheckPointPlaneCollision(glm::vec4 p);
bool CheckBoxBoxCollision(const char* object, glm::vec3 offset);
void TextRendering_ShowObjectives(GLFWwindow* window);
void TextRendering_EToInterack(GLFWwindow* window);
void TextRendering_PigTalk(GLFWwindow* window);
void TextRendering_cowTalk(GLFWwindow* window);
void TextRendering_GameEnd(GLFWwindow* window);
bool render_cowTalk = false;
bool render_pigTalk = false;
bool nearPig();
int nearCow();
bool nearGate();
void rotateGate(bool open);
float parameterizeTime(float t);
void setCowPositions();


glm::vec4 getBezierPoint(float t);

// Estrutura para ficar mais facil a manipulação dos objetos do jogo
struct ObjectProperties
{
    std::string  name_model;
    glm::vec4 pos;
    float rotation;
    glm::vec4 forward;
    float forward_velocity = 4.0f;
    float turning_velocity = 10.0f;
};
struct ObjectProperties SetObjectProperties(const char* name_model, glm::vec4 initial_position, float initial_rotation, glm::vec4 initial_forward);

// Variáveis de controle global do jogo
float g_ScreenRatio = 1.0f;
bool g_LeftMouseButtonPressed = false;
float g_CameraTheta = 3*M_PI/2; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = M_PI/6;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.0f; // Distância da câmera para o objeto

bool g_UsePerspectiveProjection = true;
bool g_ShowInfoText = true;
bool isMissionPigTalk = true;
bool isMissionCowTalk = false;
bool isMissionEscape = false;
bool escaped = false;

float t_now;
float t_old;
float delta_time;
const int total_vaquinhas = 5;
std::vector<bool> aderidas(total_vaquinhas);
int calculateAderidas();

ObjectProperties player;
ObjectProperties porco;
ObjectProperties gate;
ObjectProperties vaquinhas[total_vaquinhas];

#define KEYS 349
bool pressed[KEYS];
float delta = M_PI / 16; // 11,25 graus, em radianos.
float t_pig_movement = 0;
bool pigCanMove = true;

float boundary = 20.0f;
float rot_portao = 0.0f;

void RotateBBox();
bool game_start;
bool game_end;
bool endGameMusicPlaying = false;

irrklang::ISoundEngine* soundEngine;
bool mooPlayed = false;
bool oincPlayed = false;

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "Fuga das Vaquinhas", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    LoadShadersFromFiles();

    // Carregamos imagens para serem utilizadas como textura
    LoadTextureImage("../../data/grassTexture.png");
    LoadTextureImage("../../data/CowTextureBrownWhite.png");
    LoadTextureImage("../../data/CowTextureBlackWhite.png");
    LoadTextureImage("../../data/CowTextureBrownWhite2.png");
    LoadTextureImage("../../data/fence.png");
    LoadTextureImage("../../data/PigTexture.jpg");
    LoadTextureImage("../../data/portao.png");


    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel cowmodel("../../data/cow.obj");
    ComputeNormals(&cowmodel);
    BuildTrianglesAndAddToVirtualScene(&cowmodel);

    ObjModel pigmodel("../../data/pig.obj");
    ComputeNormals(&pigmodel);
    BuildTrianglesAndAddToVirtualScene(&pigmodel);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Seta propriedades do portão
    gate = SetObjectProperties("plane",
                                glm::vec4(0.0f,-0.5f,boundary-1.0f,1.0f),
                                M_PI/2,
                                glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    gate.turning_velocity = 5.0f;

    // Seta propriedades do jogador
    player = SetObjectProperties("cow",
                                 glm::vec4(gate.pos.x, -0.38f, gate.pos.z-2.5f*getZSize("cow"), 1.0f),
                                 M_PI/2,
                                 glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Seta propriedades do porquinho
    porco = SetObjectProperties("pig",
                                glm::vec4(boundary-6.0f, -1.0f, -boundary-1.0f, 1.0f),
                                -M_PI/2,
                                glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

    // Altera valores inicias para a posição da camera
    g_CameraTheta = 0.0f;
    g_CameraPhi = M_PI/8;

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);

    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    // Gera posições iniciais para as vaquinhas(NPCs) do jogo
    setCowPositions();

    // Variáveis de controle para fim do jogo
    game_start = true;
    game_end = false;

    // Inicializa engine de som para o mugido das vaquinhas
    soundEngine = irrklang::createIrrKlangDevice();
    if(!soundEngine)
        printf("Failed to load Soung Engine");

    // Setamos o volume do som para não ser tão alto
    soundEngine->setSoundVolume(0.35f);

    t_old = (float)glfwGetTime();
    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Constantes para o shader_fragment linkar com os modelos
        #define PLANE      0
        #define PLAYER_COW 1
        #define COW2       2
        #define COW3       3
        #define FENCE      4
        #define PIG        5
        #define PORTAO     6

        // Habilitamos o backface Culling
        glEnable(GL_CULL_FACE);

        // Definimos a cor do "fundo" do framebuffer como cor do ceú(azul).
        //           R     G     B     A
        glClearColor(0.25f, 0.72f, 0.94f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program_id);

        // Calculamos as variaveis de controle da camera como em laboratório
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        glm::vec4 camera_position_c  = player.pos + glm::vec4(x,y,z,0.0f); // Ponto "c", centro da câmera
        glm::vec4 camera_view_vector = -glm::vec4(x,y,z,0.0f);//player.pos - camera_position_c; // Vetor "view", sentido para onde a câmera está virada
        glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)

        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        glm::mat4 projection;
        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -50.0f; // Posição do "far plane"
        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);

        // Cena de Fim de Jogo
        // É carregada quando o portão se abre, utiliza Projeção Ortográfica
        // e altera onde os objetos são desenhados.
        //game_end = true;
        if(game_end){
            float t = 1.5f*g_CameraDistance/2.5f;
            float b = -t;
            float r = t*g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);

            g_CameraDistance = 4.0f;
            g_CameraPhi = M_PI/7;
            g_CameraTheta = M_PI/2;

            r = g_CameraDistance;
            y = r*sin(g_CameraPhi);
            z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
            x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

            player.pos = glm::vec4(0.0f,player.pos.y,0.0f,1.0f);
            camera_position_c  = player.pos + glm::vec4(x,y,z,0.0f); // Ponto "c", centro da câmera
            camera_view_vector = -glm::vec4(x,y,z,0.0f);

            view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

            glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

            glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
            glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

            // Desenhamos o chão
            model = Matrix_Translate(0.0f, -1.0f,0.0f)
                * Matrix_Scale(50.0f, 50.0f, 50.f);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, PLANE);
            DrawVirtualObject("plane");

            // Desenhamos o player
            model = Matrix_Translate(player.pos.x, player.pos.y, player.pos.z);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, PLAYER_COW);
            DrawVirtualObject("cow");

            // Desenhamos as vaquinhas
            model = Matrix_Translate(-3.0f, player.pos.y, -2.5f);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, COW3);
            DrawVirtualObject("cow");
            model = Matrix_Translate(-1.5f, player.pos.y, -1.25f);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, COW2);
            DrawVirtualObject("cow");
            model = Matrix_Translate(-1.5f, player.pos.y, 1.25f);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, COW3);
            DrawVirtualObject("cow");
            model = Matrix_Translate(-3.0f, player.pos.y, 2.5f);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, COW2);
            DrawVirtualObject("cow");

            TextRendering_GameEnd(window);
            if(!endGameMusicPlaying){
                soundEngine->play2D("..//..//data/endGameMusic.mp3",true);
                endGameMusicPlaying = true;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }
        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        // Calcula o vetor de diração do player
        player.forward.x = cos(player.rotation);
        player.forward.z = -sin(player.rotation);

        // Cacula delta_time
        t_now = (float)glfwGetTime();
        delta_time = t_now - t_old;
        t_old = t_now;
        // Chamada de função de movimentação do player
        HandlePressedKeys();

        // Lógica para o porquinho não bugar na hora do diálogo com ele
        if(!nearPig() && pigCanMove)
        {
            t_pig_movement = t_now;
            porco.pos = getBezierPoint(t_pig_movement);
            pigCanMove = true;
        }
        else
            pigCanMove = false;

        if((parameterizeTime(t_pig_movement) >= parameterizeTime(t_now) - delta_time)
           && (parameterizeTime(t_pig_movement) <= parameterizeTime(t_now) + delta_time))
            pigCanMove = true;


        // Desenhamos o plano do chão
        model = Matrix_Translate(0.0f, -1.0f,0.0f)
                * Matrix_Scale(50.0f, 50.0f, 50.f);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, PLANE);
        DrawVirtualObject("plane");


        // Desenhamos a vaca principal (Player)
        model = Matrix_Translate(player.pos.x, player.pos.y, player.pos.z)
                * Matrix_Rotate_Y(player.rotation);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, PLAYER_COW);
        DrawVirtualObject("cow");

        // Desenhamos o porco
        model = Matrix_Translate(porco.pos.x, porco.pos.y, porco.pos.z)
                * Matrix_Rotate_Y(-M_PI/4)
                * Matrix_Rotate_X(porco.rotation);
        glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(object_id_uniform, PIG);
        DrawVirtualObject("pig");

        // Desenhamos as vaquinhas
        for(int i=0; i<total_vaquinhas; i++){
            glm::vec4 v = player.pos - vaquinhas[i].pos;
            vaquinhas[i].rotation = atan2(v.z,v.x);

            model = Matrix_Translate(vaquinhas[i].pos.x, vaquinhas[i].pos.y, vaquinhas[i].pos.z)
                    * Matrix_Rotate_Y(-vaquinhas[i].rotation);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));

            if(i%2 == 0)
                glUniform1i(object_id_uniform, COW3);
            else
                glUniform1i(object_id_uniform, COW2);

            DrawVirtualObject("cow");
        }


        // Desabilitamos o backface Culling para conseguirmos ver as cercas e o portão do outro lado deles
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        // Desenhamos o portão
        model = Matrix_Translate(gate.pos.x, gate.pos.y, gate.pos.z)
                        * Matrix_Scale(1.0f, 0.5f, 1.0f)
                        * Matrix_Rotate_X(M_PI/2)
                        * Matrix_Translate(-getXSize("plane")/2,0.0f,0.0f)
                        * Matrix_Rotate_Z(gate.rotation)
                        * Matrix_Translate(getXSize("plane")/2,0.0f,0.0f);
                glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(object_id_uniform, PORTAO);
                DrawVirtualObject("plane");

        // Desenhamos as cercas
        for(float i=-boundary;i<boundary;i += getXSize("plane"))
        {
            model = Matrix_Translate(i, -0.5f, -boundary-1.0f)
                    * Matrix_Scale(1.0f, 0.5f, 0.0f)
                    * Matrix_Rotate_X(M_PI/2);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, FENCE);
            DrawVirtualObject("plane");

            if(i != 0.0f)
            {
                model = Matrix_Translate(i, -0.5f, boundary-1.0f)
                        * Matrix_Scale(1.0f, 0.5f, 0.0f)
                        * Matrix_Rotate_X(M_PI/2);
                glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(object_id_uniform, FENCE);
                 DrawVirtualObject("plane");
            }

            model = Matrix_Translate(boundary-1.0f, -0.5f, i)
                    * Matrix_Scale(0.0f, 0.5f, 1.0f)
                    * Matrix_Rotate_X(M_PI/2)
                    * Matrix_Rotate_Z(M_PI/2);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, FENCE);
            DrawVirtualObject("plane");

            model = Matrix_Translate(-boundary-1.0f, -0.5f, i)
                    * Matrix_Scale(0.0f, 0.5f, 1.0f)
                    * Matrix_Rotate_X(M_PI/2)
                    * Matrix_Rotate_Z(M_PI/2);
            glUniformMatrix4fv(model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            glUniform1i(object_id_uniform, FENCE);
            DrawVirtualObject("plane");

        }

        TextRendering_ShowObjectives(window);
        TextRendering_ShowFramesPerSecond(window);

        // Lógica do jogo, quais diálogos mostrar e quais animações/sons executar
        if(game_start) {
            rotateGate(false);
            if(gate.rotation <= 0.0f)
                game_start = false;
        }
        if(nearPig()){
            if(render_pigTalk){
                TextRendering_PigTalk(window);
                if(!oincPlayed){
                    soundEngine->play2D("..//..//data/pig.mp3",false);
                    oincPlayed = true;
                }
            }
            else{
                TextRendering_EToInterack(window);
                oincPlayed = false;
            }
        }
        if(nearCow() > 0 && isMissionCowTalk){
            if(render_cowTalk){
                TextRendering_cowTalk(window);
                if(!mooPlayed){
                    soundEngine->play2D("..//..//data/moo.mp3",false);
                    mooPlayed = true;
                }
                if (calculateAderidas() == total_vaquinhas){
                    isMissionCowTalk = false;
                    isMissionEscape = true;
                }
            }
            else{
                TextRendering_EToInterack(window);
                mooPlayed = false;
            }
        }
        else{
            render_cowTalk = false;

        }

        if(nearGate() && isMissionEscape)
            TextRendering_EToInterack(window);

        if(escaped)
        {
            isMissionEscape = false;
            rotateGate(true);
            if(gate.rotation >= M_PI/2)
                game_end = true;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    soundEngine->drop();
    return 0;
}

// Calculas as posições das vaquinhas aleatoriamente mas cuidando para elas não colidirem e ficarem uma em cima da outra
void setCowPositions()
{
    float LO = -boundary + 5.0f;
    float HI = -LO;
    srand (static_cast <unsigned> (time(0)));
    for(int i=0; i<total_vaquinhas; i++){
        bool collided = false;
        float random1, random2;
        do{
            collided = false;
            random1 = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
            random2 = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));

            for(int j=i-1; j>=0; j--){
               collided = collided || CheckBoxBoxCollision("cow",
                                                           "cow",
                                                           glm::vec3(vaquinhas[j].pos),
                                                           glm::vec3(random1,0.0f,random2)
                                                           );
            }
            collided = collided || CheckBoxBoxCollision("cow",
                                                        "cow",
                                                        glm::vec3(player.pos.x,player.pos.y,player.pos.z),
                                                        glm::vec3(random1,0.0f,random2)
                                                        );
        }while(collided);
        vaquinhas[i].pos.x = random1;
        vaquinhas[i].pos.y = player.pos.y;
        vaquinhas[i].pos.z = random2;
        vaquinhas[i].rotation = 0.0f;
        aderidas[i] = false;
    }
}
// Parametriza o valor de t >= 0 para o intervalo [0,1]
float parameterizeTime(float t)
{
    if((int)floor(t) % 2 == 0)
        t = t - floor(t);
    else
        t = ceil(t) - t;
    return t;
}
// Função que retorna ponto da curva de bezier dado parametro t, este parametro pode ser qualquer número real >= 0
glm::vec4 getBezierPoint(float t)
{
    t = parameterizeTime(t);

    glm::vec4 p1 = glm::vec4(boundary - 6.0f, porco.pos.y, -boundary - 1.0f, 1.0f);
    glm::vec4 p2 = glm::vec4(boundary - 7.0f, porco.pos.y, -boundary       , 1.0f);
    glm::vec4 p3 = glm::vec4(boundary - 3.5f, porco.pos.y, -boundary + 3.0f, 1.0f);
    glm::vec4 p4 = glm::vec4(boundary - 3.0f, porco.pos.y, -boundary + 2.0f, 1.0f);

    return glm::vec4(pow(1-t,3)*p1.x + 3*t*pow(1-t,2)*p2.x + 3*pow(t,2)*(1-t)*p3.x + pow(t,3)*p4.x,
                     pow(1-t,3)*p1.y + 3*t*pow(1-t,2)*p2.y + 3*pow(t,2)*(1-t)*p3.y + pow(t,3)*p4.y,
                     pow(1-t,3)*p1.z + 3*t*pow(1-t,2)*p2.z + 3*pow(t,2)*(1-t)*p3.z + pow(t,3)*p4.z,
                     1.0f
                     );
}

struct ObjectProperties SetObjectProperties(const char* name_model,
                                            glm::vec4 initial_position,
                                            float initial_rotation,
                                            glm::vec4 initial_forward)
{
    struct ObjectProperties obj_p;
    obj_p.name_model = name_model;
    obj_p.pos = initial_position;
    obj_p.rotation = initial_rotation;
    obj_p.forward = initial_forward;

    return obj_p;
}

// Função que faz a movimentação do personagem
void HandlePressedKeys()
{
    bool collided = false;
    for(int i =0 ; i < total_vaquinhas; i++){
        collided = collided || CheckBoxBoxCollision("cow", vaquinhas[i].pos);
    }
    collided = collided || CheckBoxBoxCollision("pig", porco.pos);
    for(int i = 0; i < KEYS; i++){
        if(!pressed[i]) continue;
        switch(i){
            case GLFW_KEY_W:
                if(!collided && !CheckPointPlaneCollision(player.pos + player.forward/norm(player.forward))){
                    player.pos.x += (player.forward.x*player.forward_velocity*delta_time);
                    player.pos.z += (player.forward.z*player.forward_velocity*delta_time);
                }
                break;
            case GLFW_KEY_S:
                if(!CheckPointPlaneCollision(player.pos - player.forward/norm(player.forward))){
                    player.pos.x -= (player.forward.x*player.forward_velocity*delta_time);
                    player.pos.z -= (player.forward.z*player.forward_velocity*delta_time);
                }
                break;
            case GLFW_KEY_D:
                player.rotation -= delta*player.turning_velocity*delta_time;
                g_CameraTheta -= delta*player.turning_velocity*delta_time;
                break;
            case GLFW_KEY_A:
                player.rotation += delta*player.turning_velocity*delta_time;
                g_CameraTheta += delta*player.turning_velocity*delta_time;
                break;
        }
    }
}

// Calcula quantas vaquinhas já foram interagidas
int calculateAderidas()
{
    int cont=0;
    for(bool aderida:aderidas){
        if(aderida)
            cont++;
    }
    return cont;
}

// Funções de deteccção se está próximo a outro objeto/npc
// Point-Planes Colission Test
bool CheckPointPlaneCollision(glm::vec4 p)
{
    float limit = boundary-1.5f;
    float error = 0.1f;
    if(p.x >= limit - error && p.x <= limit + error)
       return true;
    if(p.z >= limit - error && p.z <= limit + error)
       return true;

    limit = -boundary-0.5f;
    if(p.x >= limit - error && p.x <= limit + error)
       return true;
    if(p.z >= limit - error && p.z <= limit + error)
       return true;
    return false;
}
// Box-Box Colission Test V1
bool CheckBoxBoxCollision(const char* object, glm::vec3 offset)
{
    glm::vec3 obj_bbox_max = g_VirtualScene[object].bbox_max + offset;
    glm::vec3 obj_bbox_min = g_VirtualScene[object].bbox_min + offset;

    glm::vec4 player_bbox_max = glm::vec4(g_VirtualScene[player.name_model].bbox_max,0.0f) + player.pos;
    glm::vec4 player_bbox_min = glm::vec4(g_VirtualScene[player.name_model].bbox_min,0.0f) + player.pos;

    player_bbox_max.x = player_bbox_max.x + 0.5*player.forward.x/norm(player.forward);
    player_bbox_max.z = player_bbox_max.z + 1.5f*player.forward.z/norm(player.forward);
    player_bbox_min.x = player_bbox_min.x + 0.5*player.forward.x/norm(player.forward);
    player_bbox_min.z = player_bbox_min.z + 1.5f*player.forward.z/norm(player.forward);

    bool xCollided = (player_bbox_min.x <= obj_bbox_max.x && player_bbox_max.x >= obj_bbox_min.x);
    bool yCollided = true;
    bool zCollided = (player_bbox_min.z <= obj_bbox_max.z && player_bbox_max.z >= obj_bbox_min.z);

    return xCollided && yCollided && zCollided;
}
// Box-Box Colission Test V2
bool CheckBoxBoxCollision(const char* object1, const char* object2, glm::vec3 offset1, glm::vec3 offset2)
{
    glm::vec3 obj1_bbox_max = g_VirtualScene[object1].bbox_max + offset1;
    glm::vec3 obj1_bbox_min = g_VirtualScene[object1].bbox_min + offset1;
    glm::vec3 obj2_bbox_max = g_VirtualScene[object2].bbox_max + offset2;
    glm::vec3 obj2_bbox_min = g_VirtualScene[object2].bbox_min + offset2;

    bool xCollided = (obj1_bbox_min.x <= obj2_bbox_max.x && obj1_bbox_max.x >= obj2_bbox_min.x);
    bool yCollided = true;
    bool zCollided = (obj1_bbox_min.z <= obj2_bbox_max.z && obj1_bbox_max.z >= obj2_bbox_min.z);

    return xCollided && yCollided && zCollided;
}
// Box-Sphere Colission Test
bool CheckBoxSphereCollision(const char* object1, const char* object2, glm::vec3 offset1, glm::vec3 offset2, float radius_offset)
{
    glm::vec3 obj1_bbox_max = g_VirtualScene[object1].bbox_max + offset1;
    glm::vec3 obj1_bbox_min = g_VirtualScene[object1].bbox_min + offset1;
    glm::vec3 obj2_bbox_max = g_VirtualScene[object2].bbox_max + offset2;
    glm::vec3 obj2_bbox_min = g_VirtualScene[object2].bbox_min + offset2;

    glm::vec3 sphere_center = obj2_bbox_max + obj2_bbox_min;
    sphere_center.x /= 2;
    sphere_center.y /= 2;
    sphere_center.z /= 2;

    float sphere_radius = sqrt(pow(obj2_bbox_max.x - sphere_center.x,2)
                                + pow(obj2_bbox_max.y - sphere_center.y,2)
                                + pow(obj2_bbox_max.z - sphere_center.z,2))
                            + radius_offset;

    float x = fmax(obj1_bbox_min.x, fmin(sphere_center.x, obj1_bbox_max.x));
    float y = fmax(obj1_bbox_min.y, fmin(sphere_center.y, obj1_bbox_max.y));
    float z = fmax(obj1_bbox_min.z, fmin(sphere_center.z, obj1_bbox_max.z));

    float distance = sqrt(pow(x - sphere_center.x,2) +
                        pow(y - sphere_center.y,2) +
                        pow(z - sphere_center.z,2));

    return distance < sphere_radius;
}

int nearCow()
{
    for(int i=0; i<total_vaquinhas; i++){
        if(CheckBoxSphereCollision("cow","cow", glm::vec3(player.pos), glm::vec3(vaquinhas[i].pos), 0.75f))
           return i+1;
    }
    return 0;
}

bool nearPig()
{
    return CheckBoxSphereCollision("cow","pig", glm::vec3(player.pos), glm::vec3(porco.pos), 1.5f);
}

bool nearGate()
{
    return CheckBoxSphereCollision("cow","plane", glm::vec3(player.pos), glm::vec3(gate.pos), 1.0f);
}

void rotateGate(bool open)
{
    if(open){
        if(gate.rotation < (M_PI/2))
            gate.rotation += delta*gate.turning_velocity*delta_time;
    }
    else{
        if(gate.rotation >= 0.0f)
            gate.rotation -= delta*gate.turning_velocity*delta_time;
    }
}

// Funções de renderização de falas/objetivos do jogo
void TextRendering_ShowObjectives(GLFWwindow* window)
{

    static char  buffer[35] = "";

    float lineheight = TextRendering_LineHeight(window);

    if(isMissionPigTalk){
        snprintf(buffer, 35, "Converse com o porquinho!");
        TextRendering_PrintString(window, buffer, -0.95f, 1.0f-lineheight, 1.0f);
        snprintf(buffer, 35, "Talvez ele tenha algo a dizer...");
        TextRendering_PrintString(window, buffer, -0.95f, 1.0f-2*lineheight, 1.0f);
    }
    else if(isMissionCowTalk){
        snprintf(buffer, 35, "Converse com outras vaquinhas!");
        TextRendering_PrintString(window, buffer, -0.95f, 1.0f-lineheight, 1.0f);

        snprintf(buffer, 35, "Vaquinhas a par da fuga: %d/%d", calculateAderidas(), total_vaquinhas);
        TextRendering_PrintString(window, buffer, -0.95f, 1.0f-2*lineheight, 1.0f);
    }
    else if(isMissionEscape){
        snprintf(buffer, 35, "FUJA!!!");
        TextRendering_PrintString(window, buffer, -0.95f, 1.0f-lineheight, 1.0f);
    }

}

void TextRendering_EToInterack(GLFWwindow* window)
{

    static char  buffer[25] = "";

    float lineheight = TextRendering_LineHeight(window);

    snprintf(buffer, 25, "Aperte E para interagir!");
    TextRendering_PrintString(window, buffer, -0.7f, -0.6f-lineheight, 1.0f);
}

void TextRendering_cowTalk(GLFWwindow* window)
{
    static char  buffer[11] = "";

    float lineheight = TextRendering_LineHeight(window);

    snprintf(buffer, 11, "Moo? Moo!!!");
    TextRendering_PrintString(window, buffer, -0.7f, -0.6f-lineheight, 1.0f);
}

void TextRendering_PigTalk(GLFWwindow* window)
{

    static char  buffer[75] = "";

    float lineheight = TextRendering_LineHeight(window);

    snprintf(buffer, 75, "Oinc, Oinc!");
    TextRendering_PrintString(window, buffer, -0.7f, -0.6f, 1.0f);

    snprintf(buffer, 75, "(Todos os animais sao iguais mas alguns sao mais iguais que os outros!)");
    TextRendering_PrintString(window, buffer, -0.7f, -0.6f-lineheight, 1.0f);
}

void TextRendering_GameEnd(GLFWwindow* window)
{
    static char  buffer[30] = "";

    snprintf(buffer, 30, "Proximo passo: Revolucao!");
    TextRendering_PrintString(window, buffer, -0.4f, 0.75f, 1.5f);

    snprintf(buffer, 30, "Aperte ESC para sair do jogo.");
    TextRendering_PrintString(window, buffer, -0.4f, -0.75f, 1.5f);
}

//Funções de Callback
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_ScreenRatio = (float)width / height;
}


double g_LastCursorPosX, g_LastCursorPosY;

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        g_LeftMouseButtonPressed = false;
    }
}


void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (g_LeftMouseButtonPressed && !game_end)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;

        // Limitamos phimin para a camera não atravessar o chão em nenhum momento
        float phimax = 3.141592f/2;
        float phimin = -phimax/10;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_CameraDistance -= 0.1f*yoffset;

    const float minCamera = 1.5f;
    const float maxCamera = 4.0f;
    if (g_CameraDistance < minCamera)
        g_CameraDistance = minCamera;
    if (g_CameraDistance > maxCamera)
        g_CameraDistance = maxCamera;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // ==============
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ==============

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);


    // Se o usuário aperta qualquer tecla ela é salva em um buffer para utilizar na movimentação
    if(action == GLFW_PRESS)
        pressed[key] = true;
    else if(action == GLFW_RELEASE)
        pressed[key] = false;


    // Se o usuário apertar a tecla E, dependendo do momento do jogo faz uma ação diferente.
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        if(nearPig())
        {
            render_pigTalk = true;
            isMissionPigTalk = false;
            isMissionCowTalk = true;
        }
        else if(nearCow() > 0 && isMissionCowTalk)
        {
            aderidas[nearCow()-1] = true;
            render_cowTalk = true;
        }
        else if(nearGate() && isMissionEscape){
            escaped = true;
        }
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }
}

void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}
