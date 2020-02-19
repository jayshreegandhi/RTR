mkdir -p LightCube.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o LightCube.app/Contents/MacOS/LightCube LightCube.mm -framework Cocoa -framework QuartzCore -framework OpenGL
