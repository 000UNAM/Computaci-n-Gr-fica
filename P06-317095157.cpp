/*
===============================================================================
PRÁCTICA 6: TEXTURIZADO EN OPENGL
===============================================================================

OBJETIVO:
- Aplicar texturas a objetos 3D.
- Integrar modelos externos (OBJ).
- Construir una escena completa con skybox.

ESTRUCTURA GENERAL:
- Inicialización de ventana y contexto OpenGL
- Creación de objetos (mallas)
- Carga de texturas
- Carga de modelos
- Render loop (ciclo principal)

PIPELINE GENERAL:
	Geometría → Texturas → Transformaciones → Renderizado

NOTA:
Este programa mezcla objetos creados manualmente y modelos importados.
===============================================================================
*/

// ==================== CARGAR IMAGEN ====================
#define STB_IMAGE_IMPLEMENTATION

// ==================== LIBRERÍAS ESTÁNDAR ====================
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

// ==================== OPENGL / GRÁFICOS ====================
#include <glew.h>
#include <glfw3.h>

// ==================== MATEMÁTICAS (GLM) ====================
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

// ==================== CLASES DEL PROYECTO ====================
#include "Window.h"
#include "Mesh.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

/// Conversión de grados a radianes (necesario para rotaciones en OpenGL)
const float toRadians = 3.14159265f / 180.0f;

/// Ventana principal
Window mainWindow;

/// Lista de mallas (objetos 3D)
std::vector<Mesh*> meshList;

/// Lista de shaders
std::vector<Shader> shaderList;

/// Cámara del usuario
Camera camera;

/// Texturas usadas en la escena
Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture dadoTexture;
Texture octaedroTexture;
Texture logofiTexture;

/// Modelos importados (.obj)
Model Kitt_M;
Model Llanta_M;
Model Dado_M;
Model Octaedro_M;
Model Jeep;
Model Llanta_01;
Model Llanta_02;
Model Llanta_03;
Model Llanta_04;
Model Cofre;


/// Skybox (fondo)
Skybox skybox;

/// Control de tiempo (FPS)
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;


// Vertex Shader
static const char* vShader = "shaders/shader_texture.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_texture.frag";




/// =============================================================================
/// CALCULAR NORMALES PROMEDIO (Phong Shading)
/// =============================================================================
///
/// ¿Qué hace?
/// - Calcula las normales de cada vértice promediando las caras adyacentes.
///
/// ¿Por qué es importante?
/// - Sin normales → iluminación incorrecta.
/// - Con normales → sombreado suave (realista).
///
/// ¿Cómo funciona?
/// 1. Recorre triángulos (índices)
/// 2. Calcula producto cruz (cross product)
/// 3. Normaliza el vector
/// 4. Lo suma a los vértices involucrados
/// 5. Normaliza el resultado final
///
/// =============================================================================
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
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


/// =============================================================================
/// CREAR OBJETOS BÁSICOS DE LA ESCENA
/// =============================================================================
///
/// Incluye:
/// 
/// - Piso
/// - Vegetación (billboards)
///
/// Estrategia:
/// - Definir vértices + índices
/// - Calcular normales (si aplica)
/// - Crear malla
/// - Guardar en lista
///
/// =============================================================================
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
	calcAverageNormals(indices, 12, vertices, 32, 8, 5);


	
	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

	Mesh* obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);
}


/// =============================================================================
/// CREACIÓN DE SHADERS
/// =============================================================================
///
/// Los shaders controlan:
/// - Transformaciones
/// - Iluminación
/// - Aplicación de texturas
///
/// Aquí simplemente:
/// - Se cargan desde archivos
/// - Se almacenan en una lista
///
/// =============================================================================
void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

