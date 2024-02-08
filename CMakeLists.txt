cmake_minimum_required(VERSION 3.9)

project(MeshColorTexture LANGUAGES CXX)

find_package(OpenMP REQUIRED)

add_subdirectory(External)

add_executable (
	MeshColorTexture
	Sources/Main.cpp
	Sources/Camera.h
	Sources/Camera.cpp
	Sources/Console.h
	Sources/Console.cpp
	Sources/Error.h
	Sources/Error.cpp
	Sources/Image.h
	Sources/Image.cpp
	Sources/IO.h
	Sources/IO.cpp
	Sources/LightSource.h
	Sources/Material.h 
	Sources/Mesh.h
	Sources/Mesh.cpp
	Sources/MeshColor.h
	Sources/MeshColorTexture.h
	Sources/RayTracer.h
	Sources/RayTracer.cpp
	Sources/Rasterizer.h
	Sources/Rasterizer.cpp
	Sources/Renderer.h
	Sources/Resources.h
	Sources/ShaderProgram.h
	Sources/ShaderProgram.cpp
	Sources/Scene.h
	Sources/Transform.h
)

set_target_properties(MeshColorTexture PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Copy the shader files in the binary location.

add_custom_command(TARGET MeshColorTexture 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MeshColorTexture> ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} External/stb_image/)

target_link_libraries(MeshColorTexture LINK_PRIVATE glad)

target_link_libraries(MeshColorTexture LINK_PRIVATE glfw)

target_link_libraries(MeshColorTexture LINK_PRIVATE glm)

target_link_libraries(MeshColorTexture PRIVATE OpenMP::OpenMP_CXX)


