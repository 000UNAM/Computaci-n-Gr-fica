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

/// Constantes matemáticas usadas en rotaciones y generación de geometría.
///
//// Uso:
/// - `toRadians`: factor de conversión de grados a radianes.
/// - `PI`: constante pi para cálculos trigonométricos.
///
/// Nota:
/// - En este archivo `toRadians` realmente no se usa directamente,
///   porque varias rotaciones se hacen con `glm::radians(...)`.
const float toRadians = 3.14159265f / 180.0f;
const float PI = 3.14159265f;

/// Variables globales para control de tiempo entre frames.
///
/// Propósito:
/// - `deltaTime`: tiempo transcurrido entre un frame y el siguiente.
/// - `lastTime`: tiempo del frame anterior.
/// - `limitFPS`: valor base usado para ajustar el tiempo y suavizar el movimiento.
///
/// Esto se usa para:
/// - Mover la cámara de forma independiente del FPS.
/// - Hacer que el movimiento no dependa de qué tan rápido renderice la PC.
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

/// Objetos globales principales del programa.
///
/// - `camera`: controla posición, orientación y movimiento de la cámara.
/// - `mainWindow`: encapsula la ventana, buffer, teclado, mouse, etc.
Camera camera;
Window mainWindow;

/// Listas globales de recursos gráficos.
///
/// - `meshList`: almacena apuntadores a las mallas creadas.
/// - `shaderList`: almacena los shaders cargados.
///
/// Idea:
/// - Cada función `Crear...()` construye una geometría y la mete en `meshList`.
vector<Mesh*> meshList;
vector<Shader> shaderList;

/// Rutas de los shaders de vértice y fragmento.
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";

/// Esfera global.
///
//// Parámetros:
/// - radio = 1.0
/// - 20 sectores
/// - 20 stacks
///
/// Aunque en el render activo no se usa, se deja preparada porque en el bloque
/// comentado sí formaba parte del modelo articulado.
Sphere sp = Sphere(1.0, 20, 20);

/// Crear un cubo a partir de vértices e índices.
///
/// Geometría:
/// - 8 vértices (esquinas del cubo).
/// - 12 triángulos.
/// - 36 índices en total.
///
/// Estructura:
/// - `cubo_vertices` guarda solo posiciones XYZ.
/// - `cubo_indices` define cómo unir esos puntos para formar las caras.
///
/// Flujo:
/// 1. Se declaran los vértices.
/// 2. Se declaran los índices.
/// 3. Se crea un objeto `Mesh`.
/// 4. Se manda la información a GPU con `CreateMesh`.
/// 5. Se guarda en `meshList`.
void CrearCubo()
{
    unsigned int cubo_indices[] =
    {
        0, 1, 2,  2, 3, 0,
        1, 5, 6,  6, 2, 1,
        7, 6, 5,  5, 4, 7,
        4, 0, 3,  3, 7, 4,
        4, 5, 1,  1, 0, 4,
        3, 2, 6,  6, 7, 3
    };

    GLfloat cubo_vertices[] =
    {
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    };

    Mesh* cubo = new Mesh();
    cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
    meshList.push_back(cubo);
}

/// Crear una pirámide triangular.
///
/// Geometría:
/// - 4 vértices.
/// - 4 caras triangulares.
/// - 12 índices.
///
/// Interpretación:
/// - Tres caras laterales.
/// - Una base triangular.
///
/// Uso:
/// - Es un sólido simple, útil para practicar creación manual de mallas.
void CrearPiramideTriangular()
{
    unsigned int indices[] =
    {
        0, 1, 2,
        1, 3, 2,
        3, 0, 2,
        1, 0, 3
    };

    GLfloat vertices[] =
    {
        -0.5f, -0.5f,  0.0f,
         0.5f, -0.5f,  0.0f,
         0.0f,  0.5f, -0.25f,
         0.0f, -0.5f, -0.5f
    };

    Mesh* obj = new Mesh();
    obj->CreateMesh(vertices, indices, 12, 12);
    meshList.push_back(obj);
}

