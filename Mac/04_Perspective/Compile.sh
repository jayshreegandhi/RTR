mkdir -p Perspective.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o Perspective.app/Contents/MacOS/Perspective Perspective.mm -framework Cocoa -framework QuartzCore -framework OpenGL
