mkdir -p GeometryShader.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o GeometryShader.app/Contents/MacOS/GeometryShader GeometryShader.mm -framework Cocoa -framework QuartzCore -framework OpenGL
