/*
    Práctica 7: Iluminación 1
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
Model Blackhawk_M;
Model Lampara;
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

    Blackhawk_M.LoadModel("Models/uh60.obj");
    Lampara.LoadModel("Models/lampara_no_texturizada.obj");

    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

    skybox = Skybox(skyboxFaces);

    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    mainLight = DirectionalLight(
        1.0f, 1.0f, 1.0f,
        0.1f, 0.1f,
        0.0f, 0.0f, 1.0f
    );

    unsigned int pointLightCount = 0;

    // Luz lampara
    pointLights[0] = PointLight(
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f
    );
    pointLightCount++;

    unsigned int spotLightCount = 0;

    // Linterna de la camara
    spotLights[0] = SpotLight(
        1.0f, 1.0f, 1.0f,
        0.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        5.0f
    );
    spotLightCount++;

    // Luz amarilla del helicoptero
    spotLights[1] = SpotLight(
        1.0f, 1.0f, 0.0f,   // amarillo
        1.0f, 2.0f,         // intensidades
        0.0f, 0.0f, 0.0f,   // posicion
        0.0f, -1.0f, 0.0f,  // apunta hacia abajo
        1.0f, 0.0f, 0.0f,   // atenuacion
        15.0f               // angulo
    );
    spotLightCount++;

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

        glm::vec3 lowerLight = camera.getCameraPosition();
        lowerLight.y -= 0.3f;
        spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        glm::mat4 model(1.0f);
        glm::vec3 color(1.0f, 1.0f, 1.0f);

        // Piso
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[0]->RenderMesh();

        // Helicóptero
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 9.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f + mainWindow.getmuevex2(), 0.5f, -3.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Blackhawk_M.RenderModel();

        // Posición de la luz amarilla del helicóptero
        glm::vec3 offsetFaro = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 posicionFaro = glm::vec3(model * glm::vec4(offsetFaro, 1.0f));
        spotLights[1].SetPos(posicionFaro);

        // Lámpara
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara.RenderModel();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}