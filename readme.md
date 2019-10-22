## Kinc

Kinc projects are built using kincmake, a nodejs tool.
In your project's directory call `node Kinc/make` - this will
create a project file for your IDE in a build subdirectory.
kincmake by default creates a project for the system you are currently using,
but you can also put one of windows, linux, android, windowsapp, osx and ios
in the arguments list to create something else. In Windows you can also
choose your graphics api (-g direct3d9/direct3d11/direct3d12/opengl/vulkan/metal)
and your Visual Studio version (-v vs2010/vs2012/vs2013/vs2015/vs2017/2019).
