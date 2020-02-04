mkdir -p TweakedSmiley.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o TweakedSmiley.app/Contents/MacOS/TweakedSmiley TweakedSmiley.mm -framework Cocoa -framework QuartzCore -framework OpenGL
