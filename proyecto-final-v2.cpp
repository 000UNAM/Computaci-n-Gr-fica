#define STB_IMAGE_IMPLEMENTATION

#include <vector>
#include <cmath>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Skybox.h"
#include "Model.h"

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
Camera camera;

Mesh pisoIzquierdoMesh;
Mesh pisoDerechoMesh;
Shader shader;
Skybox skybox;

Texture pisoTexture;
Texture pisoTexture2;
Material materialOpaco;

Model Artorias;
Model Via_Recta;
Model Tren;
Model Lampara;
Model Libro;
Model Rotonda;
Model Engrane;
Model Quijote;
Model Via_Giro;
Model Medieval_House;
Model Arbol;
Model Edificio_Casa;
Model Lampara_calle;
Model Gravestone;
Model Dirigible;
Model Grave;
Model Espada;
Model Golem;
Model Nick_Pierna_Izq;
Model Nick_Pierna_Der;
Model Nick_Brazo_Izq;
Model Nick_Brazo_Der;
Model Nick_Cola;
Model Nick_Cabeza;
Model Nick_Camisa;
Model Nick_Pantalon;

DirectionalLight mainLight;

PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0f;

void CreateFloor()
{
    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorIzquierdoVertices[] = {

        -10.0f,  0.0f, -10.0f,   0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
          0.0f,  0.0f, -10.0f,  10.0f,  0.0f,   0.0f,  1.0f,  0.0f,
        -10.0f,  0.0f,  10.0f,   0.0f, 10.0f,   0.0f,  1.0f,  0.0f,
          0.0f,  0.0f,  10.0f,  10.0f, 10.0f,   0.0f,  1.0f,  0.0f
    };

    GLfloat floorDerechoVertices[] = {

          0.0f,  0.0f, -10.0f,   0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
         10.0f,  0.0f, -10.0f,  10.0f,  0.0f,   0.0f,  1.0f,  0.0f,
          0.0f,  0.0f,  10.0f,   0.0f, 10.0f,   0.0f,  1.0f,  0.0f,
         10.0f,  0.0f,  10.0f,  10.0f, 10.0f,   0.0f,  1.0f,  0.0f
    };

    pisoIzquierdoMesh.CreateMesh(floorIzquierdoVertices, floorIndices, 32, 6);
    pisoDerechoMesh.CreateMesh(floorDerechoVertices, floorIndices, 32, 6);
}

