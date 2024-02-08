#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Transform.h"

class MeshColor : public Transform {
public:
	virtual ~MeshColor (){
		clear();
	}

	inline const std::vector<glm::vec3> & vertexColors () const { return m_vertexColors; } 
	inline std::vector<glm::vec3> & vertexColors () { return m_vertexColors; }
	inline void addVertexColor (const glm::vec3& vertexColor) { m_vertexColors.push_back(vertexColor); }

    inline const std::vector<glm::vec3> & edgeColors () const { return m_edgeColors; } 
	inline std::vector<glm::vec3> & edgeColors () { return m_edgeColors; }
	inline void addEdgeColor (const glm::vec3& edgeColor) { m_edgeColors.push_back(edgeColor); }

    inline const std::vector<glm::vec3> & faceColors () const { return m_faceColors; } 
	inline std::vector<glm::vec3> & faceColors () { return m_faceColors; }
	inline void addFaceColor (const glm::vec3& faceColor) { m_faceColors.push_back(faceColor); }
	
	void clear (){
		m_vertexColors.clear();
		m_edgeColors.clear();
		m_faceColors.clear();
	};

private:
	std::vector<glm::vec3> m_vertexColors;  // RGB for vertices
	std::vector<glm::vec3> m_edgeColors;    // RGB for edges 
	// The order will be 0 1, 1 2, 2 0  of the face
	std::vector<glm::vec3> m_faceColors;    // RGB for faces
};
