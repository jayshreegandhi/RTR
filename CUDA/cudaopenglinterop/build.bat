cl.exe /c /EHsc /I C:\glew-2.1.0\include /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.1\include" CudaOGLInterop.cpp

nvcc.exe -c -I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.1\include" -o SinewaveVbo.obj SinewaveVbo.cu

link.exe /LIBPATH:C:\glew-2.1.0\lib\Release\x64 /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v10.1\lib\x64" /OUT:Sinewave.exe CudaOGLInterop.obj SinewaveVbo.obj