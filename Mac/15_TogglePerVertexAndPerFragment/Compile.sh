mkdir -p TogglePerVertexAndPerFragment.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o TogglePerVertexAndPerFragment.app/Contents/MacOS/TogglePerVertexAndPerFragment TogglePerVertexAndPerFragment.mm -framework Cocoa -framework QuartzCore -framework OpenGL
