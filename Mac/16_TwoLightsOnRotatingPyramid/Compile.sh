mkdir -p TwoLightsOnRotatingPyramid.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o TwoLightsOnRotatingPyramid.app/Contents/MacOS/TwoLightsOnRotatingPyramid TwoLightsOnRotatingPyramid.mm -framework Cocoa -framework QuartzCore -framework OpenGL
