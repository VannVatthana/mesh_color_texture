// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Transform.h"
#include "MeshColor.h"

class Mesh : public Transform {
public:
	virtual ~Mesh ();

	inline const std::vector<glm::vec3> & vertexPositions () const { return m_vertexPositions; } 
	inline std::vector<glm::vec3> & vertexPositions () { return m_vertexPositions; }
	inline void addVertexPosition (const glm::vec3& vertexPos) { m_vertexPositions.push_back(vertexPos); }

	inline const std::vector<std::vector<glm::vec2>> & textureCoordinates() const { return m_textureCoordinates; }
	inline const std::vector<std::vector<glm::vec2>> & textureCoordinates() { return m_textureCoordinates; }
	inline void addTextureCoordinates (const std::vector<glm::vec2>& textureCoord) { m_textureCoordinates.push_back(textureCoord);}

	inline const std::vector<glm::vec3> & vertexNormals () const { return m_vertexNormals; } 
	inline std::vector<glm::vec3> & vertexNormals () { return m_vertexNormals; } 
	inline void addVertexNormal (const glm::vec3& vertexNormal) {m_vertexNormals.push_back(vertexNormal);}
	
	inline const std::vector<glm::uvec3> & triangleIndices () const { return m_triangleIndices; }
	inline std::vector<glm::uvec3> & triangleIndices () { return m_triangleIndices; }
	inline void addTriangleFace (const glm::uvec3& triangleFace) {m_triangleIndices.push_back(triangleFace);}
	
	inline const std::vector<std::vector<glm::vec3>> & mipmaps () const { return m_mipmaps; }
	inline std::vector<std::vector<glm::vec3>> & mipmaps () { return m_mipmaps; }

	inline const MeshColor & meshColors () const {return m_meshcolors;}
	inline MeshColor & meshColors () {return m_meshcolors;}

	inline const int & resolution () const {return m_resolution;}
	inline void setResolution (int res) {m_resolution = res;}

	void recomputePerVertexNormals (bool angleBased = false);

	void clear ();

private:
	std::vector<glm::vec3> m_vertexPositions;
	std::vector<glm::vec3> m_vertexNormals;
	
	std::vector<glm::uvec3> m_triangleIndices;
	int m_resolution = 0; // Power of 2 (or R = 2^r - 1), but here I store small r

	std::vector<std::vector<glm::vec2>> m_textureCoordinates;   // Texture coordinates for the 3 vertices of each face

	MeshColor m_meshcolors;

	std::vector<std::vector<glm::vec3>> m_mipmaps;
};
