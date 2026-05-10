#define STB_IMAGE_IMPLEMENTATION

#include <cmath>
#include <cstdio>
#include <vector>
#include <string>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

// -----------------------------------------------------------------------------
// Objetos globales basicos
// -----------------------------------------------------------------------------
Window mainWindow;
Camera camera;
Skybox skybox;

std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Texture pisoTexture;
Model Nave_M;
Model Ala_M;

Material Material_brillante;
Material Material_opaco;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// -----------------------------------------------------------------------------
// Keyframes de la nave
// -----------------------------------------------------------------------------
const int MAX_FRAMES = 100;
const int STEPS_BETWEEN_FRAMES = 90;

struct EstadoNave
{
    float x;
    float y;
    float z;
    float giroZ;
};

struct Frame
{
    float x;
    float y;
    float z;
    float giroZ;

    float xInc;
    float yInc;
    float zInc;
    float giroZInc;
};

EstadoNave nave = { 0.0f, 0.0f, 0.0f, 180.0f };

// -----------------------------------------------------------------------------
// Aleteo de alas
// -----------------------------------------------------------------------------
// alaAngulo cambia con una onda senoidal.
// Si el ala gira en un eje raro, cambia el eje en renderNave(), donde dice:
// glm::vec3(1.0f, 0.0f, 0.0f)
EstadoNave naveAnterior = nave;
float alaTiempo = 0.0f;
float alaAngulo = 0.0f;

const float ALA_ANGULO_MAX = 28.0f;
const float ALA_VELOCIDAD = 12.0f;
const float ALA_RETORNO = 8.0f;

Frame keyFrame[MAX_FRAMES];

int frameCount = 0;
int playIndex = 0;
int currentStep = 0;
bool play = false;

// La ruta esta centrada con un desplazamiento para que se vea mejor en camara.
const glm::vec3 origenNave(-4.0f, 2.0f, 0.0f);

// Ruta sugerida: dos ondas de ida, giro en el extremo y regreso por la misma ruta.
// Ida: orientacion contraria = 180 grados.
// Regreso: despues de girar = 0 grados.
const EstadoNave rutaSugerida[] = {
    {  0.0f,  0.0f, 0.0f, 180.0f },
    {  1.0f, -1.0f, 0.0f, 180.0f },
    {  2.0f,  0.0f, 0.0f, 180.0f },
    {  3.0f, -1.0f, 0.0f, 180.0f },
    {  4.0f,  0.0f, 0.0f, 180.0f },

    // Giro en el extremo
    {  4.0f,  0.0f, 0.0f,   0.0f },

    // Regreso
    {  3.0f, -1.0f, 0.0f,   0.0f },
    {  2.0f, -2.0f, 0.0f,   0.0f },
    {  1.0f, -1.0f, 0.0f,   0.0f },
    {  0.0f, -2.0f, 0.0f,   0.0f },
    { -1.0f, -1.0f, 0.0f,   0.0f },
    { -2.0f, -2.0f, 0.0f,   0.0f },
    { -3.0f, -1.0f, 0.0f,   0.0f },

    // Segundo giro
    { -2.0f,  0.0f, 0.0f, 180.0f },

    // Estos dos los agrego para que termine en el punto inicial
    { -1.0f, -1.0f, 0.0f, 180.0f },
    {  0.0f,  0.0f, 0.0f, 180.0f }
};
const int RUTA_SUGERIDA_COUNT = sizeof(rutaSugerida) / sizeof(rutaSugerida[0]);
int rutaSugeridaIndex = 0;


// -----------------------------------------------------------------------------
// Utilidades
// -----------------------------------------------------------------------------
bool keyPressedOnce(bool* keys, int key)
{
    static bool previous[1024] = { false };

    bool pressed = keys[key] && !previous[key];
    previous[key] = keys[key];

    return pressed;
}

float normalizeAngle(float angle)
{
    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;
    return angle;
}

void printControls()
{
    printf("\n================= CONTROLES KEYFRAMES =================\n");
    printf("Flecha derecha / izquierda : mover nave en X\n");
    printf("Flecha arriba / abajo      : mover nave en Y\n");
    printf("Q / E                      : rotar nave -10 / +10 grados\n");
    printf("G                          : girar nave 180 grados\n");
    printf("L                          : guardar keyframe actual POR TECLADO\n");
    printf("C                          : colocar siguiente cuadro sugerido de la ruta senoidal\n");
    printf("Z                          : borrar ultimo keyframe\n");
    printf("R                          : reiniciar nave y borrar keyframes\n");
    printf("ESPACIO                    : reproducir animacion guardada\n");
    printf("H                          : mostrar ayuda\n");
    printf("========================================================\n\n");
}

