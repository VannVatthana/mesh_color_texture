// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "Rasterizer.h"
#include <glad/glad.h>
#include "Resources.h"
#include "Error.h"

void Rasterizer::init (const std::string & basePath, const std::shared_ptr<Scene> scenePtr) {
 	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
	// GPU resources
	initScreeQuad ();
	loadShaderProgram (basePath);
	initDisplayedImage ();
	// Allocate GPU ressources for the heavy data components of the scene 
	size_t numOfMeshes = scenePtr->numOfMeshes ();
	for (size_t i = 0; i < numOfMeshes; i++) 
		m_vaos.push_back (toGPU (scenePtr->mesh (i)));
}

void Rasterizer::setResolution (int width, int height)  {
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

void Rasterizer::loadShaderProgram (const std::string & basePath) {
	std::string shaderPath = basePath + "/" + SHADER_PATH;
	// PBR shader
	m_pbrShaderProgramPtr.reset ();
	try {
		m_pbrShaderProgramPtr = ShaderProgram::genBasicShaderProgram (shaderPath + "/PBRVertexShader.glsl",
													         	 	  shaderPath + "/PBRFragmentShader.glsl");
		
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}
	// Display Shader
	m_displayShaderProgramPtr.reset ();
	try {
		m_displayShaderProgramPtr = ShaderProgram::genBasicShaderProgram (shaderPath + "/DisplayVertexShader.glsl",
													         	 		  shaderPath + "/DisplayFragmentShader.glsl");
		m_displayShaderProgramPtr->set ("imageTex", 0);
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading display shader program]") + e.what ());
	}
}

void Rasterizer::updateDisplayedImageTexture (std::shared_ptr<Image> imagePtr) {
	glBindTexture (GL_TEXTURE_2D, m_displayImageTex);
   	// Uploading the image data to GPU memory
	glTexImage2D (
		GL_TEXTURE_2D, 
		0, 
   		GL_RGB, // We assume only greyscale or RGB pixels
   		static_cast<GLsizei> (imagePtr->width()), 
   		static_cast<GLsizei> (imagePtr->height()), 
   		0, 
   		GL_RGB, // We assume only greyscale or RGB pixels
   		GL_FLOAT, 
   		imagePtr->pixels().data());
   	// Generating mipmaps for filtered texture fetch
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, 0);
}

void Rasterizer::initDisplayedImage () {
	// Creating and configuring the GPU texture that will contain the image to display
	glGenTextures (1, &m_displayImageTex);
	glBindTexture (GL_TEXTURE_2D, m_displayImageTex);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture (GL_TEXTURE_2D, 0);
}

// The main rendering call
void Rasterizer::render (std::shared_ptr<Scene> scenePtr) {
	const glm::vec3 & bgColor = scenePtr->backgroundColor ();
	glClearColor (bgColor[0], bgColor[1], bgColor[2], 1.f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	setLightSources (scenePtr);
	size_t numOfMeshes = scenePtr->numOfMeshes ();
	m_pbrShaderProgramPtr->use ();
	setCamera (scenePtr);
	glm::mat4 viewMatrix = scenePtr->camera()->computeViewMatrix ();
	for (size_t i = 0; i < numOfMeshes; i++) {
		size_t matId = scenePtr->mesh2material (i);
		setMaterial (scenePtr->material (matId));
		glm::mat4 modelMatrix = scenePtr->mesh (i)->computeTransformMatrix ();
		glm::mat4 normalMatrix = glm::transpose (glm::inverse (viewMatrix * modelMatrix));
		m_pbrShaderProgramPtr->set ("modelMat", modelMatrix);
		m_pbrShaderProgramPtr->set ("normalMat", normalMatrix);
		draw (i, scenePtr->mesh (i)->triangleIndices().size ());
	}
	m_pbrShaderProgramPtr->stop ();

}

void Rasterizer::display (std::shared_ptr<Image> imagePtr) {
	updateDisplayedImageTexture (imagePtr);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	m_displayShaderProgramPtr->use (); // Activate the program to be used for upcoming primitive
	m_displayShaderProgramPtr->set ("width", int (imagePtr->width()));
	m_displayShaderProgramPtr->set ("height", int (imagePtr->height()));
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, m_displayImageTex);
	glBindVertexArray (m_screenQuadVao); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (6), GL_UNSIGNED_INT, 0);
	m_displayShaderProgramPtr->stop ();
	glBindTexture (GL_TEXTURE_2D, 0);
}

std::shared_ptr<Image> Rasterizer::generateImage () const {
	GLint viewport[4];
    glGetIntegerv (GL_VIEWPORT, viewport);
    std::shared_ptr<Image> imagePtr = std::make_shared<Image> (viewport[2], viewport[3]);
    unsigned char * data = new unsigned char [3*viewport[2]*viewport[3]];
    glReadPixels (0,0, viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, (void*)data);
    for (int x = 0; x < viewport[2]; x++)
#pragma omp parallel for
    	for (int y = 0; y < viewport[3]; y++)
    		imagePtr->operator() (x, y) = glm::vec3 (data[3*(y*viewport[2]+x)]/255.f, 
    												 data[3*(y*viewport[2]+x)+1]/255.f, 
    												 data[3*(y*viewport[2]+x)+2]/255.f);
    delete [] data;
    return imagePtr;
}

