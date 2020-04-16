mkdir -p TessellationShader.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o TessellationShader.app/Contents/MacOS/TessellationShader TessellationShader.mm -framework Cocoa -framework QuartzCore -framework OpenGL