void resetAnimationAndFrames()
{
    nave = { 0.0f, 0.0f, 0.0f, 180.0f };
    frameCount = 0;
    playIndex = 0;
    currentStep = 0;
    play = false;
    rutaSugeridaIndex = 0;

    // Reinicia también el aleteo.
    naveAnterior = nave;
    alaTiempo = 0.0f;
    alaAngulo = 0.0f;

    printf("Animacion reiniciada. No hay keyframes guardados.\n");
}

void cargarRutaSugeridaComoKeyframes()
{
    resetAnimationAndFrames();

    int totalFrames = RUTA_SUGERIDA_COUNT;

    if (totalFrames > MAX_FRAMES)
    {
        totalFrames = MAX_FRAMES;
    }

    for (int i = 0; i < totalFrames; i++)
    {
        keyFrame[i].x = rutaSugerida[i].x;
        keyFrame[i].y = rutaSugerida[i].y;
        keyFrame[i].z = rutaSugerida[i].z;
        keyFrame[i].giroZ = rutaSugerida[i].giroZ;

        keyFrame[i].xInc = 0.0f;
        keyFrame[i].yInc = 0.0f;
        keyFrame[i].zInc = 0.0f;
        keyFrame[i].giroZInc = 0.0f;
    }

    frameCount = totalFrames;
    rutaSugeridaIndex = totalFrames;

    nave.x = keyFrame[0].x;
    nave.y = keyFrame[0].y;
    nave.z = keyFrame[0].z;
    nave.giroZ = keyFrame[0].giroZ;

    printf("Ruta sugerida cargada como animacion fija con %d keyframes.\n", frameCount);
}

void saveFrame()
{
    if (frameCount >= MAX_FRAMES)
    {
        printf("No se pueden guardar mas keyframes. MAX_FRAMES = %d\n", MAX_FRAMES);
        return;
    }

    keyFrame[frameCount].x = nave.x;
    keyFrame[frameCount].y = nave.y;
    keyFrame[frameCount].z = nave.z;
    keyFrame[frameCount].giroZ = nave.giroZ;

    keyFrame[frameCount].xInc = 0.0f;
    keyFrame[frameCount].yInc = 0.0f;
    keyFrame[frameCount].zInc = 0.0f;
    keyFrame[frameCount].giroZInc = 0.0f;

    printf("KEYFRAME %02d GUARDADO POR TECLADO -> X: %.2f, Y: %.2f, Z: %.2f, GiroZ: %.2f\n",
        frameCount, nave.x, nave.y, nave.z, nave.giroZ);

    frameCount++;
}

void deleteLastFrame()
{
    if (frameCount <= 0)
    {
        printf("No hay keyframes para borrar.\n");
        return;
    }

    frameCount--;
    printf("Se borro el keyframe %02d. Keyframes actuales: %d\n", frameCount, frameCount);
}

void applyNextSuggestedFrame()
{
    if (rutaSugeridaIndex >= RUTA_SUGERIDA_COUNT)
    {
        printf("Ya colocaste todos los cuadros sugeridos. Presiona ESPACIO para reproducir.\n");
        return;
    }

    nave = rutaSugerida[rutaSugeridaIndex];

    printf("Cuadro sugerido %02d colocado por teclado -> X: %.2f, Y: %.2f, GiroZ: %.2f. Presiona L para guardarlo.\n",
        rutaSugeridaIndex, nave.x, nave.y, nave.giroZ);

    rutaSugeridaIndex++;
}

void resetNaveToFirstFrame()
{
    nave.x = keyFrame[0].x;
    nave.y = keyFrame[0].y;
    nave.z = keyFrame[0].z;
    nave.giroZ = keyFrame[0].giroZ;

    naveAnterior = nave;
}

void calculateInterpolation()
{
    keyFrame[playIndex].xInc = (keyFrame[playIndex + 1].x - keyFrame[playIndex].x) / STEPS_BETWEEN_FRAMES;
    keyFrame[playIndex].yInc = (keyFrame[playIndex + 1].y - keyFrame[playIndex].y) / STEPS_BETWEEN_FRAMES;
    keyFrame[playIndex].zInc = (keyFrame[playIndex + 1].z - keyFrame[playIndex].z) / STEPS_BETWEEN_FRAMES;
    keyFrame[playIndex].giroZInc = (keyFrame[playIndex + 1].giroZ - keyFrame[playIndex].giroZ) / STEPS_BETWEEN_FRAMES;
}

void startAnimation()
{
    if (frameCount < 2)
    {
        printf("Necesitas guardar por lo menos 2 keyframes con L.\n");
        return;
    }

    play = true;
    playIndex = 0;
    currentStep = 0;
    resetNaveToFirstFrame();
    calculateInterpolation();

    printf("Reproduciendo animacion con %d keyframes creados por teclado.\n", frameCount);
}

