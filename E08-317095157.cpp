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
Model Lampara;
Model Jeep;
Model Jeep_Rueda1;
Model Jeep_Rueda2;
Model Jeep_Rueda3;
Model Jeep_Rueda4;
Model Jeep_Cofre;
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

// Crear octaedro
void CrearOctaedro()
{
    unsigned int octaedro_indices[] = {
        0, 1, 2,
        3, 5, 4,
        6, 7, 8,
        9, 11, 10,

        12, 14, 13,
        15, 16, 17,
        18, 20, 19,
        21, 22, 23
    };

    GLfloat octaedro_vertices[] = {
        // x,    y,    z,    S,   T,    NX, NY, NZ
         0.0f, 0.5f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.0f,  0.5f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.5f, 0.0f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,

         0.0f, 0.5f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.0f,  0.5f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
        -0.5f, 0.0f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,

         0.0f, 0.5f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.5f, 0.0f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.0f, -0.5f, 0.0f,0.0f, 0.0f,0.0f,0.0f,

         0.0f, 0.5f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
        -0.5f, 0.0f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.0f, -0.5f, 0.0f,0.0f, 0.0f,0.0f,0.0f,

         0.0f,-0.5f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.0f,  0.5f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.5f, 0.0f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,

         0.0f,-0.5f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.0f,  0.5f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
        -0.5f, 0.0f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,

         0.0f,-0.5f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.5f, 0.0f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.0f, -0.5f, 0.0f,0.0f, 0.0f,0.0f,0.0f,

         0.0f,-0.5f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
        -0.5f, 0.0f,  0.0f, 0.0f,0.0f, 0.0f,0.0f,0.0f,
         0.0f, 0.0f, -0.5f, 0.0f,0.0f, 0.0f,0.0f,0.0f
    };

    calcAverageNormals(octaedro_indices, 24, octaedro_vertices, 192, 8, 5);

    Mesh* octaedro = new Mesh();
    octaedro->CreateMesh(octaedro_vertices, octaedro_indices, 192, 24);
    meshList.push_back(octaedro);
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
glm::vec3 offsetFaro(0.3f, -0.1f, 0.0f);
glm::vec3 posicionFaro(0.0f, 0.0f, 0.0f);

int main() {
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CrearOctaedro();
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

    Lampara.LoadModel("Models/lampara_no_texturizada.obj");

    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

    Jeep.LoadModel("Models/jeep.obj");
    Jeep_Rueda1.LoadModel("Models/jeep_rueda1.obj");
    Jeep_Rueda2.LoadModel("Models/jeep_rueda2.obj");
    Jeep_Rueda3.LoadModel("Models/jeep_rueda3.obj");
    Jeep_Rueda4.LoadModel("Models/jeep_rueda4.obj");
    Jeep_Cofre.LoadModel("Models/jeep_cofre.obj");

    skybox = Skybox(skyboxFaces);

    Material_brillante = Material(4.0f, 256);
    Material_opaco = Material(0.3f, 4);

    // Luz que siempre debe existir
    mainLight = DirectionalLight(
        1.0f, 1.0f, 1.0f,
        0.3f, 0.3f,
        0.0f, 0.0f, -1.0f
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

    // Luces del faro
    spotLights[0] = SpotLight(
        1.0f, 0.0f, 0.0f, // Rojo
        1.0f, 2.0f, 
        0.0f, 0.0f, 0.0f, 
       -1.0f, 0.0f, 0.0f, 
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;

    spotLights[1] = SpotLight(
        1.0f, 1.0f, 0.0f, // Amarillo
        1.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;

    spotLights[2] = SpotLight(
        0.0f, 1.0f, 0.0f, // Verde
        1.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;

    spotLights[3] = SpotLight(
        0.0f, 1.0f, 1.0f, // Cian
        1.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;

    spotLights[4] = SpotLight(
        0.0f, 0.0f, 1.0f, // Azul
        1.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;

    spotLights[5] = SpotLight(
        1.0f, 0.0f, 1.0f, // Magenta
        1.0f, 2.0f,
        0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        15.0f);
    spotLightCount++;

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

        if (mainWindow.getLamparaEncendida())
        {
            shaderList[0].SetPointLights(pointLights, pointLightCount);
        }
        else
        {
            shaderList[0].SetPointLights(pointLights, 0);
        }

        shaderList[0].SetSpotLights(&spotLights[mainWindow.getColorFaro()], 1);

        // Piso
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[0]->RenderMesh();

        /*Ejercicio 1: Agregar su dado de 8 caras y editar sus normales para que 
                     las caras del dado sean iluminadas correctamente.*/
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(1.0f, 2.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        meshList[1]->RenderMesh();

        //Ejercicio 2: Apagar con teclado la luz(pointlight) de su lámpara
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara.RenderModel();

        /*Ejercicio 3: Hacer que al presionar una tecla, 
                      la luz del faro del coche cicle de color entre rojo, 
                      amarillo, verde, cian, azul, magenta*/
        // Instancia del coche 
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f + mainWindow.getmuevex(), 0.5f, -3.0f));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep.RenderModel();
        modelaux = model;

        //Llanta delantera izquierda
        model = modelaux;
        model = glm::rotate(model,glm::radians(mainWindow.getarticulacion1()),glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(-0.79f, -0.3f, 1.28f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Rueda1.RenderModel();

        //Llanta trasera izquierda
        model = modelaux;
        model = glm::rotate(model,glm::radians(mainWindow.getarticulacion1()),glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.74f, -0.3f, 1.28f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Rueda2.RenderModel();

        //Llanta delantera derecha
        model = modelaux;
        model = glm::rotate(model,glm::radians(mainWindow.getarticulacion1()),glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(-0.79f, -0.3f, -1.1f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Rueda3.RenderModel();

        //Llanta trasera derecha
        model = modelaux;
        model = glm::rotate(model,glm::radians(mainWindow.getarticulacion1()),glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.74f, -0.3f, -1.1f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Rueda4.RenderModel();

        // Cofre
        model = modelaux;
        model = glm::rotate(model,glm::radians(mainWindow.getarticulacion2()),glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0.0f, 0.4f, 1.2f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Cofre.RenderModel();

        // Luz
        posicionFaro = glm::vec3(model * glm::vec4(offsetFaro, 1.0f));
        for (int i = 0; i < 6; i++)
        {
            spotLights[i].SetPos(posicionFaro);
        }

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}