#define STB_IMAGE_IMPLEMENTATION

#include <cmath>
#include <vector>

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
#include "Model.h"
#include "Skybox.h"
#include "Sphere.h" // (Nuevo)

#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

// Ventana, cámara y listas
Window mainWindow;
Camera camera;

std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

// Texturas 
Texture pisoTexture;
Texture numerosTexture;
Texture fuegoTexture;
Texture numero_01_Texture;
Texture numero_02_Texture;
Texture humoTexture;

// Modelos 
Model Aeolipile_base_M;
Model Aeolipile_M;

Skybox skybox;

// Materiales
Material Material_brillante;
Material Material_opaco;

// Luces
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Shaders
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

//Esfera (Nuevo)
//Sphere(radio = 1, 20 sectores, 20 stacks)
Sphere sp = Sphere(1.0, 20, 20);

// Tiempo
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0f;

// Variables para animación
float toffsetnumerou = 0.0f;
float toffsetnumerov = 0.0f;
float toffsetnumerocambiau = 0.0f;
float ultimoCambioNumero = 0.0f; // (Nuevo)
float anguloAeolipile = 0.0f; // (Nuevo)
float toffsetHumoV = 0.0f; // (Nuevo)
float velocidadAeolipile = 0.0f; // (Nuevo)
float tiempoInicioHumo = 0.0f; // (Nuevo)
float tiempoParaMostrarHumo = 3.0f; // segundos que esperará antes de aparecer (Nuevo)
bool humoActivo = false; // (Nuevo)
bool estabaEncendidoAntes = false; // (Nuevo)
float tiempoApagadoFuego = 0.0f; // (Nuevo)
float tiempoParaQuitarHumo = 2.0f; // segundos después de apagar el fuego (Nuevo)

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

void CreateObjects()
{
    unsigned int indices[] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices[] = {
        //	x      y      z			u	  v			nx	  ny    nz
            -1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
    };

    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
    };
    unsigned int vegetacionIndices[] = {
       0, 1, 2,
       0, 2, 3,
       4,5,6,
       4,6,7
    };

    GLfloat vegetacionVertices[] = {
        -0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

        0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


    };


    unsigned int flechaIndices[] = {
       0, 1, 2,
       0, 2, 3,
    };

    GLfloat flechaVertices[] = {
        -0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
        0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
        0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

    };

    unsigned int scoreIndices[] = {
       0, 1, 2,
       0, 2, 3,
    };

    GLfloat scoreVertices[] = {
        -0.5f, 0.0f, 0.5f,		0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
        0.5f, 0.0f, 0.5f,		1.0f, 0.0f,		0.0f, -1.0f, 0.0f,
        0.5f, 0.0f, -0.5f,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

    };

    unsigned int numeroIndices[] = {
       0, 1, 2,
       0, 2, 3,
    };

    GLfloat numeroVertices[] = {
        -0.5f, 0.0f, 0.5f,		0.0f, 0.67f,		0.0f, -1.0f, 0.0f,
        0.5f, 0.0f, 0.5f,		0.25f, 0.67f,		0.0f, -1.0f, 0.0f,
        0.5f, 0.0f, -0.5f,		0.25f, 1.0f,		0.0f, -1.0f, 0.0f,
        -0.5f, 0.0f, -0.5f,		0.0f, 1.0f,		0.0f, -1.0f, 0.0f,

    };
    // (Nuevo)
    unsigned int cuboIndices[] = {
        // Frente
        0, 1, 2,
        2, 3, 0,

        // Derecha
        4, 5, 6,
        6, 7, 4,

        // Atrás
        8, 9, 10,
        10, 11, 8,

        // Izquierda
        12, 13, 14,
        14, 15, 12,

        // Arriba
        16, 17, 18,
        18, 19, 16,

        // Abajo
        20, 21, 22,
        22, 23, 20
    };

    GLfloat cuboVertices[] = {
        // x      y      z       u     v      nx     ny     nz

        // Frente, normal hacia +Z
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        // Derecha, normal hacia +X
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f, 1.0f,   1.0f, 0.0f, 0.0f,

         // Atrás, normal hacia -Z
          0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
         -0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
         -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
          0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   0.0f, 0.0f, -1.0f,

          // Izquierda, normal hacia -X
          -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
          -0.5f, -0.5f,  0.5f,   1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
          -0.5f,  0.5f,  0.5f,   1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
          -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,

          // Arriba, normal hacia +Y
          -0.5f,  0.5f,  0.5f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
           0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
           0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
          -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,

          // Abajo, normal hacia -Y
          -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
           0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
           0.5f, -0.5f,  0.5f,   1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
          -0.5f, -0.5f,  0.5f,   0.0f, 1.0f,   0.0f, -1.0f, 0.0f
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

    Mesh* obj4 = new Mesh();
    obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
    meshList.push_back(obj4);

    Mesh* obj5 = new Mesh();
    obj5->CreateMesh(flechaVertices, flechaIndices, 32, 6);
    meshList.push_back(obj5);

    Mesh* obj6 = new Mesh();
    obj6->CreateMesh(scoreVertices, scoreIndices, 32, 6);
    meshList.push_back(obj6); // todos los números

    Mesh* obj7 = new Mesh();
    obj7->CreateMesh(numeroVertices, numeroIndices, 32, 6);
    meshList.push_back(obj7); // solo un número

    Mesh* obj8 = new Mesh();
    obj8->CreateMesh(cuboVertices, cuboIndices, 192, 36);
    meshList.push_back(obj8); // cubo 

}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}


