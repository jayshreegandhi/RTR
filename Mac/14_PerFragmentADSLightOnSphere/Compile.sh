mkdir -p PerFragmentADSLightOnSphere.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o PerFragmentADSLightOnSphere.app/Contents/MacOS/PerFragmentADSLightOnSphere PerFragmentADSLightOnSphere.mm -framework Cocoa -framework QuartzCore -framework OpenGL