/// Crear un cilindro mediante discretización angular.
///
/// Parámetros:
/// - `res`: número de divisiones alrededor del círculo.
/// - `R`: radio del cilindro.
///
/// Idea geométrica:
/// - Se generan puntos de la tapa inferior y superior a lo largo del contorno.
/// - Después se agregan puntos extra para las tapas.
/// - Los índices se generan en orden secuencial.
///
/// Notas:
/// - `dt = 2*PI/res` es el paso angular.
/// - Cada vuelta genera puntos usando coseno y seno.
/// - La altura va de `y = -0.5` a `y = 0.5`.
///
/// Observación:
/// - Aquí se usa `CreateMeshGeometry`, probablemente pensada para geometría
///   más general o para modos de dibujo distintos al cubo.
void CrearCilindro(int res, float R)
{
    int n, i;
    GLfloat dt = 2 * PI / res;
    GLfloat x, z, y = -0.5f;

    vector<GLfloat> vertices;
    vector<unsigned int> indices;

    /// Generación de la superficie lateral.
    ///
    /// Por cada división angular:
    /// - se calcula un punto abajo
    /// - se calcula su correspondiente punto arriba
    ///
    /// Eso produce pares de vértices sobre el borde del cilindro.
    for (n = 0; n <= (res); n++)
    {
        if (n != res)
        {
            x = R * cos((n) * dt);
            z = R * sin((n) * dt);
        }
        else
        {
            /// En la última iteración se repite el punto inicial
            /// para cerrar correctamente la figura.
            x = R * cos((0) * dt);
            z = R * sin((0) * dt);
        }

        for (i = 0; i < 6; i++)
        {
            switch (i)
            {
            case 0: vertices.push_back(x);    break;
            case 1: vertices.push_back(y);    break;
            case 2: vertices.push_back(z);    break;
            case 3: vertices.push_back(x);    break;
            case 4: vertices.push_back(0.5);  break;
            case 5: vertices.push_back(z);    break;
            }
        }
    }

    /// Generación de puntos de la tapa inferior.
    for (n = 0; n <= (res); n++)
    {
        x = R * cos((n) * dt);
        z = R * sin((n) * dt);

        for (i = 0; i < 3; i++) {
            switch (i)
            {
                case 0: vertices.push_back(x);      break;
                case 1: vertices.push_back(-0.5f);  break;
                case 2: vertices.push_back(z);      break;
            }
        }
    }

    /// Generación de puntos de la tapa superior.
    for (n = 0; n <= (res); n++)
    {
        x = R * cos((n) * dt);
        z = R * sin((n) * dt);

        for (i = 0; i < 3; i++) {
            switch (i)
            {
                case 0: vertices.push_back(x);      break;
                case 1: vertices.push_back(0.5f);   break;
                case 2: vertices.push_back(z);      break;
            }
        }
    }

    /// Los índices se agregan de forma secuencial:
    /// 0, 1, 2, 3, ..., N-1
    ///
    /// Eso implica que el modo de dibujo dentro de `CreateMeshGeometry`
    /// determina cómo se conectan esos vértices.
    for (i = 0; i < vertices.size(); i++)
    {
        indices.push_back(i);
    }

    Mesh* cilindro = new Mesh();
    cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
    meshList.push_back(cilindro);
}

/// Crear un cono mediante discretización angular.
///
/// Parámetros:
/// - `res`: resolución angular.
/// - `R`: radio de la base.
///
/// Idea:
/// - Primero se agrega el vértice superior (punta).
/// - Luego se agregan los vértices del contorno de la base.
/// - Al final se repite un punto para cerrar la figura.
///
/// Notas:
/// - La punta está en `(0, 0.5, 0)`.
/// - La base está centrada en `y = -0.5`.
void CrearCono(int res, float R)
{
    int n, i;
    GLfloat dt = 2 * PI / res;
    GLfloat x, z, y = -0.5f;

    vector<GLfloat> vertices;
    vector<unsigned int> indices;

    /// Punta del cono.
    vertices.push_back(0.0);
    vertices.push_back(0.5);
    vertices.push_back(0.0);

    /// Contorno de la base.
    for (n = 0; n <= res; n++)
    {
        x = R * cos(n * dt);
        z = R * sin(n * dt);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }

    /// Punto extra para cerrar la figura.
    ///
    /// Ojo:
    /// aquí en tu código original aparece:
    /// `R * cos(0) * dt` y `R * sin(0) * dt`
    ///
    /// Eso hace que el punto no sea exactamente igual al primero del borde,
    /// porque lo multiplicas por `dt`.
    /// Lo dejé igual para no alterar la lógica original,
    /// pero geométricamente sería más natural usar:
    /// `R * cos(0)` y `R * sin(0)`.
    vertices.push_back(R * cos(0) * dt);
    vertices.push_back(-0.5);
    vertices.push_back(R * sin(0) * dt);

    for (i = 0; i < res + 2; i++)
    {
        indices.push_back(i);
    }

    Mesh* cono = new Mesh();
    cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
    meshList.push_back(cono);
}

