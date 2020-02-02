mkdir -p TexturePyramidAndCube.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o TexturePyramidAndCube.app/Contents/MacOS/TexturePyramidAndCube TexturePyramidAndCube.mm -framework Cocoa -framework QuartzCore -framework OpenGL