int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f,
        0.0f,
        0.5f,
        0.5f
    );

    pisoTexture = Texture("Textures/piso.tga");
    pisoTexture.LoadTextureA();

    numerosTexture = Texture("Textures/numerosbase.tga");
    numerosTexture.LoadTextureA();

    numero_01_Texture = Texture("Textures/numero1.tga");
    numero_01_Texture.LoadTextureA();

    numero_02_Texture = Texture("Textures/numero2.tga");
    numero_02_Texture.LoadTextureA();

    fuegoTexture = Texture("Textures/fuego.tga");
    fuegoTexture.LoadTextureA();

    humoTexture = Texture("Textures/humo.tga");
    humoTexture.LoadTextureA();

    Aeolipile_base_M = Model();
    Aeolipile_base_M.LoadModel("Models/Aeolipile_base.obj");

    Aeolipile_M = Model();
    Aeolipile_M.LoadModel("Models/Aeolipile.obj");

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

    // Luz direccional que siempre debe de existir
    mainLight = DirectionalLight(
        1.0f, 1.0f, 1.0f,
        0.5f, 0.5f,
        0.0f, -1.0f, -1.0f
    );

    // Luz puntual del fuego
    pointLights[0] = PointLight(
        1.0f, 0.45f, 0.05f,
        0.0f, 1.8f,
        20.0f, 0.5f, 1.5f,
        0.3f, 0.2f, 0.1f
    );

    unsigned int pointLightCount = 0;
    unsigned int spotLightCount = 0;

    // Se crean mas luces puntuales y spotlight 
    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformEyePosition = 0;
    GLuint uniformSpecularIntensity = 0;
    GLuint uniformShininess = 0;
    GLuint uniformTextureOffset = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(
        45.0f,
        (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f,
        1000.0f
    );

    shaderList[0].UseShader();

    uniformModel = shaderList[0].GetModelLocation();
    uniformProjection = shaderList[0].GetProjectionLocation();
    uniformView = shaderList[0].GetViewLocation();
    uniformEyePosition = shaderList[0].GetEyePositionLocation();
    uniformColor = shaderList[0].getColorLocation();
    uniformTextureOffset = shaderList[0].getOffsetLocation();
    uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
    uniformShininess = shaderList[0].GetShininessLocation();

    // Inicialización de la esfera
    sp.init();
    sp.load();

    glm::mat4 model(1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    glm::vec3 lowerLight(0.0f, 0.0f, 0.0f);
    glm::vec2 textureOffset(0.0f, 0.0f);
    glm::mat4 modelaux(1.0);
    glm::mat4 modelaux2(1.0);

    lastTime = glfwGetTime();

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        /*Para girar la parte circular del Aeolipile (Nuevo)
        anguloAeolipile += 30.0f * deltaTime; Velocidad de giro 

        if (anguloAeolipile >= 360.0f)
        {
            anguloAeolipile = 0.0f;
        }*/

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        shaderList[0].UseShader();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(
            uniformEyePosition,
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z
        );

        // Prender y apagar (Nuevo)
        if (mainWindow.getEncender())
        {
            pointLightCount = 1;
        }
        else
        {
            pointLightCount = 0;
        }

        //// Si la luz está prendida, el ángulo aumenta (Nuevo)
        //if (mainWindow.getEncender())
        //{
        //    anguloAeolipile += 30.0f * deltaTime;

        //    if (anguloAeolipile >= 360.0f)
        //    {
        //        anguloAeolipile = 0.0f;
        //    }
        //}

        // Si la luz está prendida, aumentar velocidad poco a poco (Nuevo)
        if (mainWindow.getEncender())
        {
            // Para aumentar poco a poco la velocidad
            velocidadAeolipile += 0.1f * deltaTime;

            // Límite de velocidad
            if (velocidadAeolipile > 80.0f)
            {
                velocidadAeolipile = 80.0f;
            }

            // Giro usando la velocidad actual
            anguloAeolipile += velocidadAeolipile * deltaTime;

            if (anguloAeolipile >= 360.0f)
            {
                anguloAeolipile = 0.0f;
            }
        }
        else
        {
            // Disminuye poco a poco la velocidad
            velocidadAeolipile -= 0.2f * deltaTime;

            // Evita que la velocidad se vuelva negativa
            if (velocidadAeolipile < 0.0f)
            {
                velocidadAeolipile = 0.0f;
            }

            // Mientras todavía haya velocidad, sigue girando
            anguloAeolipile += velocidadAeolipile * deltaTime;

            if (anguloAeolipile >= 360.0f)
            {
                anguloAeolipile = 0.0f;
            }
        }

        //// Tiempo para que aparezca el humo (Nuevo)
        //if (mainWindow.getEncender() && !estabaEncendidoAntes)
        //{
        //    tiempoInicioHumo = now;
        //    humoActivo = false;
        //}

        //if (mainWindow.getEncender())
        //{
        //    if (now - tiempoInicioHumo >= tiempoParaMostrarHumo)
        //    {
        //        humoActivo = true;
        //    }
        //}
        //else
        //{
        //    humoActivo = false;
        //}

        //estabaEncendidoAntes = mainWindow.getEncender();

        // Tiempo para que aparezca y desaparezca el humo
        if (mainWindow.getEncender() && !estabaEncendidoAntes)
        {
            tiempoInicioHumo = now;
            humoActivo = false;
        }

        if (!mainWindow.getEncender() && estabaEncendidoAntes)
        {
            tiempoApagadoFuego = now;
        }

        if (mainWindow.getEncender())
        {
            if (now - tiempoInicioHumo >= tiempoParaMostrarHumo)
            {
                humoActivo = true;
            }
        }
        else
        {
            if (now - tiempoApagadoFuego >= tiempoParaQuitarHumo)
            {
                humoActivo = false;
            }
        }

        estabaEncendidoAntes = mainWindow.getEncender();

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, pointLightCount);
        shaderList[0].SetSpotLights(spotLights, spotLightCount);

        // Reiniciando variables cada ciclo de reloj
        model = glm::mat4(1.0);
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        textureOffset = glm::vec2(0.0f, 0.0f);
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));

        // Piso
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));

        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[2]->RenderMesh();

        // Base del Aeolipile
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(20.0f, 0.0f, 1.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Aeolipile_base_M.RenderModel();

        // Parte giratoria del Aeolipile
        model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
        model = glm::rotate(model, anguloAeolipile * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));  // Giro (Nuevo)
        model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        modelaux = model; // (Nuevo)
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Aeolipile_M.RenderModel();

        // Brazo (Nuevo)
        model = modelaux;
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.5f));
        modelaux2 = model;
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 3.0f));
        color = glm::vec3(0.0f, 1.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        meshList[7]->RenderMesh();

        // Esfera (Nuevo)
        model = modelaux2;
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        sp.render();

        // Humo en los extremos que aparecen después de cierto tiempo
        if (humoActivo) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            humoTexture.UseTexture();
            Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

            color = glm::vec3(1.0f, 1.0f, 1.0f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));

            textureOffset = glm::vec2(0.0f, toffsetHumoV);
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));

            // Humo extremo 1
            model = modelaux;
            model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            meshList[3]->RenderMesh();

            // Humo extremo 2
            model = modelaux;
            model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            meshList[3]->RenderMesh();

            glDisable(GL_BLEND);
        }
        else
        {
            humoActivo = false;
        }


        // Plano con todos los números
        toffsetnumerou = 0.0;
        toffsetnumerov = 0.0;
        textureOffset = glm::vec2(toffsetnumerou, toffsetnumerov);

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-6.0f, 2.0f, -6.0f));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        numerosTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[5]->RenderMesh();

        // Número 1
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-10.0f, 2.0f, -6.0f));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        numerosTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[6]->RenderMesh();

        for (int i = 1; i < 4; i++)
        {
            // Números 2-4
            toffsetnumerou += 0.25;
            toffsetnumerov = 0.0;
            textureOffset = glm::vec2(toffsetnumerou, toffsetnumerov);
            model = glm::mat4(1.0);
            model = glm::translate(model, glm::vec3(-10.0f - (i * 3.0), 2.0f, -6.0f));
            model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            color = glm::vec3(1.0f, 1.0f, 1.0f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            numerosTexture.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[6]->RenderMesh();

        }

        for (int j = 1; j < 5; j++)
        {
            // Números 5-8
            toffsetnumerou += 0.25;
            toffsetnumerov = -0.33;
            textureOffset = glm::vec2(toffsetnumerou, toffsetnumerov);
            model = glm::mat4(1.0);
            model = glm::translate(model, glm::vec3(-7.0f - (j * 3.0), 5.0f, -6.0f));
            model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
            glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            color = glm::vec3(1.0f, 1.0f, 1.0f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            numerosTexture.UseTexture();
            Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
            meshList[6]->RenderMesh();
        }

        /*Ejercicio 01: Agregar que el número cambiante sea a una velocidad visible*/
        // Número cambiante a velocidad visible

        if (now - ultimoCambioNumero >= 0.5f)
        {
            toffsetnumerocambiau += 0.25f;

            if (toffsetnumerocambiau > 0.75f)
            {
                toffsetnumerocambiau = 0.0f;
            }

            ultimoCambioNumero = now;
        }

        toffsetnumerov = 0.0;
        textureOffset = glm::vec2(toffsetnumerocambiau, toffsetnumerov);

        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-10.0f, 10.0f, -6.0f));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        numerosTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[6]->RenderMesh();

        // Cambiar automáticamente entre textura número 1 y número 2
        toffsetnumerou = 0.0;
        toffsetnumerov = 0.0;
        textureOffset = glm::vec2(toffsetnumerou, toffsetnumerov);
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(-13.0f, 10.0f, -6.0f));
        model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        numero_01_Texture.UseTexture();

        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[5]->RenderMesh();

        // Fuego en la base del Aeolipile
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(20.0f, 0.6f, 1.5f));
        model = glm::scale(model, glm::vec3(2.0f, 2.5f, 2.0f));

        textureOffset = glm::vec2(0.0f, 0.0f);

        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(textureOffset));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // Para la transparencia del fuego
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        fuegoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshList[3]->RenderMesh();

        glDisable(GL_BLEND);
        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
