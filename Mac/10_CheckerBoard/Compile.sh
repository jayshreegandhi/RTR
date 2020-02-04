mkdir -p CheckerBoard.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o CheckerBoard.app/Contents/MacOS/CheckerBoard CheckerBoard.mm -framework Cocoa -framework QuartzCore -framework OpenGL
