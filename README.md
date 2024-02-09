# MeshColorTexture
## Skeleton code is from the course INF584 "Image Synthesis" by Tamy Boubekeur

Modified for the project by student: Vannvatthana Norng (vannvatthana.norng@telecom-paris.fr) 

### Building

This is a standard CMake project. Building it consists in running:

```
cd <path-to-MeshColorTexture-directory>
mkdir Build
cd Build
cmake ..
cd ..
cmake --build Build
```

The resuling MeshColorTexture executable is automatically copied to the root MeshColorTexture directory, so that resources (shaders, meshes) can be loaded easily. By default, the program is compile in Debug mode. For a high performance Release binary, just us:

```
cmake --build Build --config Release
```

### Running

To run the program
```
cd <path-to-MeshColorTexture-directory>
./MeshColorTexture [file.off]
```
Note that a collection of example meshes are provided in the Resources/Models directory.

When starting to edit the source code, rerun 

```
cmake --build Build --config Release
```
each time you want to recompile MeshColorTexture

The resulting binary to use is always the one located in the mesh_color_texture directory, you can safely ignore whatever is generated in the Build directory. 
