mkdir -p Color2DShape.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o Color2DShape.app/Contents/MacOS/Color2DShape Color2DShape.mm -framework Cocoa -framework QuartzCore -framework OpenGL
