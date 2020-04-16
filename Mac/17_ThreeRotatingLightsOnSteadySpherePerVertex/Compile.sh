mkdir -p ThreeRotatingLightsOnSteadySpherePerVertex.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o ThreeRotatingLightsOnSteadySpherePerVertex.app/Contents/MacOS/ThreeRotatingLightsOnSteadySpherePerVertex ThreeRotatingLightsOnSteadySpherePerVertex.mm -framework Cocoa -framework QuartzCore -framework OpenGL
