/*
Práctica: Modelos jerárquicos + carga de modelos (.obj) + skybox

Objetivo general:
- Renderizar un modelo complejo (Goddard) compuesto por múltiples partes.
- Implementar jerarquía de transformaciones (mandíbula, patas, cola).
- Integrar entorno (skybox) y piso.
*/

#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

/// Conversión grados → radianes (aunque GLM ya tiene glm::radians)
const float toRadians = 3.14159265f / 180.0f;

///----------------------------------------
/// OBJETOS GLOBALES
///----------------------------------------

Window mainWindow;

/// Lista de mallas simples (piso y pruebas)
std::vector<Mesh*> meshList;

/// Lista de shaders
std::vector<Shader> shaderList;

/// Cámara de la escena
Camera camera;

/// Modelos .obj de Goddard (cada parte separada)
Model Goddard_M;
Model Goddard_Mandibula;
Model Goddard_Pata1;
Model Goddard_Pata2;
Model Goddard_Pata3;
Model Goddard_Pata4;
Model Goddard_Cola;

/// Skybox (entorno)
Skybox skybox;

/// Control de tiempo (para FPS independiente)
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

/// Shaders
static const char* vShader = "shaders/shader_m.vert";
static const char* fShader = "shaders/shader_m.frag";


///------------------------------------------------------------
/// CREACIÓN DE OBJETOS BÁSICOS (mallas simples)
///
/// Aquí se crean:
/// - Un tetraedro (obj1 y obj2)
/// - Un plano (piso)
///
/// Nota:
/// Cada vértice contiene:
/// posición (x,y,z) + textura (u,v) + normales (nx,ny,nz)
///------------------------------------------------------------
void CreateObjects()
{
    unsigned int indices[] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices[] = {
        -1.0f, -1.0f, -0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.0f, -1.0f,  1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -0.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.0f,  1.0f,  0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f
    };

    /// Piso (plano grande)
    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, -10.0f, 10.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f,  10.0f, 0.0f, 10.0f, 0.0f, -1.0f, 0.0f,
         10.0f, 0.0f,  10.0f, 10.0f,10.0f, 0.0f, -1.0f, 0.0f
    };

    Mesh* obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj1);

    Mesh* obj2 = new Mesh();
    obj2->CreateMesh(vertices, indices, 32, 12);
    meshList.push_back(obj2);

    Mesh* obj3 = new Mesh();
    obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
    meshList.push_back(obj3);
}


///------------------------------------------------------------
/// CREACIÓN DE SHADERS
///
/// Carga shaders desde archivos y los almacena.
///------------------------------------------------------------
void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}



int main()
{
    ///----------------------------------------
    /// INICIALIZACIÓN
    ///----------------------------------------
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    /// Cámara inicial
    camera = Camera(
        glm::vec3(0.0f, 0.5f, 7.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f,
        0.0f,
        0.3f,
        1.0f
    );

    ///----------------------------------------
    /// CARGA DE MODELOS (.obj)
    ///----------------------------------------
    Goddard_M.LoadModel("Models/goddard_cuerpo.obj");
    Goddard_Mandibula.LoadModel("Models/goddard_mandibula.obj");
    Goddard_Pata1.LoadModel("Models/goddard_pata1.obj");
    Goddard_Pata2.LoadModel("Models/goddard_pata2.obj");
    Goddard_Pata3.LoadModel("Models/goddard_pata3.obj");
    Goddard_Pata4.LoadModel("Models/goddard_pata4.obj");
    Goddard_Cola.LoadModel("Models/goddard_cola.obj");

    ///----------------------------------------
    /// SKYBOX (entorno)
    ///----------------------------------------
    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/cupertin-lake_rt.tga",
        "Textures/Skybox/cupertin-lake_lf.tga",
        "Textures/Skybox/cupertin-lake_dn.tga",
        "Textures/Skybox/cupertin-lake_up.tga",
        "Textures/Skybox/cupertin-lake_bk.tga",
        "Textures/Skybox/cupertin-lake_ft.tga"
    };

    skybox = Skybox(skyboxFaces);

    ///----------------------------------------
    /// UNIFORMS
    ///----------------------------------------
    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformColor = 0;

    /// Matriz de proyección
    glm::mat4 projection = glm::perspective(
        45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f,
        1000.0f
    );

    /// Matrices de transformación
    glm::mat4 model(1.0);
    glm::mat4 modelaux(1.0);

    glm::vec3 color(1.0f, 1.0f, 1.0f);

    ///========================================================
    /// BUCLE PRINCIPAL
    ///========================================================
    while (!mainWindow.getShouldClose())
    {
        /// Tiempo
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        /// Entrada
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        /// Limpiar pantalla
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// Skybox
        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        /// Shader
        shaderList[0].UseShader();

        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        //----------------------------------------------------
        /// PISO
        //----------------------------------------------------
        color = glm::vec3(0.5f);

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0, -2, 0));
        model = glm::scale(model, glm::vec3(30, 1, 30));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[2]->RenderMesh();

        //----------------------------------------------------
        /// CUERPO (NODO RAÍZ)
        //----------------------------------------------------
        color = glm::vec3(0);

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0, 1, 0));

        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        Goddard_M.RenderModel();

        color = glm::vec3(0.0f, 0.0f, 1.0f);

        /// Guardamos transformación base
        modelaux = model;

        //----------------------------------------------------
        /// MANDÍBULA
        //----------------------------------------------------
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion6()), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(3.61f, 0.2f, 0.5f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Goddard_Mandibula.RenderModel();

        //----------------------------------------------------
        /// COLA
        //----------------------------------------------------
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion1()), glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(-1.4f, -1, 0.5f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Goddard_Cola.RenderModel();

        //----------------------------------------------------
        /// PATAS (jerárquicas)
        //----------------------------------------------------

        // Pata 1
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(1.5f, -1.5f, 1.2f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Goddard_Pata1.RenderModel();

        // Pata 2
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion3()), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(1.5f, -1.5f, 0));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Goddard_Pata2.RenderModel();

        // Pata 3
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion4()), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(-0.5f, -2, 0));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Goddard_Pata3.RenderModel();

        // Pata 4
        modelaux = model; // <---- Aquí me equivoqué (CORREGIR EN EL FUTURO)
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion5()), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(0, 0, 1));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Goddard_Pata4.RenderModel();

        //----------------------------------------------------
        /// FIN FRAME
        //----------------------------------------------------
        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}