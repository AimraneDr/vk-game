@echo off

REM Check if the output directory exists, if not, create it
if not exist "bin\shaders" (
    mkdir "bin\shaders"
)

REM Compile the shaders
C:\VulkanSDK\1.3.290.0\Bin\glslc.exe resources\shader.vert -o bin\shaders\vert.spv
C:\VulkanSDK\1.3.290.0\Bin\glslc.exe resources\shader.frag -o bin\shaders\frag.spv

@echo Shaders compiled successfully.