// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "IO.h" 

#include <iostream>
#include <fstream>
#include <exception>
#include <ios>
#include <algorithm>
#include <sstream>

#include "Console.h"

using namespace std;

std::shared_ptr<Mesh> IO::loadOFFMesh (const std::string & filename) {
    Console::print ("Start loading mesh <" + filename + ">");
    auto meshPtr = std::make_shared<Mesh>();
    ifstream in (filename.c_str ());
    if (!in) 
        throw std::ios_base::failure ("[IO][loadOFFMesh] Cannot open " + filename);
    string offString;
    unsigned int sizeV, sizeT, tmp;
    in >> offString >> sizeV >> sizeT >> tmp;
    auto & P = meshPtr->vertexPositions ();
    auto & T = meshPtr->triangleIndices ();
    P.resize (sizeV);
    T.resize (sizeT);
    size_t tracker = std::max<size_t> (1, (sizeV + sizeT)/20);
    Console::print (" > [", false);
    for (unsigned int i = 0; i < sizeV; i++) {
        if (i % tracker == 0)
            Console::print ("-",false);
        in >> P[i][0] >> P[i][1] >> P[i][2];
    }
    int s;
    for (unsigned int i = 0; i < sizeT; i++) {
        if ((sizeV + i) % tracker == 0)
            Console::print ("-",false);
        in >> s;
        for (unsigned int j = 0; j < 3; j++) 
            in >> T[i][j];
    }
    Console::print ("]\n", false);
    in.close ();
    meshPtr->recomputePerVertexNormals ();
    Console::print ("Mesh <" + filename + "> loaded");
    return meshPtr;
}

std::shared_ptr<Mesh> IO::loadOBJMesh (const std::string & filename) {
	Console::print ("Start loading mesh <" + filename + ">");
	auto meshPtr = std::make_shared<Mesh>();
    ifstream in (filename.c_str ());
    if (!in) 
        throw std::ios_base::failure ("[Mesh Loader][loadOBJ] Cannot open " + filename);
	
    string line;
    while (getline(in, line)) {
        std::istringstream iss(line);
        std::string dataType;
        iss >> dataType;

        if (dataType == "v") {  // Vertex positions
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            meshPtr->addVertexPosition(vertex);
        } else if (dataType == "vn") {  // Vertex normals
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            meshPtr->addVertexNormal(normal);
        } else if (dataType == "vt") {  // Vertex texture coordinates
            glm::vec2 textureCoord;
            iss >> textureCoord.x >> textureCoord.y; 
            //meshPtr->addTextureCoordinates(textureCoord);
        } 
        else if (dataType == "f") {  // Faces  
            std::vector<int> face;
            while (iss) {
                int vertexIndex, textureIndex, normalIndex;
                iss >> vertexIndex;
                if (iss.peek() == '/') {
                    iss.ignore(); // Ignore the '/'
                    // Skip the texture coordinate and normal indices
                    //iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
                    //if (iss.peek()!= '/')
                    //    iss >> textureIndex;
                      //if (iss.peek() == '/') {
                      //  iss.ignore();
                      //  iss >> normalIndex;
                      //}
                }
                if(face.size() < 4)
                    face.push_back(vertexIndex);
            }
            //std::cout<<face.size()<<std::endl;
            //std::cout<<face[0] << " " <<face[1]<<" "<<face[2]<<" "<<face[3]<<std::endl;
            meshPtr->addTriangleFace(glm::uvec3(face[0]-1,face[1]-1,face[2]-1));
            //meshPtr->addTriangleFace(glm::uvec3(face[0]-1,face[2]-1,face[3]-1));         
        } 
        /*else if (dataType == "mtllib") {  // MTL file reference
            iss >> mtlFile;
        } else if (dataType == "usemtl") {  // Use material
            iss >> currentMtl;
        }*/
        
    }
    
    in.close();
    
    //meshPtr->vertexNormals().resize(meshPtr->vertexPositions().size(), glm::vec3(0.f, 0.f, 1.f));
    //meshPtr->recomputePerVertexNormals();

    //for (auto& face : meshPtr->triangleIndices())
    //    std::cout<<face.x << " " <<face.y<< " " << face.z <<std::endl;


    //Load material information from MTL file
    //loadMTL(mtlFile, currentMtl, meshPtr);

    Console::print("Mesh <" + filename + "> loaded");
    return meshPtr;
}

std::shared_ptr<Material> IO::loadMTL (const std::string & filename)
{
    Console::print ("Start loading material <" + filename + ">");
	
    ifstream in (filename.c_str ());
    if (!in) 
        throw std::ios_base::failure ("[Material Loader][loadMTL] Cannot open " + filename);

    //std::vector<std::make_shared<Material>()> materials;
    //Material currentMaterial ;
    auto currentMaterial = std::make_shared<Material>();
    string line;
    while (getline(in, line)) {
        std::istringstream iss(line);
        std::string dataType;
        iss >> dataType;

        /*if(dataType == "newmtl"){
            //if (!currentMaterial->name().empty()) {
                // If not the first material, add the current one to the vector
            //    materials.push_back(currentMaterial);
            //}
            currentMaterial = {};
            string matName;
            iss >> matName;
            currentMaterial->setName(matName);
        }*/ /*else if (dataType == "Ka") {
            // Ambient color
            iss >> currentMaterial->ambient.r >> currentMaterial->ambient.g >> currentMaterial->ambient.b;
        }*/ if (dataType == "Kd") {
            // Diffuse color
            glm::vec3 albedo;
            iss >> albedo[0] >> albedo[1] >> albedo[2];
            currentMaterial->setAlbedo(albedo);
        }/*else if (dataType == "Ks") {
            // Specular color
            //iss >> currentMaterial->specular.r >> currentMaterial->specular.g >> currentMaterial->specular.b;
        }*/ else if (dataType == "Ns") {
            // Shininess
            float shininess;
            iss >> shininess;
            currentMaterial->setRoughness(1-(shininess/1000));
        }
        //currentMaterial->setMetallicness(0.0f);
    }

    // Add the last material to the vector
    //if (!currentMaterial->name().empty()) {
    //    materials.push_back(currentMaterial);
    //}

    in.close();
    //std::cout<< currentMaterial->albedo()[0] << currentMaterial->metallicness() << currentMaterial->roughness() <<std::endl;
    return currentMaterial;
}