/// =============================================================================
/// CREAR DADO TEXTURIZADO (CUBO)
/// =============================================================================
///
/// Características:
/// - 6 caras
/// - Coordenadas de textura (UV)
/// - Normales definidas manualmente
///
/// Nota:
/// - Cada cara tiene sus propios vértices para permitir texturizado correcto.
///
/// =============================================================================
void CrearDado()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		
		// back
		8, 9, 10,
		10, 11, 8,

		// left
		12, 13, 14,
		14, 15, 12,
		// bottom
		16, 17, 18,
		18, 19, 16,
		// top
		20, 21, 22,
		22, 23, 20,

		// right
		4, 5, 6,
		6, 7, 4,

	};
	//Ejercicio 1: reemplazar con sus dados de 6 caras texturizados, agregar normales
	GLfloat cubo_vertices[] = {
		// front
		//x		y		z		S		T			NX		NY		NZ
		-0.5f, -0.5f,  0.5f,	0.34f,  0.26f,		0.0f,	0.0f,	-1.0f,// 0
		0.5f, -0.5f,  0.5f,		0.66f,	0.26f,		0.0f,	0.0f,	-1.0f,// 1
		0.5f,  0.5f,  0.5f,		0.66f,  0.49f,		0.0f,	0.0f,	-1.0f,// 2
		-0.5f,  0.5f,  0.5f,	0.34f,	0.49f,		0.0f,	0.0f,	-1.0f,// 3
		// right
		//x		y		z		S		T
		0.5f, -0.5f,  0.5f,	    0.34f,  0.01f,		-1.0f,	0.0f,	0.0f,// 4
		0.5f, -0.5f,  -0.5f,	0.66f,	0.01f,		-1.0f,	0.0f,	0.0f,// 5
		0.5f,  0.5f,  -0.5f,	0.66f,	0.24f,		-1.0f,	0.0f,	0.0f,// 6
		0.5f,  0.5f,  0.5f,	    0.34f,	0.24f,		-1.0f,	0.0f,	0.0f,// 7
		// back
		-0.5f, -0.5f, -0.5f,	0.66f,  0.76f,		0.0f,	0.0f,	1.0f,// 8
		0.5f, -0.5f, -0.5f,		0.34f,	0.76f,		0.0f,	0.0f,	1.0f,// 9
		0.5f,  0.5f, -0.5f,		0.34f,	0.99f,		0.0f,	0.0f,	1.0f,// 10
		-0.5f,  0.5f, -0.5f,	0.66f,	0.99f,		0.0f,	0.0f,	1.0f,// 11

		// left
		//x		y		z		S		T
		-0.5f, -0.5f,  -0.5f,	0.34f,  0.51f,		1.0f,	0.0f,	0.0f,// 12
		-0.5f, -0.5f,  0.5f,	0.66f,  0.51f,		1.0f,	0.0f,	0.0f,// 13
		-0.5f,  0.5f,  0.5f,	0.66f,  0.74f,		1.0f,	0.0f,	0.0f,// 14
		-0.5f,  0.5f,  -0.5f,	0.34f,  0.74f,		1.0f,	0.0f,	0.0f,// 15

		// bottom
		//x		y		z		S		T
		-0.5f, -0.5f,  0.5f,	0.01f,  0.74f,		0.0f,	1.0f,	0.0f,// 16
		0.5f,  -0.5f,  0.5f,	0.33f, 0.74f,		0.0f,	1.0f,	0.0f,// 17
		 0.5f,  -0.5f, -0.5f,	0.33f, 0.5f,        0.0f,   1.0f,   0.0f,// 18
		-0.5f, -0.5f,  -0.5f,	0.01f,  0.5f,		0.0f,	1.0f,	0.0f,// 19

		//UP
		 //x		y		z		S		T
		 -0.5f, 0.5f,  0.5f,	0.67f,  0.51f,		0.0f,	-1.0f,	0.0f,// 20
		 0.5f,  0.5f,  0.5f,	0.99f,	0.51f,		0.0f,	-1.0f,	0.0f,// 21
		  0.5f, 0.5f,  -0.5f,	0.99f,  0.74f,      0.0f,   -1.0f,  0.0f,// 22
		 -0.5f, 0.5f,  -0.5f,	0.67f,	0.74f,		0.0f,	-1.0f,	0.0f,// 23

	};

	Mesh* dado = new Mesh();
	dado->CreateMesh(cubo_vertices, cubo_indices, 192, 36);
	meshList.push_back(dado);

}
/// =============================================================================
/// CREAR OCTAEDRO TEXTURIZADO (CUBO)
/// =============================================================================
///
/// Características:
/// - 8 caras
/// - Coordenadas de textura (UV)
/// - Normales definidas manualmente
///
/// Nota:
/// - Cada cara tiene sus propios vértices para permitir texturizado correcto.
///
/// =============================================================================
void CrearOctaedro()
{
	unsigned int octaedro_indices[] = {
		//top
		// frontD
		0, 1, 2,
		// FronI
		3, 4, 5,
		// backD
		6, 7, 8,
		// backI
		9, 10, 11,

		// bottom
		// frontD
		12, 13, 14,
		// FronI
	    15, 16, 17,
		// backD
		18, 19, 20,
		// backI
		21, 22, 23,

	};

	GLfloat octaedro_vertices[] = {
		// frontD
		//x		y		z	S		T			NX		NY		NZ
		0.0f, 0.5f,  0.0f,	0.0f,   0.33f,		0.0f,	0.0f,	0.0f,// 0
		0.0f, 0.0f,  0.5f,	0.125f,	0.33f,		0.0f,	0.0f,	0.0f,// 1
		0.5f, 0.0f,  0.0f,	0.25f,  0.66f,		0.0f,	0.0f,	0.0f,// 2
		// frontI
		//x		y		z	S		T			NX		NY		NZ
		0.0f, 0.5f,  0.0f,	0.25f,	0.66f,	    0.0f,	0.0f,	 0.0f,// 3
		0.0f, 0.0f,  0.5f,	0.125f, 0.33f,	    0.0f,	0.0f,	 0.0f,// 4
		-0.5f, 0.0f,  0.0f,	0.5f,	0.66f,	    0.0f,	0.0f,	 0.0f,// 5
		// backD
		//x		y		z	S		T			NX		NY		NZ
		0.0f, 0.5f,  0.0f,	0.125f, 0.33f,		0.0f,	0.0f,	0.0f,// 6
		0.5f,  0.0f,  0.0f,	0.5f,	0.66f,		0.0f,	0.0f,	0.0f,// 7
		0.0f, 0.0f,  -0.5f,	0.625f, 0.33f,		0.0f,	0.0f,	0.0f,// 8
		// backI
		//x		y		z	S		T			NX		NY		NZ
		0.0f, 0.5f,  0.0f,	0.125f, 0.33f,		0.0f,	0.0f,	0.0f,// 9
		-0.5f, 0.0f,  0.0f,	0.5f,	0.0f,	    0.0f,	0.0f,	0.0f,// 10
		0.0f, 0.0f,  -0.5f,	0.625f, 0.33f,		0.0f,	0.0f,	0.0f,// 11
		// bottom
		//frontD
		//x		y		z	S		T			NX		NY		NZ
		0.0f, -0.5f,  0.0f,	0.5f,  0.66f,		0.0f,	0.0f,	0.0f,// 12
		0.0f, 0.0f,  0.5f,	0.625f,0.33f,		0.0f,	0.0f,	0.0f,// 13
		0.5f,  0.0f,  0.0f,	0.75f, 0.66f,		0.0f,	0.0f,	0.0f,// 14
		// frontI
		//x		y		z	S		T			NX		NY		NZ
		0.0f, -0.5f,  0.0f,	0.5f,  0.66f,		0.0f,	0.0f,	0.0f,// 15
		0.0f, 0.0f,  0.5f,	0.625f,1.0f,		0.0f,	0.0f,	0.0f,// 16
		-0.5f, 0.0f,  0.0f,	0.75f, 0.66f,		0.0f,	0.0f,	0.0f,// 17
		// backD
		//x		y		z	S		T			NX		NY		NZ
		0.0f, -0.5f,  0.0f,	0.625f, 0.33f,      0.0f,   0.0f,   0.0f,// 18
		0.5f,  0.0f,  0.0f,	0.75f,  0.66f,		0.0f,	0.0f,	0.0f,// 19
		0.0f, 0.0f,  -0.5f,	0.875f, 0.33f,		0.0f,	0.0f,	0.0f,// 20
		// backI
		//x		y		z   S		T			NX		NY		NZ
		0.0f, -0.5f,  0.0f,	0.75f,	0.66f,		0.0f,	0.0f,	0.0f,// 21
		-0.5f, 0.0f,  0.0f,	0.875f, 0.33f,      0.0f,   0.0f,   0.0f,// 22
		0.0f, 0.0f,  -0.5f,	1.0f,	0.66f,		0.0f,	0.0f,	0.0f,// 23
	};
	Mesh* octaedro = new Mesh();
	octaedro->CreateMesh(octaedro_vertices, octaedro_indices, 192, 24);
	meshList.push_back(octaedro);

}