void animate()
{
    if (!play)
    {
        return;
    }

    if (currentStep >= STEPS_BETWEEN_FRAMES)
    {
        playIndex++;

        if (playIndex > frameCount - 2)
        {
            // Al terminar, dejamos la nave exactamente en el ultimo keyframe.
            nave.x = keyFrame[frameCount - 1].x;
            nave.y = keyFrame[frameCount - 1].y;
            nave.z = keyFrame[frameCount - 1].z;
            nave.giroZ = keyFrame[frameCount - 1].giroZ;

            play = false;
            playIndex = 0;
            currentStep = 0;

            printf("Animacion terminada. La nave regreso al punto inicial.\n");
            return;
        }

        currentStep = 0;
        calculateInterpolation();
    }

    nave.x += keyFrame[playIndex].xInc;
    nave.y += keyFrame[playIndex].yInc;
    nave.z += keyFrame[playIndex].zInc;
    nave.giroZ += keyFrame[playIndex].giroZInc;
    currentStep++;
}


bool naveSeEstaMoviendo()
{
    const float epsilon = 0.0001f;

    if (play)
    {
        return true;
    }

    if (std::fabs(nave.x - naveAnterior.x) > epsilon) return true;
    if (std::fabs(nave.y - naveAnterior.y) > epsilon) return true;
    if (std::fabs(nave.z - naveAnterior.z) > epsilon) return true;
    if (std::fabs(nave.giroZ - naveAnterior.giroZ) > epsilon) return true;

    return false;
}

void actualizarAleteo()
{
    bool moviendose = naveSeEstaMoviendo();

    if (moviendose)
    {
        // Mientras la nave se mueve, las alas suben y bajan.
        alaTiempo += deltaTime * ALA_VELOCIDAD;
        alaAngulo = std::sin(alaTiempo) * ALA_ANGULO_MAX;
    }
    else
    {
        // Cuando la nave se detiene, las alas regresan suavemente a reposo.
        float factorRetorno = 1.0f - (ALA_RETORNO * deltaTime);

        if (factorRetorno < 0.0f)
        {
            factorRetorno = 0.0f;
        }

        alaAngulo *= factorRetorno;

        if (std::fabs(alaAngulo) < 0.1f)
        {
            alaAngulo = 0.0f;
        }
    }

    naveAnterior = nave;
}

void inputKeyframes(bool* keys)
{
    const float posStep = 1.0f;
    const float rotStep = 10.0f;

    if (keyPressedOnce(keys, GLFW_KEY_H)) printControls();
    if (keyPressedOnce(keys, GLFW_KEY_R)) resetAnimationAndFrames();

    if (keyPressedOnce(keys, GLFW_KEY_SPACE))
    {
        startAnimation();
    }

    if (play)
    {
        return;
    }

    // Movimiento manual para crear cuadros clave por teclado.
    if (keyPressedOnce(keys, GLFW_KEY_RIGHT)) nave.x += posStep;
    if (keyPressedOnce(keys, GLFW_KEY_LEFT))  nave.x -= posStep;
    if (keyPressedOnce(keys, GLFW_KEY_UP))    nave.y += posStep;
    if (keyPressedOnce(keys, GLFW_KEY_DOWN))  nave.y -= posStep;

    if (keyPressedOnce(keys, GLFW_KEY_Q)) nave.giroZ = normalizeAngle(nave.giroZ - rotStep);
    if (keyPressedOnce(keys, GLFW_KEY_E)) nave.giroZ = normalizeAngle(nave.giroZ + rotStep);

    // Giro rapido para el extremo de la trayectoria.
    if (keyPressedOnce(keys, GLFW_KEY_G)) nave.giroZ = normalizeAngle(nave.giroZ + 180.0f);

    // Ayuda opcional: coloca la siguiente posicion senoidal.
    // El keyframe NO se guarda hasta que presiones L.
    if (keyPressedOnce(keys, GLFW_KEY_C)) applyNextSuggestedFrame();

    if (keyPressedOnce(keys, GLFW_KEY_L)) saveFrame();
    if (keyPressedOnce(keys, GLFW_KEY_Z)) deleteLastFrame();
}

// -----------------------------------------------------------------------------
// Creacion de geometria sencilla
// -----------------------------------------------------------------------------
void createFloor()
{
    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        // x       y      z       u      v       nx     ny     nz
        -10.0f,  0.0f, -10.0f,   0.0f,  0.0f,   0.0f, -1.0f,  0.0f,
         10.0f,  0.0f, -10.0f,  10.0f,  0.0f,   0.0f, -1.0f,  0.0f,
        -10.0f,  0.0f,  10.0f,   0.0f, 10.0f,   0.0f, -1.0f,  0.0f,
         10.0f,  0.0f,  10.0f,  10.0f, 10.0f,   0.0f, -1.0f,  0.0f
    };

    Mesh* floorMesh = new Mesh();
    floorMesh->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(floorMesh);
}

