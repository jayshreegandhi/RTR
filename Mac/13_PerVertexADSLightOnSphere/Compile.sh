mkdir -p PerVertexADSLightOnSphere.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o PerVertexADSLightOnSphere.app/Contents/MacOS/PerVertexADSLightOnSphere PerVertexADSLightOnSphere.mm -framework Cocoa -framework QuartzCore -framework OpenGL
