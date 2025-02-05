@echo off

REM Check if the output directory exists, if not, create it
if not exist "game\bin\shaders\default" (
    mkdir "game\bin\shaders\default"
)

REM Compile the shaders
C:\VulkanSDK\1.3.290.0\Bin\glslc.exe engine\resources\shaders\shader.vert -o game\bin\shaders\default\vert.spv
C:\VulkanSDK\1.3.290.0\Bin\glslc.exe engine\resources\shaders\shader.frag -o game\bin\shaders\default\frag.spv

@echo Shaders compiled successfully.