void createShaders()
{
    shaderList.push_back(Shader());
    shaderList[0].CreateFromFiles(vShader, fShader);
}

void loadTextures()
{
    pisoTexture = Texture("Textures/piso.tga");
    pisoTexture.LoadTextureA();
}

void loadModels()
{
    Nave_M = Model();
    Nave_M.LoadModel("Models/nave.obj");

    Ala_M = Model();
    Ala_M.LoadModel("Models/ala.obj");
}

void createSkybox()
{
    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

    skybox = Skybox(skyboxFaces);
}

void createLights(unsigned int& pointLightCount, unsigned int& spotLightCount)
{
    mainLight = DirectionalLight(
        1.0f, 1.0f, 1.0f,
        0.3f, 0.3f,
        0.0f, 0.0f, -1.0f
    );

    pointLightCount = 0;
    pointLights[0] = PointLight(
        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 3.0f, 2.0f,
        0.3f, 0.2f, 0.1f
    );
    pointLightCount++;

    spotLightCount = 0;
    spotLights[0] = SpotLight(
        1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        5.0f
    );
    spotLightCount++;
}

void renderFloor(GLuint uniformModel, GLuint uniformColor, GLuint uniformTextureOffset,
    GLuint uniformSpecularIntensity, GLuint uniformShininess)
{
    glm::mat4 model(1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    glm::vec2 offset(0.0f, 0.0f);

    model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
    model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));

    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(color));
    glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(offset));

    pisoTexture.UseTexture();
    Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
    meshList[0]->RenderMesh();
}

void renderNave(GLuint uniformModel, GLuint uniformColor,
    GLuint uniformSpecularIntensity, GLuint uniformShininess)
{
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    glUniform3fv(uniformColor, 1, glm::value_ptr(color));
    Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);

    glm::mat4 naveBase(1.0f);
    naveBase = glm::translate(naveBase, origenNave + glm::vec3(nave.x, nave.y, nave.z));
    naveBase = glm::rotate(naveBase, nave.giroZ * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(naveBase));
    Nave_M.RenderModel();

    // Ala como hija de la nave: se mueve con el cuerpo y además aletea.
    glm::mat4 alaModel = naveBase;

    // Esta traslacion pega el ala al cuerpo.
    alaModel = glm::translate(alaModel, glm::vec3(0.0f, 0.0f, -0.3f));

    // Esta rotacion produce el aleteo.
    alaModel = glm::rotate(alaModel, alaAngulo * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(alaModel));
    Ala_M.RenderModel();
}

void clearMeshes()
{
    for (size_t i = 0; i < meshList.size(); i++)
    {
        delete meshList[i];
        meshList[i] = nullptr;
    }
    meshList.clear();
}

int main()
{
    mainWindow = Window(1366, 768);
    if (mainWindow.Initialise() != 0)
    {
        return 1;
    }

    createFloor();
    createShaders();
    loadTextures();
    loadModels();
    createSkybox();

    camera = Camera(
        glm::vec3(0.0f, 2.0f, 14.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f,
        -8.0f,
        4.0f,
        0.35f
    );

    Material_brillante = Material(4.0f, 256.0f);
    Material_opaco = Material(0.3f, 4.0f);

    unsigned int pointLightCount = 0;
    unsigned int spotLightCount = 0;
    createLights(pointLightCount, spotLightCount);

    glm::mat4 projection = glm::perspective(
        45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f,
        1000.0f
    );

    printControls();
    cargarRutaSugeridaComoKeyframes();
    printf("Para crear rapido la evidencia: repite C, L hasta terminar la ruta; despues presiona ESPACIO.\n");

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        glfwPollEvents();

        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        inputKeyframes(mainWindow.getsKeys());
        animate();
        actualizarAleteo();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        shaderList[0].UseShader();

        GLuint uniformModel = shaderList[0].GetModelLocation();
        GLuint uniformProjection = shaderList[0].GetProjectionLocation();
        GLuint uniformView = shaderList[0].GetViewLocation();
        GLuint uniformEyePosition = shaderList[0].GetEyePositionLocation();
        GLuint uniformColor = shaderList[0].getColorLocation();
        GLuint uniformTextureOffset = shaderList[0].getOffsetLocation();
        GLuint uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        GLuint uniformShininess = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(
            uniformEyePosition,
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z
        );

        glm::vec3 lowerLight = camera.getCameraPosition();
        lowerLight.y -= 0.3f;
        spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        renderFloor(uniformModel, uniformColor, uniformTextureOffset,
            uniformSpecularIntensity, uniformShininess);

        renderNave(uniformModel, uniformColor,
            uniformSpecularIntensity, uniformShininess);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    clearMeshes();
    return 0;
}
