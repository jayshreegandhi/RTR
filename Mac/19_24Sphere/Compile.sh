mkdir -p 24Sphere.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o 24Sphere.app/Contents/MacOS/24Sphere 24Sphere.mm -framework Cocoa -framework QuartzCore -framework OpenGL