void CreateShader()
{
    shader.CreateFromFiles(vShader, fShader);
}

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    glEnable(GL_DEPTH_TEST);

    CreateFloor();
    CreateShader();

    camera = Camera(
        glm::vec3(0.0f, 2.0f, 8.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f,
        -10.0f,
        0.5f,
        0.5f
    );

    pisoTexture = Texture("Textures/piso.tga");
    pisoTexture.LoadTextureA();

    pisoTexture2 = Texture("Textures/piso-piedra-oxidado.tga");
    pisoTexture2.LoadTextureA();

    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

    skybox = Skybox(skyboxFaces);

    Artorias = Model();
    Artorias.LoadModel("Models/Knight_Artorias/c4100.obj");

    Via_Recta = Model();
    Via_Recta.LoadModel("Models/Ferrovia/Tr_Sec_1.obj");

    Via_Giro = Model();
    Via_Giro.LoadModel("Models/Ferrovia/Tr_Sec_6.obj");

    Tren = Model();
    Tren.LoadModel("Models/train.obj");

    Medieval_House = Model();
    Medieval_House.LoadModel("Models/GHouse_obj.obj");

    Libro = Model();
    Libro.LoadModel("Models/libro.obj");

    Lampara_calle = Model();
    Lampara_calle.LoadModel("Models/lampara_calle.obj");

    Rotonda = Model();
    Rotonda.LoadModel("Models/roundabout.obj");

    Edificio_Casa = Model();
    Edificio_Casa.LoadModel("Models/objHouyse.obj");

    Arbol = Model();
    Arbol.LoadModel("Models/arbol.obj");

    Gravestone = Model();
    Gravestone.LoadModel("Models/Gravestone LowPoLy.obj");

    Engrane = Model();
    Engrane.LoadModel("Models/gear.obj");

    Quijote = Model();
    Quijote.LoadModel("Models/quijote.obj");

    Dirigible = Model();
    Dirigible.LoadModel("Models/objZeppelin.obj");

    Grave = Model();
    Grave.LoadModel("Models/grave.obj");

    Espada = Model();
    Espada.LoadModel("Models/espada.obj");

    Golem = Model();
    Golem.LoadModel("Models/Golem/Golem/Stone.obj");

    Nick_Brazo_Der = Model();
    Nick_Brazo_Der.LoadModel("Models/nick while/nick while/Models/NWbrazoderecho.obj");

    Nick_Brazo_Izq = Model();
    Nick_Brazo_Izq.LoadModel("Models/nick while/nick while/Models/NWbrazoizquierdo.obj");

    Nick_Cabeza = Model();
    Nick_Cabeza.LoadModel("Models/nick while/nick while/Models/NWcabeza.obj");

    Nick_Camisa = Model();
    Nick_Camisa.LoadModel("Models/nick while/nick while/Models/NWcamisa.obj");

    Nick_Cola = Model();
    Nick_Cola.LoadModel("Models/nick while/nick while/Models/NWcola.obj");

    Nick_Pantalon = Model();
    Nick_Pantalon.LoadModel("Models/nick while/nick while/Models/NWpantalon.obj");

    Nick_Pierna_Der = Model();
    Nick_Pierna_Der.LoadModel("Models/nick while/nick while/Models/NWpiernaderecha.obj");

    Nick_Pierna_Izq = Model();
    Nick_Pierna_Izq.LoadModel("Models/nick while/nick while/Models/NWpiernaizquierda.obj");

    materialOpaco = Material(0.3f, 4.0f);

    mainLight = DirectionalLight(
        1.0f, 1.0f, 1.0f,
        0.5f, 0.5f,
        0.0f, -1.0f, -1.0f
    );

    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformEyePosition = 0;
    GLuint uniformSpecularIntensity = 0;
    GLuint uniformShininess = 0;
    GLuint uniformTextureOffset = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f,
        1000.0f
    );

    shader.UseShader();

    uniformModel = shader.GetModelLocation();
    uniformProjection = shader.GetProjectionLocation();
    uniformView = shader.GetViewLocation();
    uniformEyePosition = shader.GetEyePositionLocation();
    uniformColor = shader.getColorLocation();
    uniformTextureOffset = shader.getOffsetLocation();
    uniformSpecularIntensity = shader.GetSpecularIntensityLocation();
    uniformShininess = shader.GetShininessLocation();

    glm::mat4 model(1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    glm::vec2 textureOffset(0.0f, 0.0f);
    glm::mat4 modelaux1(1.0);

    glm::vec3 nickPos = glm::vec3(90.0f, 12.0f, 0.0f);

    glm::vec3 nickDireccion = glm::vec3(0.0f, 0.0f, -1.0f);

    float nickRotY = 180.0f;

    float velocidadNick = 90.0f;

    float correccionNick = 180.0f;

    int camaraActiva = 1;

    bool teclaCamaraPresionada = false;

    lastTime = glfwGetTime();

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();

        GLfloat dtNick = now - lastTime;

        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();

        if (camaraActiva == 0)
        {
            camera.keyControl(mainWindow.getsKeys(), deltaTime);
            camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
        }

        bool* keys = mainWindow.getsKeys();

        if (keys[GLFW_KEY_C] && !teclaCamaraPresionada)
        {
            camaraActiva++;

            if (camaraActiva > 1)
            {
                camaraActiva = 0;
            }

            teclaCamaraPresionada = true;
        }

        if (!keys[GLFW_KEY_C])
        {
            teclaCamaraPresionada = false;
        }

        glm::vec3 movimientoNick = glm::vec3(0.0f, 0.0f, 0.0f);

        if (keys[GLFW_KEY_UP])
        {
            movimientoNick.z -= 1.0f;
        }

        if (keys[GLFW_KEY_DOWN])
        {
            movimientoNick.z += 1.0f;
        }

        if (keys[GLFW_KEY_LEFT])
        {
            movimientoNick.x -= 1.0f;
        }

        if (keys[GLFW_KEY_RIGHT])
        {
            movimientoNick.x += 1.0f;
        }

        bool nickCaminando = glm::length(movimientoNick) > 0.0f;

        if (nickCaminando)
        {
            movimientoNick = glm::normalize(movimientoNick);

            nickPos += movimientoNick * velocidadNick * dtNick;

            nickDireccion = movimientoNick;

            nickRotY = std::atan2(nickDireccion.x, nickDireccion.z) / toRadians + correccionNick;
        }

        nickPos.x = glm::clamp(nickPos.x, -500.0f, 500.0f);
        nickPos.z = glm::clamp(nickPos.z, -500.0f, 500.0f);

        glm::vec3 cameraFollowPos = nickPos - nickDireccion * 120.0f + glm::vec3(0.0f, 65.0f, 0.0f);

        glm::vec3 cameraTarget = nickPos + glm::vec3(0.0f, 25.0f, 0.0f);

        glm::mat4 viewNick = glm::lookAt(
            cameraFollowPos,
            cameraTarget,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 viewActiva;
        glm::vec3 eyeActiva;

        if (camaraActiva == 0)
        {

            viewActiva = camera.calculateViewMatrix();
            eyeActiva = camera.getCameraPosition();
        }
        else
        {

            viewActiva = viewNick;
            eyeActiva = cameraFollowPos;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(viewActiva, projection);

        shader.UseShader();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewActiva));

        glUniform3f(uniformEyePosition,
            eyeActiva.x,
            eyeActiva.y,
            eyeActiva.z
        );

        shader.SetDirectionalLight(&mainLight);

        shader.SetPointLights(pointLights, pointLightCount);
        shader.SetSpotLights(spotLights, spotLightCount);

        textureOffset = glm::vec2(0.0f, 0.0f);
        color = glm::vec3(1.0f, 1.0f, 1.0f);

        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));

        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(70.0f, 1.0f, 70.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        pisoTexture.UseTexture();
        materialOpaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

        pisoIzquierdoMesh.RenderMesh();

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(70.0f, 1.0f, 70.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        pisoTexture2.UseTexture();
        materialOpaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

        pisoDerechoMesh.RenderMesh();

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Rotonda.RenderModel();

        float yLampara = 1.0f;
        float escalaLampara = 12.0f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-95.0f, yLampara, 260.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-95.0f, yLampara, 140.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-95.0f, yLampara, 20.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-95.0f, yLampara, -100.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-95.0f, yLampara, -220.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(95.0f, yLampara, 260.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(95.0f, yLampara, 140.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(95.0f, yLampara, 20.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(95.0f, yLampara, -100.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(95.0f, yLampara, -220.0f));
        model = glm::scale(model, glm::vec3(escalaLampara, escalaLampara, escalaLampara));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Lampara_calle.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-200.0f, 1.0f, -15.0f));
        model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Engrane.RenderModel();

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(70.0f, 70.0f, 70.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Quijote.RenderModel();

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-90.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Grave.RenderModel();

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-100.0f, 40.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Espada.RenderModel();

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-50.0f, 0.0f, 30.0f));
        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Artorias.RenderModel();

        float borde = 550.0f;
        float alturaVia = 1.2f;
        float escalaVia = 2.5f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(borde, alturaVia, -borde));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.5f, 2.5f, 2.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Giro.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(borde, alturaVia, borde));
        model = glm::rotate(model, 360.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Giro.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-borde, alturaVia, borde));
        model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Giro.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-borde, alturaVia, -borde));
        model = glm::rotate(model, -180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Giro.RenderModel();

        model = glm::mat4(1.0f);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(559.5f, alturaVia, 386.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(559.5f, alturaVia, 120.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(559.5f, alturaVia, -146.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(559.5f, alturaVia, -412.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-559.5, alturaVia, -386.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-559.5, alturaVia, -120.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-559.5, alturaVia, 146.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-559.5, alturaVia, 412.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-386.0f, alturaVia, 559.5f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-120.0f, alturaVia, 559.5f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(146.0f, alturaVia, 559.5f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(412.0f, alturaVia, 559.5f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(386.0f, alturaVia, -559.5f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(120.0f, alturaVia, -559.5f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-146.0f, alturaVia, -559.5f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-412.0f, alturaVia, -559.5f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaVia, escalaVia, escalaVia));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Via_Recta.RenderModel();

        static const glm::vec2 rutaTren[] = {

            glm::vec2(559.5f,  386.0f),
            glm::vec2(559.5f,  120.0f),
            glm::vec2(559.5f, -146.0f),
            glm::vec2(559.5f, -412.0f),

            glm::vec2(555.0f, -455.0f),
            glm::vec2(535.0f, -500.0f),
            glm::vec2(500.0f, -535.0f),
            glm::vec2(455.0f, -555.0f),
            glm::vec2(386.0f, -559.5f),

            glm::vec2(120.0f,  -559.5f),
            glm::vec2(-146.0f, -559.5f),
            glm::vec2(-412.0f, -559.5f),

            glm::vec2(-455.0f, -555.0f),
            glm::vec2(-500.0f, -535.0f),
            glm::vec2(-535.0f, -500.0f),
            glm::vec2(-555.0f, -455.0f),
            glm::vec2(-559.5f, -386.0f),

            glm::vec2(-559.5f, -120.0f),
            glm::vec2(-559.5f,  146.0f),
            glm::vec2(-559.5f,  412.0f),

            glm::vec2(-555.0f, 455.0f),
            glm::vec2(-535.0f, 500.0f),
            glm::vec2(-500.0f, 535.0f),
            glm::vec2(-455.0f, 555.0f),
            glm::vec2(-386.0f, 559.5f),

            glm::vec2(-120.0f, 559.5f),
            glm::vec2(146.0f,  559.5f),
            glm::vec2(412.0f,  559.5f),

            glm::vec2(455.0f, 555.0f),
            glm::vec2(500.0f, 535.0f),
            glm::vec2(535.0f, 500.0f),
            glm::vec2(555.0f, 455.0f)
        };

        int totalPuntos = sizeof(rutaTren) / sizeof(rutaTren[0]);

        float velocidadTren = 0.45f;

        float tiempoTren = glfwGetTime() * velocidadTren;

        int tramoActual = (int)std::floor(std::fmod(tiempoTren, (float)totalPuntos));
        int tramoSiguiente = (tramoActual + 1) % totalPuntos;

        float avance = tiempoTren - std::floor(tiempoTren);

        glm::vec2 puntoA = rutaTren[tramoActual];
        glm::vec2 puntoB = rutaTren[tramoSiguiente];

        glm::vec2 posicionTren = puntoA + (puntoB - puntoA) * avance;

        glm::vec2 direccionTren = puntoB - puntoA;

        float anguloTren = std::atan2(direccionTren.x, direccionTren.y) / toRadians;

        float correccionTren = 90.0f;

        float yTren = alturaVia + 8.0f;
        float escalaTren = 8.0f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(posicionTren.x, yTren, posicionTren.y));
        model = glm::rotate(model, (anguloTren + correccionTren) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaTren, escalaTren, escalaTren));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Tren.RenderModel();

        float yLibro = 2.0f;
        float escalaLibro = 5.0f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-30.0f, yLibro, 0.0f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaLibro, escalaLibro, escalaLibro));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Libro.RenderModel();

        float yEdificio = 0.0f;
        float yArbol = 0.0f;

        float escalaCasa = 8.5f;
        float escalaArbol = 5.5f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-330.0f, yEdificio, 290.0f));
        model = glm::rotate(model, 0.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Edificio_Casa.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-170.0f, yEdificio, 210.0f));
        model = glm::rotate(model, 0.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Edificio_Casa.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(170.0f, yEdificio, 210.0f));
        model = glm::rotate(model, 0.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Edificio_Casa.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(330.0f, yEdificio, 290.0f));
        model = glm::rotate(model, 0.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Edificio_Casa.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-260.0f, yEdificio, -300.0f));
        model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Edificio_Casa.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(260.0f, yEdificio, -300.0f));
        model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasa, escalaCasa, escalaCasa));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Edificio_Casa.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-370.0f, yEdificio, 80.0f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Medieval_House.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(370.0f, yEdificio, 80.0f));
        model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Medieval_House.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-170.0f, yEdificio, -210.0f));
        model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Medieval_House.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(170.0f, yEdificio, -210.0f));
        model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Medieval_House.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-145.0f, yArbol, 330.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-145.0f, yArbol, 230.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-145.0f, yArbol, 130.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-145.0f, yArbol, 30.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-145.0f, yArbol, -70.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-145.0f, yArbol, -170.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(145.0f, yArbol, 330.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(145.0f, yArbol, 230.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(145.0f, yArbol, 130.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(145.0f, yArbol, 30.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(145.0f, yArbol, -70.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(145.0f, yArbol, -170.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-430.0f, yArbol, 330.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(430.0f, yArbol, 330.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-430.0f, yArbol, -120.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(430.0f, yArbol, -120.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        float yCasaTunel = 0.0f;
        float escalaCasaTunel = 1.0f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(535.0f, yCasaTunel, 0.0f));
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasaTunel, escalaCasaTunel, escalaCasaTunel));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Medieval_House.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-535.0f, yCasaTunel, 0.0f));
        model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasaTunel, escalaCasaTunel, escalaCasaTunel));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Medieval_House.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, yCasaTunel, -535.0f));
        model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasaTunel, escalaCasaTunel, escalaCasaTunel));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Medieval_House.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, yCasaTunel, 535.0f));
        model = glm::rotate(model, 360.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaCasaTunel, escalaCasaTunel, escalaCasaTunel));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Medieval_House.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-370.0f, yArbol, -230.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-170.0f, yArbol, -230.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, yArbol, -230.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(170.0f, yArbol, -230.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(370.0f, yArbol, -230.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-450.0f, yArbol, 70.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(450.0f, yArbol, 70.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-360.0f, yArbol, 360.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-160.0f, yArbol, 360.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(160.0f, yArbol, 360.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(360.0f, yArbol, 360.0f));
        model = glm::scale(model, glm::vec3(escalaArbol, escalaArbol, escalaArbol));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Arbol.RenderModel();

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(50.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Gravestone.RenderModel();

        float tiempoDirigible = glfwGetTime() * 0.25f;

        float radioXDirigible = 620.0f;
        float radioZDirigible = 620.0f;

        float alturaDirigible = 260.0f;

        float movimientoVertical = sin(glfwGetTime() * 1.2f) * 15.0f;

        float xDirigible = cos(tiempoDirigible) * radioXDirigible;
        float zDirigible = sin(tiempoDirigible) * radioZDirigible;
        float yDirigible = alturaDirigible + movimientoVertical;

        float tiempoAdelante = tiempoDirigible + 0.05f;

        float xAdelante = cos(tiempoAdelante) * radioXDirigible;
        float zAdelante = sin(tiempoAdelante) * radioZDirigible;

        float dirX = xAdelante - xDirigible;
        float dirZ = zAdelante - zDirigible;

        float anguloDirigible = atan2(dirX, dirZ) / toRadians;

        float correccionDirigible = 0.0f;

        float escalaDirigible = 10.0f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xDirigible, yDirigible, zDirigible));
        model = glm::rotate(model, (anguloDirigible + correccionDirigible) * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaDirigible, escalaDirigible, escalaDirigible));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Dirigible.RenderModel();;

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(250.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Golem.RenderModel();

        float escalaNick = 20.0f;

        float animNick = 0.0f;

        if (nickCaminando)
        {
            animNick = sin(glfwGetTime() * 10.0f);
        }

        float pasoPierna = animNick * 0.10f;
        float pasoBrazo = animNick * 0.07f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, nickPos);
        model = glm::rotate(model, nickRotY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escalaNick, escalaNick, escalaNick));

        modelaux1 = model;

        model = modelaux1;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nick_Camisa.RenderModel();

        model = modelaux1;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nick_Cabeza.RenderModel();

        model = modelaux1;
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, pasoBrazo));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nick_Brazo_Izq.RenderModel();

        model = modelaux1;
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -pasoBrazo));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nick_Brazo_Der.RenderModel();

        model = modelaux1;
        model = glm::translate(model, glm::vec3(-0.06f, -0.38f, -pasoPierna));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nick_Pierna_Izq.RenderModel();

        model = modelaux1;
        model = glm::translate(model, glm::vec3(0.06f, -0.38f, pasoPierna));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nick_Pierna_Der.RenderModel();

        model = modelaux1;
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nick_Cola.RenderModel();

        model = modelaux1;
        model = glm::translate(model, glm::vec3(0.0f, -0.2f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Nick_Pantalon.RenderModel();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
