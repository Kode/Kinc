const child_process = require('child_process');
const fs = require('fs');
const path = require('path');

const excludes = [
	'Backends/System/Android/Sources/android_native_app_glue.h',
	'Backends/System/Android/Sources/gl3stub.h',
	'Backends/System/Android/Sources/GLContext.cpp',
	'Backends/System/Android/Sources/GLContext.h',
	'Backends/System/Android/Sources/JNIHelper.cpp',
	'Backends/System/Android/Sources/JNIHelper.h',
	'Backends/System/Windows/Sources/d3dx12.h',
	'Backends/Graphics3/OpenGL1/Sources/GL/eglew.h',
	'Backends/Graphics3/OpenGL1/Sources/GL/glew.h',
	'Backends/Graphics3/OpenGL1/Sources/GL/glxew.h',
	'Backends/Graphics3/OpenGL1/Sources/GL/wglew.h',
	'Backends/Graphics4/OpenGL/Sources/GL/eglew.h',
	'Backends/Graphics4/OpenGL/Sources/GL/glew.h',
	'Backends/Graphics4/OpenGL/Sources/GL/glxew.h',
	'Backends/Graphics4/OpenGL/Sources/GL/wglew.h',
	'Backends/Graphics5/Direct3D12/Sources/d3dx12.h',
	'Backends/Graphics5/Vulkan/Sources/vulkan/vk_platform.h',
	'Backends/Graphics5/Vulkan/Sources/vulkan/vulkan.h',
	'Sources/Kore/Audio1/stb_vorbis.c',
	'Sources/Kore/Graphics1/stb_image.h',
	'Sources/Kore/IO/snappy/snappy-c.h',
	'Sources/Kore/IO/snappy/snappy-internal.h',
	'Sources/Kore/IO/snappy/snappy-sinksource.h',
	'Sources/Kore/IO/snappy/snappy-stubs-internal.h',
	'Sources/Kore/IO/snappy/snappy-stubs-public.h',
	'Sources/Kore/IO/snappy/snappy.h',
	'Backends/System/Linux/Sources/kinc/backend/wayland/wayland-protocol.c.h',
	'Backends/System/Linux/Sources/kinc/backend/wayland/wayland-protocol.h',
	'Backends/System/Linux/Sources/kinc/backend/wayland/xdg-decoration.c.h',
	'Backends/System/Linux/Sources/kinc/backend/wayland/xdg-decoration.h',
	'Backends/System/Linux/Sources/kinc/backend/wayland/xdg-shell.c.h',
	'Backends/System/Linux/Sources/kinc/backend/wayland/xdg-shell.h',
];

function isExcluded(filepath) {
	return excludes.indexOf(filepath.replace(/\\/g, '/')) >= 0
	|| filepath.indexOf('Libraries') >= 0
	|| filepath.indexOf('lz4') >= 0
	|| filepath.indexOf('Tools') >= 0
	|| filepath.indexOf('libs') >= 0
	|| filepath.endsWith('.winrt.cpp');
}

function format(dir) {
	const files = fs.readdirSync(dir);
	for (let file of files) {
		if (file.startsWith('.')) continue;
		let filepath = path.join(dir, file);
		let info = fs.statSync(filepath);
		if (info.isDirectory()) {
			format(filepath);
		}
		else {
			if (isExcluded(filepath)) continue;
			if (file.endsWith('.cpp') || file.endsWith('.h') || file.endsWith('.m') || file.endsWith('.mm')) {
				console.log('Format ' + filepath);
				child_process.execFileSync('clang-format', ['-style=file', '-i', filepath]);
			}
		}
	}
}

format('.');
