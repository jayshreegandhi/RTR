mkdir -p LightSphere.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o LightSphere.app/Contents/MacOS/LightSphere LightSphere.mm -framework Cocoa -framework QuartzCore -framework OpenGL
