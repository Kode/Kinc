* 2023-12-16: Dave is at it again and now the gamepad-callbacks also take an additional data-parameter.
* 2023-03-23: The socket-API is getting some revisions...
* 2023-03-11: The index-buffer-API was revised. It can now lock partially (like the vertex-buffers) and the return-value of locks was changed to void\*. To fix your code, simply append "\_all" to your lock/unlock-calls and maybe add a cast to int\* - or better yet cast to the proper types which will be uint16_t\* or uint32_t\* depending on the format-parameter of your index-buffer.
* 2023-02-11: The randomizer now uses xoshiro256** instead of a mermeme-twister.
* 2023-02-09: The keyboard-callbacks now take an additional data-parameter as well but it's again Dave's fault.
* 2022-12-19: The mouse-callbacks now take an additional data-parameter as well but it's Dave's fault.
* 2022-12-17: The callbacks in system.h now all take an additional data-parameter that's set via the callback-set-functions.
* 2022-12-14: The --shaderversion-parameter for kmake was removed. In its place there's now a Project.setMinimumShaderVersion-function that can be used in kfiles.
* 2022-09-29: kinc_g4_non_pow2_textures_supported was renamed to kinc_g4_supports_non_pow2_textures.
* 2022-09-08: The init-functions for render-targets have been changed - it should be apparent from the function-signatures what has to be done, they are simply less silly now.
* 2022-08-15: The C-replacement-functions (memory.h, string.h and some functions in math/core.h) are being removed in favor of some code that directly replaces the parts of the C-library that are used by Kinc.
* 2022-07-30: The html5-target has been renamed to emscripten.
* 2022-07-23: Kinc and Kore have been split up into separate repositories. If you are using the Kinc-API and your repo points to https://github.com/Kode/Kinc or you are using the Kore-API and your repo points to https://github.com/Kode/Kore, things should just continue to work though. Otherwise, just change that, git fetch the changes and git reset to the latest commit on main.
* 2022-06-06: The stencil-API changed a little to support separate stencil-settings for front-sides and back-sides.
* 2022-05-02: The submodules have been reorganized so only the relevant files for the current system have to be downloaded. When doing a recursive clone everything is still pretty much as it used to be but alternatively you can now do a regular clone and then call Kinc/get_dlc to only download what's actually needed.

Also kincmake has now been completely replaced by kmake but it should be compatible with old files so just call Kinc/make (without any node) and everything will just continue to work - maybe.
* 2022-01-23: Blending has been reworked and blending-operations are actually blending-operations. The former blending-operations have been renamed to blending-factors.
* 2021-12-21: A parameter specyfiying the index-format was added to kinc_g5_index_buffer_init
* 2021-11-15: The input callbacks are now set using function calls like in other parts of the API.
* 2021-07-05: kinc_g4_index_buffer_init now has an additional usage-parameter, just like kinc_g4_vertex_buffer_init. Pass KINC_G4_USAGE_STATIC to retain the previous behaviour.
* 2021-05-24: The window-parameter has been removed from most of the mouse-locking functions. If you use them, just remove that parameter, too.
* 2020-09-19: When using a kincfile.js by default the Kore API isn't included anymore. It continues to be auto-included when using a korefile.js and can also be included via a kincfile by setting project.cpp to true.
* 2019-06-01: Kore is now a C project (called Kinc, short for Kore in C) with a C++ wrapper (still called Kore). For C++ everything should still work the same (in theory) but for one detail: The start procedure is now called kickstart instead of kore.
* 2018-07-08: Kore::Window and Kore::Display have been overhauled and Kore::System was simplified. Projects are now always initialized using Kore::System::init (see Tests/Empty/Sources/Shader.cpp), additional windows are created using Kore::Window::create.
* 2017-02-10: createProject and addSubProject were replaced by the simpler addProject. New code looks like this:

```
let project = new Project('Cool Name');
await project.addProject('aCoolLib');
resolve(project);
```

addProject is asynchronous, please do not forget to await it.

Kore itself is now added automatically so if you used createProject to add Kore, just remove it.

* 2017-04-02: What was previously Kore::Mixer and Kore::Audio is now Kore::Audio1 and Kore::Audio2.
* 2017-03-30: What was previously Kore::Graphics is now Kore::Graphics4.
