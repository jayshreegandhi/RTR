mkdir -p ThreeRotatingLightsOnSteadySpherePerFragment.app/Contents/MacOS

Clang++ -Wno-deprecated-declarations -o ThreeRotatingLightsOnSteadySpherePerFragment.app/Contents/MacOS/ThreeRotatingLightsOnSteadySpherePerFragment ThreeRotatingLightsOnSteadySpherePerFragment.mm -framework Cocoa -framework QuartzCore -framework OpenGL
