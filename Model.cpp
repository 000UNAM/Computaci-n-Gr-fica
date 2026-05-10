#include "Model.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

Model::Model()
{
}

void Model::LoadModel(const std::string& fileName)
{
    ClearModel();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        fileName,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene)
    {
        printf("Fallo al cargar el modelo: %s - %s\n", fileName.c_str(), importer.GetErrorString());
        return;
    }

    LoadNode(scene->mRootNode, scene);
    LoadMaterials(scene);
}

void Model::RenderModel()
{
    for (unsigned int i = 0; i < MeshList.size(); i++)
    {
        unsigned int materialIndex = meshTotex[i];

        if (materialIndex < TextureList.size() && TextureList[materialIndex])
        {
            TextureList[materialIndex]->UseTexture();
        }

        MeshList[i]->RenderMesh();
    }
}

void Model::ClearModel()
{
    for (unsigned int i = 0; i < MeshList.size(); i++)
    {
        delete MeshList[i];
        MeshList[i] = nullptr;
    }
    MeshList.clear();
    meshTotex.clear();

    // Puede haber texturas reutilizadas por varios materiales; no se deben borrar dos veces.
    std::unordered_set<Texture*> deletedTextures;
    for (unsigned int i = 0; i < TextureList.size(); i++)
    {
        Texture* texture = TextureList[i];
        if (texture && deletedTextures.insert(texture).second)
        {
            delete texture;
        }
    }
    TextureList.clear();
}

Model::~Model()
{
    ClearModel();
}

void Model::LoadNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        LoadNode(node->mChildren[i], scene);
    }
}

void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.insert(vertices.end(), {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        });

        if (mesh->mTextureCoords[0])
        {
            vertices.insert(vertices.end(), {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            });
        }
        else
        {
            vertices.insert(vertices.end(), { 0.0f, 0.0f });
        }

        if (mesh->HasNormals())
        {
            // Se conserva el signo de normales que traia tu proyecto original.
            vertices.insert(vertices.end(), {
                -mesh->mNormals[i].x,
                -mesh->mNormals[i].y,
                -mesh->mNormals[i].z
            });
        }
        else
        {
            vertices.insert(vertices.end(), { 0.0f, 1.0f, 0.0f });
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    Mesh* newMesh = new Mesh();
    newMesh->CreateMesh(&vertices[0], &indices[0], vertices.size(), indices.size());

    MeshList.push_back(newMesh);
    meshTotex.push_back(mesh->mMaterialIndex);
}

static std::string getFileNameFromPath(const std::string& path)
{
    size_t slash = path.find_last_of("/\\");
    if (slash == std::string::npos)
    {
        return path;
    }

    return path.substr(slash + 1);
}

static std::string toLowerText(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    return text;
}

void Model::LoadMaterials(const aiScene* scene)
{
    TextureList.resize(scene->mNumMaterials);
    std::unordered_map<std::string, Texture*> loadedTextures;

    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* material = scene->mMaterials[i];
        TextureList[i] = nullptr;

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            {
                std::string filename = getFileNameFromPath(path.data);
                std::string texPath = std::string("Textures/") + filename;

                auto existingTexture = loadedTextures.find(texPath);
                if (existingTexture != loadedTextures.end())
                {
                    TextureList[i] = existingTexture->second;
                }
                else
                {
                    Texture* newTex = new Texture(texPath.c_str());
                    std::string extension = toLowerText(filename.substr(filename.find_last_of('.') + 1));

                    bool loaded = false;
                    if (extension == "tga" || extension == "png")
                    {
                        loaded = newTex->LoadTextureA();
                    }
                    else
                    {
                        loaded = newTex->LoadTexture();
                    }

                    if (loaded)
                    {
                        TextureList[i] = newTex;
                        loadedTextures[texPath] = newTex;
                    }
                    else
                    {
                        printf("Fallo al cargar textura: %s\n", texPath.c_str());
                        delete newTex;
                    }
                }
            }
        }

        if (!TextureList[i])
        {
            TextureList[i] = new Texture("Textures/plain.png");
            TextureList[i]->LoadTextureA();
        }
    }
}
