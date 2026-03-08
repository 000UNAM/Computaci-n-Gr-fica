#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/random.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

const float toRadians = 3.14159265f / 180.0f;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

Camera camera;
Window mainWindow;

vector<Mesh*> meshList;
vector<Shader> shaderList;

static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";

Sphere sp = Sphere(1.0, 20, 20);

void CrearPiramideTriangular()
{
    unsigned int indices[] = {
        0,1,2,
        1,3,2,
        3,0,2,
        1,0,3
    };

    GLfloat vertices[] = {
        -0.5f,-0.5f,0.0f,
         0.5f,-0.5f,0.0f,
         0.0f, 0.5f,-0.5f,
         0.0f,-0.5f,-1.0f
    };

    Mesh* obj = new Mesh();
    obj->CreateMesh(vertices, indices, 12, 12);
    meshList.push_back(obj);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);

    Shader* shader2 = new Shader();
    shader2->CreateFromFiles(vShaderColor, fShader);
    shaderList.push_back(*shader2);
}

int main()
{
    mainWindow = Window(800, 800);
    mainWindow.Initialise();

    CrearPiramideTriangular();
    CreateShaders();

    camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f,
        0.1f, 0.5f
    );

    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f,
        100.0f
    );

    sp.init();
    sp.load();

    glm::mat4 model(1.0f);
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();

        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0].useShader();

        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        // Triangulo negro
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        color = glm::vec3(0.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul arriba 
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(0.0f, 0.3f, -2.34f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul en medio izquierdo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(-0.15f, 0.0f, -2.19f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul en medio derecho
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(0.15f, 0.0f, -2.19f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul en medio en medio volteado
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.19f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        model = glm::rotate(model, 51 * toRadians, glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul en medio hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(0.0f, -0.3f, -2.04f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul izquierdo hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(-0.30f, -0.3f, -2.04f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul girado
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(-0.15f, -0.3f, -2.04f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        model = glm::rotate(model, 51 * toRadians, glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul girado
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(0.15f, -0.3f, -2.04f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        model = glm::rotate(model, 51 * toRadians, glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo azul derecho hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.0f, 0.0f, 1.0f);

        model = glm::translate(model, glm::vec3(0.30f, -0.3f, -2.04f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo rojo arriba 
        model = glm::mat4(1.0f);
        color = glm::vec3(0.5f, 0.0f, 0.13f);

        model = glm::translate(model, glm::vec3(0.03f, 0.3f, -2.40f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo rojo en medio izquierdo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.5f, 0.0f, 0.13f);

        model = glm::translate(model, glm::vec3(0.18f, 0.0f, -2.25f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo rojo en medio en medio volteado <----- 
        model = glm::mat4(1.0f);
        color = glm::vec3(0.5f, 0.0f, 0.13f);

        model = glm::translate(model, glm::vec3(0.15f, 0.0f, -2.40f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        //model = glm::rotate(model, 180 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        //model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        //meshList[0]->RenderMesh();

        // Triangulo rojo en medio derecho
        model = glm::mat4(1.0f);
        color = glm::vec3(0.5f, 0.0f, 0.13f);

        model = glm::translate(model, glm::vec3(0.04f, 0.0f, -2.54f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo rojo en medio hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.5f, 0.0f, 0.13f);

        model = glm::translate(model, glm::vec3(0.18f, -0.3f, -2.40f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo rojo izquierdo hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.5f, 0.0f, 0.13f);

        model = glm::translate(model, glm::vec3(0.33f, -0.3f, -2.11f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo rojo derecho hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.5f, 0.0f, 0.13f);

        model = glm::translate(model, glm::vec3(0.04f, -0.3f, -2.69f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo rojo girado <-----
        model = glm::mat4(1.0f);
        color = glm::vec3(0.5f, 0.0f, 0.13f);

        model = glm::translate(model, glm::vec3(0.2f, -0.3f, -2.75f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        //meshList[0]->RenderMesh();

        // Triangulo oro arriba 
        model = glm::mat4(1.0f);
        color = glm::vec3(1.0f, 0.84f, 0.0f);

        model = glm::translate(model, glm::vec3(-0.03f, 0.3f, -2.40f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo oro en medio izquierdo
        model = glm::mat4(1.0f);
        color = glm::vec3(1.0f, 0.84f, 0.0f);

        model = glm::translate(model, glm::vec3(-0.18f, 0.0f, -2.25f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo oro en medio derecho
        model = glm::mat4(1.0f);
        color = glm::vec3(1.0f, 0.84f, 0.0f);

        model = glm::translate(model, glm::vec3(-0.04f, 0.0f, -2.54f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo oro en medio hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(1.0f, 0.84f, 0.0f);

        model = glm::translate(model, glm::vec3(-0.18f, -0.3f, -2.40f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo oro izquierdo hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(1.0f, 0.84f, 0.0f);

        model = glm::translate(model, glm::vec3(-0.33f, -0.3f, -2.11f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo oro derecho hasta abajo
        model = glm::mat4(1.0f);
        color = glm::vec3(1.0f, 0.84f, 0.0f);

        model = glm::translate(model, glm::vec3(-0.04f, -0.3f, -2.69f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo morado punta
        model = glm::mat4(1.0f);
        color = glm::vec3(0.8f, 0.3f, 0.8f);

        model = glm::translate(model, glm::vec3(0.0f, -0.41f, -2.7f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo morado en medio
        model = glm::mat4(1.0f);
        color = glm::vec3(0.8f, 0.3f, 0.8f);

        model = glm::translate(model, glm::vec3(0.0f, -0.41f, -2.4f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo morado en medio izquierdo
        model = glm::mat4(1.0f);
        color = glm::vec3(0.8f, 0.3f, 0.8f);

        model = glm::translate(model, glm::vec3(-0.15f, -0.41f, -2.4f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo morado en medio derecho
        model = glm::mat4(1.0f);
        color = glm::vec3(0.8f, 0.3f, 0.8f);

        model = glm::translate(model, glm::vec3(0.15f, -0.41f, -2.4f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo morado hasta abajo en medio
        model = glm::mat4(1.0f);
        color = glm::vec3(0.8f, 0.3f, 0.8f);

        model = glm::translate(model, glm::vec3(0.0f, -0.41f, -2.1f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo morado hasta abajo en medio
        model = glm::mat4(1.0f);
        color = glm::vec3(0.8f, 0.3f, 0.8f);

        model = glm::translate(model, glm::vec3(-0.27f, -0.41f, -2.1f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        // Triangulo morado hasta abajo en medio
        model = glm::mat4(1.0f);
        color = glm::vec3(0.8f, 0.3f, 0.8f);

        model = glm::translate(model, glm::vec3(0.27f, -0.41f, -2.1f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
// Por si quiero rotar una figura completa
//model = glm::rotate(model, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f));
//model = glm::rotate(model, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f)); 
//model = glm::rotate(model, glm::radians(mainWindow.getrotaz()), glm::vec3(0.0f, 0.0f, 1.0f));