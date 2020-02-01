mkdir -p PyramidAndCube.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o PyramidAndCube.app/Contents/MacOS/PyramidAndCube PyramidAndCube.mm -framework Cocoa -framework QuartzCore -framework OpenGL