/// =============================================================================
/// FUNCIÓN PRINCIPAL
/// =============================================================================
///
/// Flujo:
/// 1. Inicializar ventana
/// 2. Crear objetos
/// 3. Cargar shaders
/// 4. Configurar cámara
/// 5. Cargar texturas
/// 6. Cargar modelos
/// 7. Loop de renderizado
///
/// =============================================================================
int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CrearDado();
	CrearOctaedro();
	CreateShaders();


	camera = Camera(
		glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f), 
		-60.0f, 0.0f, 0.3f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();

	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();

	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();

	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();

	dadoTexture = Texture("Textures/dado_emociones_editado.tga");
	dadoTexture.LoadTextureA();

	octaedroTexture = Texture("Textures/octaedro-ds.tga");
	octaedroTexture.LoadTextureA();

	
	
	Kitt_M = Model();
	Kitt_M.LoadModel("Models/kitt_optimizado.obj");

	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");

	Dado_M = Model();
	Dado_M.LoadModel("Models/dado_emociones.obj");

	Jeep.LoadModel("Models/jeep.obj");

	Llanta_01.LoadModel("Models/jeep_rueda1_texturizada.obj");

	Llanta_02.LoadModel("Models/jeep_rueda2_texturizada.obj");

	Llanta_03.LoadModel("Models/jeep_rueda1_texturizada.obj");

	Llanta_04.LoadModel("Models/jeep_rueda2_texturizada.obj");
	
	Cofre.LoadModel("Models/jeep_cofre_texturizado.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, 
		   uniformModel = 0, 
		   uniformView = 0, 
		   uniformEyePosition = 0,
		   uniformSpecularIntensity = 0, 
		   uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(
		45.0f, 
		(GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 
		0.1f, 1000.0f);
	
	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		// Tiempo
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();

		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		color = glm::vec3(1.0f, 1.0f, 1.0f);//color blanco, multiplica a la información de color de la textura

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		meshList[2]->RenderMesh();


		////Dado de Opengl
		////Ejercicio 1: Texturizar su cubo con la imagen dado_animales ya optimizada por ustedes
		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(-1.5f, 4.5f, -2.0f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//dadoTexture.UseTexture();
		//meshList[4]->RenderMesh();
		//
		////Ejercicio 2:Importar el cubo texturizado en el programa de modelado con 
		////la imagen dado_animales ya optimizada por ustedes
		//
		////Dado importado
		//model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(-3.0f, 3.0f, -2.0f));
		//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		//Dado_M.RenderModel();


		/*Reporte de práctica :
		Ejercicio 1: Crear un dado de 8 caras y texturizarlo por medio de código
		Ejercicio 2: Importar el modelo de su coche con sus 4 llantas acomodadas
		y tener texturizadas las 4 llantas (diferenciar caucho y rin)
		*/

		//Dado de Opengl
		//Ejercicio 1: Crear un dado de 8 caras y texturizarlo por medio de código
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(1.0f, 2.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		octaedroTexture.UseTexture();
		meshList[5]->RenderMesh();

		/*Ejercicio 2: Importar el modelo de su coche con sus 4 llantas acomodadas
		y tener texturizadas las 4 llantas(diferenciar caucho y rin)*/

		//----------------------------------------------------
		/// CUERPO (NODO RAÍZ)
		//----------------------------------------------------

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Jeep.RenderModel();

		/// Guardamos transformación base
		modelaux = model;

		//----------------------------------------------------
		/// Ruedas (jerárquicas)
		//----------------------------------------------------

		// Rueda 1
		model = modelaux;
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion1()), glm::vec3(1, 0, 0));
		model = glm::translate(model, glm::vec3(-0.77f, -0.3f, 1.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_01.RenderModel();

		// Rueda 2
		model = modelaux;
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion1()), glm::vec3(1, 0, 0));
		model = glm::translate(model, glm::vec3(0.74f, -0.3f, 1.1f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_02.RenderModel();

		// Rueda 3
		model = modelaux;
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion1()), glm::vec3(1, 0, 0));
		model = glm::translate(model, glm::vec3(-0.77f, 1.2f, -0.99f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_03.RenderModel();

		// Rueda 4
		model = modelaux;
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion1()), glm::vec3(1, 0, 0));
		model = glm::translate(model, glm::vec3(0.74f, 1.2f, -0.99f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_04.RenderModel();

		////----------------------------------------------------
		///// Cofre (No funciona)
		////----------------------------------------------------
		model = modelaux;
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(1, 0, 0));
		model = glm::translate(model, glm::vec3(0.0f, 0.4f, 1.2f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Cofre.RenderModel();

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
/*
//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		logofiTexture.UseTexture(); //textura con transparencia o traslucidez
		FIGURA A RENDERIZAR de OpenGL, si es modelo importado no se declara UseTexture
		glDisable(GL_BLEND);
*/