/*
Práctica 05: Modelos jerárquicos + carga de modelos (.obj) + skybox

Objetivo general:
- Renderizar un modelo complejo (Jeep) compuesto por múltiples partes.
- Implementar jerarquía de transformaciones (ruedas y cofre).
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

/// Modelos .obj del Jeep (modelo general y cada parte separada)
Model Jeep;
Model Jeep_Rueda1;
Model Jeep_Rueda2;
Model Jeep_Rueda3;
Model Jeep_Rueda4;
Model Jeep_Cofre;

/// Skybox (entorno)
Skybox skybox;

/// Control de tiempo (para FPS independiente)
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

/// Shaders
static const char* vShader = "shaders/shader_m.vert";
static const char* fShader = "shaders/shader_m.frag";

Sphere sp = Sphere(1.0f, 20, 20);

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

    /// Inicialización de la esfera.
    ///
    /// Aunque no se usa en el render activo, se deja cargada
    /// porque el bloque comentado sí la utiliza.
    sp.init();
    sp.load();

    ///----------------------------------------
    /// CARGA DE MODELOS (.obj)
    ///----------------------------------------

    Jeep.LoadModel("Models/jeep.obj");
    Jeep_Rueda1.LoadModel("Models/jeep_rueda1.obj");
    Jeep_Rueda2.LoadModel("Models/jeep_rueda2.obj");
    Jeep_Rueda3.LoadModel("Models/jeep_rueda3.obj");
    Jeep_Rueda4.LoadModel("Models/jeep_rueda4.obj");
    Jeep_Cofre.LoadModel("Models/jeep_cofre.obj");

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
        model = glm::translate(model, glm::vec3(0, 0, 0)); // había un -2 en 'Y' 
        model = glm::scale(model, glm::vec3(30, 1, 30));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[2]->RenderMesh();

        //----------------------------------------------------
        /// CUERPO (NODO RAÍZ)
        //----------------------------------------------------
        color = glm::vec3(0);

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f)); // Flota un poquito pero ya está bien 

        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep.RenderModel();

        color = glm::vec3(0.0f, 0.0f, 1.0f);

        /// Guardamos transformación base
        modelaux = model;

        //----------------------------------------------------
        /// Ruedas (jerárquicas)
        //----------------------------------------------------

        // Rueda 1
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(-0.79f, -0.3f, 1.28f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Rueda1.RenderModel();

        // Rueda 2
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0.74f, -0.3f, 1.28f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Rueda2.RenderModel();

        // Rueda 3
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(-0.79f, -0.3f, -1.1f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Rueda3.RenderModel();

        // Rueda 4
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0.74f, -0.3f, -1.1f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Rueda4.RenderModel();

        ////----------------------------------------------------
        ///// Cofre
        ////----------------------------------------------------
        model = modelaux;
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion3()), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0.0f, 0.4f, 1.2f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Jeep_Cofre.RenderModel();

        //----------------------------------------------------
        /// FIN FRAME
        //----------------------------------------------------
        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}