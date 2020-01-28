mkdir -p BlueWindow.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o BlueWindow.app/Contents/MacOS/BlueWindow BlueWindow.mm -framework Cocoa -framework QuartzCore -framework OpenGL
