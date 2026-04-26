/*
    Práctica 8: Iluminación 2
*/
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <vector>
#include <cmath>

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

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture pisoTexture;
Model Acuario;
Model Nave;
Model Pez;
Skybox skybox;

Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0f;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// Cálculo de normales
void calcAverageNormals(
    unsigned int* indices,
    unsigned int indiceCount,
    GLfloat* vertices,
    unsigned int verticeCount,
    unsigned int vLength,
    unsigned int normalOffset
)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;

        glm::vec3 v1(
            vertices[in1] - vertices[in0],
            vertices[in1 + 1] - vertices[in0 + 1],
            vertices[in1 + 2] - vertices[in0 + 2]
        );

        glm::vec3 v2(
            vertices[in2] - vertices[in0],
            vertices[in2 + 1] - vertices[in0 + 1],
            vertices[in2 + 2] - vertices[in0 + 2]
        );

        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);

        in0 += normalOffset;
        in1 += normalOffset;
        in2 += normalOffset;

        vertices[in0] += normal.x;
        vertices[in0 + 1] += normal.y;
        vertices[in0 + 2] += normal.z;

        vertices[in1] += normal.x;
        vertices[in1 + 1] += normal.y;
        vertices[in1 + 2] += normal.z;

        vertices[in2] += normal.x;
        vertices[in2 + 1] += normal.y;
        vertices[in2 + 2] += normal.z;
    }

    for (size_t i = 0; i < verticeCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(
            vertices[nOffset],
            vertices[nOffset + 1],
            vertices[nOffset + 2]
        );

        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x;
        vertices[nOffset + 1] = vec.y;
        vertices[nOffset + 2] = vec.z;
    }
}
void CreateObjects() {
    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
    -10.0f, 0.0f, -10.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,
     10.0f, 0.0f, -10.0f, 10.0f,  0.0f,  0.0f, 1.0f, 0.0f,
    -10.0f, 0.0f,  10.0f,  0.0f, 10.0f,  0.0f, 1.0f, 0.0f,
     10.0f, 0.0f,  10.0f, 10.0f, 10.0f,  0.0f, 1.0f, 0.0f
    };

    Mesh* piso = new Mesh();
    piso->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(piso);
}

void CreateShaders() {
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

    glm::mat4 model(1.0f);
    glm::mat4 modelaux(1.0f);
    glm::mat4 view(1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);
    glm::vec3 offsetNave(0.3f, -0.1f, 0.0f);
    glm::vec3 posicionNave(0.0f, 0.0f, 0.0f);
    // Nuevo
    GLfloat movimientoPez = 0.0f;
    // Nuevo
    glm::vec3 posicionBulboPez(0.0f, 0.0f, 0.0f);

int main() {
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f,
        0.0f,
        0.3f,
        0.5f
    );

    pisoTexture = Texture("Textures/piso.tga");
    pisoTexture.LoadTextureA();

    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

    Acuario.LoadModel("Models/acuario.obj");
    Pez.LoadModel("Models/pez-abisal.obj");
    Nave.LoadModel("Models/nave.obj");

    skybox = Skybox(skyboxFaces);

    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    // Luz que siempre debe existir
    mainLight = DirectionalLight(
        1.0f, 1.0f, 1.0f,
        0.3f, 0.3f,
        0.0f, 0.0f, -1.0f
    );
    unsigned int spotLightCount = 0;

    // Luz amarilla del helicoptero
    spotLights[0] = SpotLight(
        1.0f, 1.0f, 0.0f,   // amarillo
        1.0f, 2.0f,         // intensidades
        0.0f, 0.0f, 0.0f,   // posicion
        0.0f, -1.0f, 0.0f,  // apunta hacia abajo
        1.0f, 0.0f, 0.0f,   // atenuacion
        35.0f               // angulo
    );
    spotLightCount++;

    unsigned int pointLightCount = 0;

    // Luz bulbo pez
    pointLights[0] = PointLight(
        0.2f, 0.8f, 1.0f,   // color azul/cian
        0.3f, 8.0f,         // ambiente y difusa más fuerte
        10.0f, 2.0f, 0.0f,  // posición inicial cerca del pez
        1.0f, 0.09f, 0.032f // atenuación más visible
    );
    pointLightCount++;

    //se crean mas luces puntuales y spotlight 
    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformEyePosition = 0;
    GLuint uniformSpecularIntensity = 0;
    GLuint uniformShininess = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(
        45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f,
        1000.0f
    );

    while (!mainWindow.getShouldClose()) {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
        shaderList[0].UseShader();

        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformColor = shaderList[0].getColorLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(
            uniformEyePosition,
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z
        );

        shaderList[0].SetDirectionalLight(&mainLight);

        // Posición de la luz de la nave
        posicionNave = glm::vec3(mainWindow.getmuevex(),14.0f,-3.0f);
        spotLights[0].SetPos(posicionNave);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        // Movimiento actual del pez
        GLfloat movPez = mainWindow.getMovimientoPez();

        // Posición aproximada del bulbo del pez
        posicionBulboPez = glm::vec3(
            10.0f + movPez + 0.25f,        // X del pez + ajuste hacia el frente
            2.0f + movPez * 0.5f + 0.25f,  // Y del pez + ajuste hacia arriba
            0.0f
        );

        // Actualizar posición de la luz puntual del pez
        pointLights[0].SetPos(posicionBulboPez);

        if (mainWindow.getBulboEncendido())
        {
            shaderList[0].SetPointLights(pointLights, pointLightCount);
        }
        else
        {
            shaderList[0].SetPointLights(pointLights, 0);
        }

        // Piso
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[0]->RenderMesh();

        // Nave
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 15.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f + mainWindow.getmuevex(), 0.5f, -3.0f));
        model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nave.RenderModel();

        // Acuario
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, 3.5f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Acuario.RenderModel();

        // Pez
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f + movPez, 2.0f + movPez * 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Pez.RenderModel();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}