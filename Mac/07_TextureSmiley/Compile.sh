mkdir -p TextureSmiley.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o TextureSmiley.app/Contents/MacOS/TextureSmiley TextureSmiley.mm -framework Cocoa -framework QuartzCore -framework OpenGL
