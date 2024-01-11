## Kinc

Kinc (short for [Kore](https://github.com/Kode/Kore) in C) is a low level toolkit
for cross-platform game engine development and similar endeavors. It is comparable
to SDL but bigger in scope as it also takes care of cross-platform GPU programming
with multiple portable APIs to choose from and its own shader language
[Kongruent](https://github.com/Kode/Kongruent).

Kinc projects are built using kmake, a meta-build-tool that resides in
a git-submodule of Kinc. In your project's directory call `path/to/Kinc/make`,
this will create a project file for your IDE in a subdirectory called build.
kmake by default creates a project for the system you are currently using,
but you can also put one of windows, linux, android, windowsapp, osx, ios
and freebsd in the arguments list to create something else.
Depending on the capabilities of the target system you can also choose
your graphics api (-g direct3d9/direct3d11/direct3d12/opengl/vulkan/metal).
Console support is implemented via separate plugins because the code can not
be provided publicly - contact Robert for details if you are interested in it.
