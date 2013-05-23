To build one of the examples navigate to it's directory and call kake.
For example:

cd Examples/Shader
../../Tools/kake/kake

This will create a project file for your IDE in a build subdirectory.
kake by default creates a project for the system you are currently using,
but you can also put one of windows, linux, android, windows8, osx and ios
in the arguments list to create something else. In Windows you can also
choose your graphics api (gfx=direct3d9, gfx=direct3d11 or gfx=opengl2)
and your Visual Studio version (vs=vs2010 or vs=vs2012).