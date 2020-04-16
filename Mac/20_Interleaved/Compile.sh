mkdir -p Interleaved.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o Interleaved.app/Contents/MacOS/Interleaved Interleaved.mm -framework Cocoa -framework QuartzCore -framework OpenGL