/// Crear una pirámide cuadrangular.
///
/// Geometría:
/// - 5 vértices:
///   * 4 para la base cuadrada
///   * 1 para la punta
///
/// - 6 triángulos definidos por índices.
///
/// Observación:
/// - Aquí parece que una de las caras de la base o una cara lateral
///   puede estar mal planteada dependiendo de cómo interprete `CreateMeshGeometry`.
/// - Aun así, mantengo tu código original, solo ordenado y comentado.
void CrearPiramideCuadrangular()
{
    vector<unsigned int> indices =
    {
        0, 3, 4,
        3, 2, 4,
        2, 1, 4,
        1, 0, 4,
        0, 1, 2,
        0, 2, 4
    };

    vector<GLfloat> vertices =
    {
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.0f,  0.5f,  0.0f
    };

    Mesh* piramide = new Mesh();
    piramide->CreateMeshGeometry(vertices, indices, 15, 18);
    meshList.push_back(piramide);
}

/// Crear y compilar los shaders del programa.
///
/// Flujo:
/// 1. Se crea un objeto `Shader`.
/// 2. Se cargan los archivos `.vert` y `.frag`.
/// 3. Se guarda el shader en `shaderList`.
///
/// Nota:
/// - Solo se crea un shader.
/// - Luego se usa como `shaderList[0]`.
void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