void Rasterizer::clear () {
	for (unsigned int i = 0; i < m_posVbos.size (); i++) {
		GLuint vbo = m_posVbos[i];
		glDeleteBuffers (1, &vbo);
	}
	m_posVbos.clear ();
	for (unsigned int i = 0; i < m_normalVbos.size (); i++) {
		GLuint vbo = m_normalVbos[i];
		glDeleteBuffers (1, &vbo);
	}
	m_normalVbos.clear ();
	for (unsigned int i = 0; i < m_ibos.size (); i++) {
		GLuint ibo = m_ibos[i];
		glDeleteBuffers (1, &ibo);
	}
	m_ibos.clear ();
	for (unsigned int i = 0; i < m_vaos.size (); i++) {
		GLuint vao = m_vaos[i];
		glDeleteVertexArrays (1, &vao);
	}
	m_ibos.clear ();
}

GLuint Rasterizer::genGPUBuffer (size_t elementSize, size_t numElements, const void * data) {
	GLuint vbo;
	glGenBuffers (1, &vbo); // Generate a GPU buffer to store the positions of the vertices
	size_t size = elementSize * numElements; // Gather the size of the buffer from the CPU-side vector
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	return vbo;
}

GLuint Rasterizer::genGPUVertexArray (GLuint posVbo, GLuint ibo, bool hasNormals, GLuint normalVbo, GLuint colorVbo) {
	GLuint vao;
	glGenVertexArrays (1, &vao); // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, posVbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	GLuint attrib = 1;
	if (hasNormals) {
		glEnableVertexAttribArray (attrib);
		glBindBuffer (GL_ARRAY_BUFFER, normalVbo);
		glVertexAttribPointer (attrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
		attrib++; // Replicate this strategy for more vertex attributes
	}
	glEnableVertexAttribArray (attrib);
	glBindBuffer (GL_ARRAY_BUFFER, colorVbo);
	glVertexAttribPointer (attrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray (0); // Desactive the VAO just created. Will be activated at rendering time.
	return vao;
}

GLuint Rasterizer::toGPU (std::shared_ptr<Mesh> meshPtr) {
	GLuint posVbo = genGPUBuffer (3 * sizeof (float), meshPtr->vertexPositions().size(), meshPtr->vertexPositions().data ()); // Position GPU vertex buffer
	GLuint normalVbo = genGPUBuffer (3 * sizeof (float), meshPtr->vertexNormals().size(), meshPtr->vertexNormals().data ()); // Normal GPU vertex buffer
	GLuint vertexColorVbo = genGPUBuffer (3 * sizeof (float), meshPtr->vertexNormals().size(), meshPtr->meshColors().vertexColors().data ()); // Normal GPU vertex buffer
	GLuint edgeColorVbo = genGPUBuffer (3 * sizeof (float), meshPtr->vertexNormals().size(), meshPtr->meshColors().edgeColors().data ()); // Normal GPU vertex buffer
	GLuint faceColorVbo = genGPUBuffer (3 * sizeof (float), meshPtr->vertexNormals().size(), meshPtr->meshColors().faceColors().data ()); // Normal GPU vertex buffer
	GLuint ibo = genGPUBuffer (sizeof (glm::uvec3), meshPtr->triangleIndices().size(), meshPtr->triangleIndices().data ()); // triangle GPU index buffer
	GLuint vao = genGPUVertexArray (posVbo, ibo, true, normalVbo,vertexColorVbo);
	return vao;
}

void Rasterizer::initScreeQuad () {
	std::vector<float> pData = {-1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, 1.0, 0.0};
	std::vector<float> uvData = {0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0};
	std::vector<unsigned int> iData = {0, 1, 2, 0, 2, 3};
	m_screenQuadVao = genGPUVertexArray (
		genGPUBuffer (3*sizeof(float), 4, pData.data()),
		genGPUBuffer (3*sizeof(unsigned int), 2, iData.data()),
		false,
		0, 1);
}

void Rasterizer::setCamera (std::shared_ptr<Scene> scenePtr) {
	m_pbrShaderProgramPtr->use (); // Activate the program to be used for upcoming primitive
	std::shared_ptr<Camera> cameraPtr = scenePtr->camera();
	glm::mat4 projectionMatrix = cameraPtr->computeProjectionMatrix ();
	glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();
	glm::mat4 transInvViewMatrix = glm::transpose (glm::inverse (viewMatrix));
	m_pbrShaderProgramPtr->set ("transInvViewMat", transInvViewMatrix);
	m_pbrShaderProgramPtr->set ("viewMat", viewMatrix);
	m_pbrShaderProgramPtr->set ("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program
}

void Rasterizer::setLightSources (std::shared_ptr<Scene> scenePtr) {
	m_pbrShaderProgramPtr->use (); // Activate the program to be used for upcoming primitive
	unsigned int numOfLightSources = scenePtr->numOfLightSources ();
	m_pbrShaderProgramPtr->set ("numOfLightSources", numOfLightSources);
	for (unsigned int i = 0; i < numOfLightSources; i++) {
		std::string locStr = "lightSources["+std::to_string(i)+"]";
		m_pbrShaderProgramPtr->set (locStr + std::string(".direction"), scenePtr->lightSource(i)->direction());
		m_pbrShaderProgramPtr->set (locStr + std::string(".color"), scenePtr->lightSource(i)->color());
		m_pbrShaderProgramPtr->set (locStr + std::string(".intensity"), scenePtr->lightSource(i)->intensity());
	}
}

void Rasterizer::setMaterial (std::shared_ptr<Material> materialPtr) {
	m_pbrShaderProgramPtr->use ();
	m_pbrShaderProgramPtr->set ("material.albedo", materialPtr->albedo ());
	m_pbrShaderProgramPtr->set ("material.roughness", materialPtr->roughness ());
	m_pbrShaderProgramPtr->set ("material.metallicness", materialPtr->metallicness ());
}

void Rasterizer::draw (size_t meshId, size_t triangleCount) {
	glBindVertexArray (m_vaos[meshId]); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (triangleCount * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}