/// Función principal del programa.
///
/// Responsabilidades:
/// 1. Crear e inicializar la ventana.
/// 2. Crear las mallas.
/// 3. Cargar shaders.
/// 4. Configurar cámara.
/// 5. Construir la matriz de proyección.
/// 6. Entrar al ciclo principal de renderizado.
/// 7. Dibujar la geometría seleccionada.
int main()
{
    /// Crear ventana de 800x800 e inicializar contexto OpenGL.
    mainWindow = Window(800, 800);
    mainWindow.Initialise();

    /// Crear geometrías y guardarlas en `meshList`.
    ///
    /// Índices finales:
    /// - 0: cubo
    /// - 1: pirámide triangular
    /// - 2: cilindro
    /// - 3: cono
    /// - 4: pirámide cuadrangular
    CrearCubo();
    CrearPiramideTriangular();
    CrearCilindro(25, 1.0f);
    CrearCono(25, 2.0f);
    CrearPiramideCuadrangular();

    /// Crear y compilar shaders.
    CreateShaders();

    /// Configurar cámara.
    ///
    /// Parámetros:
    /// - posición inicial: (0, 0, 0)
    /// - vector up: (0, 1, 0)
    /// - yaw: -60 grados
    /// - pitch: 0 grados
    /// - velocidad de movimiento: 0.2
    /// - sensibilidad del mouse: 0.2
    camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f,
        0.0f,
        0.2f,
        0.2f
    );

    /// Ubicaciones de uniformes en el shader.
    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformColor = 0;

    /// Matriz de proyección en perspectiva.
    ///
    /// Parámetros:
    /// - FOV: 60 grados
    /// - aspect ratio: ancho / alto de la ventana
    /// - near plane: 0.1
    /// - far plane: 100.0
    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        mainWindow.getBufferWidth() / mainWindow.getBufferHeight(),
        0.1f,
        100.0f
    );

    /// Inicialización de la esfera.
    ///
    /// Aunque no se usa en el render activo, se deja cargada
    /// porque el bloque comentado sí la utiliza.
    sp.init();
    sp.load();

    //--------------------------------------------------------------------
    /// Matrices auxiliares de transformación.
    ///
    /// Uso típico:
    /// - `model`: matriz actual del objeto a dibujar.
    /// - `modelaux` y `modelaux2`: sirven para guardar estados intermedios
    ///   al construir jerarquías de transformaciones.
    ///
    /// Esto es útil en modelos articulados.
    glm::mat4 model(1.0);
    glm::mat4 modelaux(1.0);
    glm::mat4 modelaux2(1.0);
    //--------------------------------------------------------------------

    /// Color base del objeto.
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

    /// Ciclo principal del programa.
    ///
    /// Se ejecuta mientras la ventana no se cierre.
    while (!mainWindow.getShouldClose())
    {
        /// Calcular tiempo entre frames.
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        /// Procesar eventos de teclado y mouse.
        glfwPollEvents();

        /// Actualizar cámara según entrada del usuario.
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        /// Limpiar pantalla y buffer de profundidad.
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// Activar shader.
        shaderList[0].useShader();

        /// Obtener ubicaciones de uniformes.
        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();
        //--------------------------------------------------------------------
        /// Construcción de un modelo jerárquico articulado (brazo mecánico)
        ///
        /// Objetivo:
        /// Crear un conjunto de piezas conectadas jerárquicamente donde
        /// cada segmento depende de la transformación del anterior.
        ///
        /// Concepto fundamental:
        /// En un modelo jerárquico cada objeto hereda la transformación
        /// del objeto padre.
        ///
        /// Esto se logra acumulando transformaciones en la matriz `model`.
        ///
        /// Ejemplo conceptual:
        ///
        ///Cabina
        /// └── Segmento 1
        /// └── Base  └── Articulación
        ///       └──Rueda1    └── Segmento 2
        ///       └──Rueda2            └── Articulación
        ///       └──Rueda3                     └── Segmento 3
        ///       └──Rueda4
        ///
        /// Para lograr esto usamos:
        ///
        /// `model`
        ///     Matriz de transformación actual.
        ///
        /// `modelaux`
        ///     Copia de la matriz antes de aplicar transformaciones
        ///     adicionales para continuar la jerarquía.
        /// 
        /// `modelaux2`
        ///     Copia de la matriz antes de aplicar transformaciones
        ///     adicionales para continuanr la jerarquía. 
        //--------------------------------------------------------------------



        //--------------------------------------------------------------------
        /// Cabina
        ///
        /// Este es el primer bloque del modelo.
        /// Representa la cabina
        ///
        /// Pasos:
        /// 1. Crear matriz identidad.
        /// 2. Posicionar el brazo dentro de la escena.
        /// 3. Guardar la transformación base.
        /// 4. Escalar el cubo para formar la base.
        ///--------------------------------------------------------------------

        // Crear matriz identidad
        model = glm::mat4(1.0);

        // Trasladar el modelo a su posición en la escena
        model = glm::translate(model, glm::vec3(0.0f, 6.0f, -6.0f));

        // Guardar esta transformación base 
        modelaux = model;
        modelaux2 = model;
        // Escalar el cubo para formar la base
        model = glm::scale(model, glm::vec3(8.0f, 4.0f, 4.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // Dibujar la cabina
        meshList[0]->RenderMesh();


        //--------------------------------------------------------------------
        /// PRIMER SEGMENTO DEL BRAZO
        ///
        /// Este segmento gira respecto a la base.
        ///
        /// Pasos:
        /// 1. Recuperar transformación base.
        /// 2. Aplicar rotación controlada por el usuario.
        /// 3. Ajustar orientación.
        /// 4. Desplazar el siguiente segmento.
        ///--------------------------------------------------------------------

        model = modelaux;
        
        // Rotación controlada por la primera articulación
        model = glm::rotate(model,
            glm::radians(mainWindow.getarticulacion1()),
            glm::vec3(0, 0, 1));

        // Rotación fija para orientar el segmento
        model = glm::rotate(model,
            glm::radians(135.0f),
            glm::vec3(0, 0, 1));

        // Trasladar el siguiente segmento
        model = glm::translate(model, glm::vec3(2.5f, 0, 0));

        // Guardar transformación para continuar la jerarquía
        modelaux = model;

        // Escalar para crear el brazo
        model = glm::scale(model, glm::vec3(5, 1, 1));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(1, 0, 1);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // Dibujar segmento
        meshList[0]->RenderMesh();


        //--------------------------------------------------------------------
        /// PRIMERA ARTICULACIÓN (ESFERA)
        ///
        /// Representa la unión entre los segmentos.
        /// Permite rotación del siguiente brazo.
        ///--------------------------------------------------------------------

        model = modelaux;

        // Mover la articulación al final del brazo
        model = glm::translate(model, glm::vec3(2.5f, 0, 0));

        // Rotación controlada por la segunda articulación
        model = glm::rotate(model,
            glm::radians(mainWindow.getarticulacion2()),
            glm::vec3(0, 0, 1));

        // Guardar transformación
        modelaux = model;

        // Escalar esfera para hacerla pequeña
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        sp.render();


        //--------------------------------------------------------------------
        /// SEGUNDO SEGMENTO DEL BRAZO
        ///
        /// Conectado a la articulación anterior.
        ///--------------------------------------------------------------------

        model = modelaux;
        
        // Desplazar hacia abajo para posicionar el segmento
        model = glm::translate(model, glm::vec3(0, -2.5f, 0));

        modelaux = model;
        
        // Escalar cubo para formar brazo largo
        model = glm::scale(model, glm::vec3(1, 5, 1));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(0, 1, 0);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();


        //--------------------------------------------------------------------
        /// SEGUNDA ARTICULACIÓN
        ///
        /// Permite rotación del siguiente segmento.
        ///--------------------------------------------------------------------

        model = modelaux;

        model = glm::translate(model, glm::vec3(0, -2.5f, 0));

        model = glm::rotate(model,
            glm::radians(mainWindow.getarticulacion3()),
            glm::vec3(0, 0, 1));

        modelaux = model;

        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        sp.render();


        //--------------------------------------------------------------------
        /// TERCER SEGMENTO DEL BRAZO
        ///
        /// Continúa la cadena jerárquica.
        ///--------------------------------------------------------------------

        model = modelaux;

        model = glm::translate(model, glm::vec3(2.5f, 0, 0));

        modelaux = model;

        model = glm::scale(model, glm::vec3(5, 1, 1));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(0, 1, 0);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[0]->RenderMesh();


        //--------------------------------------------------------------------
        /// TERCERA ARTICULACIÓN
        ///
        /// Última unión del brazo.
        ///--------------------------------------------------------------------

        model = modelaux;

        model = glm::translate(model, glm::vec3(2.5f, 0, 0));

        model = glm::rotate(model,
            glm::radians(mainWindow.getarticulacion4()),
            glm::vec3(0, 0, 1));

        modelaux = model;

        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        sp.render();


        //--------------------------------------------------------------------
        /// PIEZA FINAL DEL BRAZO
        ///
        /// Último elemento del modelo jerárquico.
        ///--------------------------------------------------------------------

        modelaux = model;

        model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));

        model = glm::scale(model, glm::vec3(3.0f, 2.5f, 2.5f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        meshList[0]->RenderMesh();

        //--------------------------------------------------------------------
        /// Base (Pirámide cuadrangular)
        ///
        /// Unida a la cabina, al brazo y articulación.
        /// Como terminé con la pieza final del brazo, necesito regrear a la
        /// cabina sin perder jerarquía, por tal motivo, se necesita un nuevo
        /// modelaux (modelaux2)
        ///--------------------------------------------------------------------
        
        model = modelaux2;

        model = glm::translate(model, glm::vec3(0, -2, 0));

        modelaux2 = model;

        model = glm::scale(model, glm::vec3(4, 2, 4));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(0, 1, 0);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[4]->RenderMesh();

        //--------------------------------------------------------------------
        /// PRIMERA RUEDA (CILINRO)
        ///
        /// Actúa como articulación.
        ///--------------------------------------------------------------------
        
        model = modelaux2;

        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = glm::translate(model, glm::vec3(-2, -1.5f, 2));

        model = glm::rotate(model, 
            glm::radians(mainWindow.getarticulacion5()), 
            glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::scale(model, glm::vec3(1, 0.5, 1));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(0, 1, 0);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[2]->RenderMesh();

        //--------------------------------------------------------------------
        /// SEGUNDA RUEDA (CILINRO)
        ///
        /// Actúa como articulación.
        ///--------------------------------------------------------------------

        model = modelaux2;

        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = glm::translate(model, glm::vec3(-2, 1.5, 2));

        model = glm::rotate(model,
            glm::radians(mainWindow.getarticulacion6()),
            glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::scale(model, glm::vec3(1, 0.5, 1));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(0, 1, 0);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[2]->RenderMesh();

        //--------------------------------------------------------------------
        /// TERCERA RUEDA (CILINRO)
        ///
        /// Actúa como articulación.
        ///--------------------------------------------------------------------

        model = modelaux2;

        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = glm::translate(model, glm::vec3(2, 1.5, 2));

        model = glm::rotate(model,
            glm::radians(mainWindow.getarticulacion7()),
            glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::scale(model, glm::vec3(1, 0.5, 1));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(0, 1, 0);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[2]->RenderMesh();

        //--------------------------------------------------------------------
        /// CUARTA RUEDA (CILINRO)
        ///
        /// Actúa como articulación.
        ///--------------------------------------------------------------------

        model = modelaux2;

        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = glm::translate(model, glm::vec3(2, -1.5, 2));

        model = glm::rotate(model,
            glm::radians(mainWindow.getarticulacion8()),
            glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::scale(model, glm::vec3(1, 0.5, 1));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        color = glm::vec3(0, 1, 0);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        meshList[2]->RenderMesh();

        /// Desactivar shader actual.
        glUseProgram(0);

        /// Intercambiar buffers para mostrar el frame en pantalla.
        mainWindow.swapBuffers();
    }

    return 0;
}