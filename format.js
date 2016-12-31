const child_process = require('child_process');
const fs = require('fs');
const path = require('path');

const excludes = [
	'Backends/Android/Sources/android_native_app_glue.h',
	'Backends/Android/Sources/gl3stub.h',
	'Backends/Android/Sources/GLContext.cpp',
	'Backends/Android/Sources/GLContext.h',
	'Backends/Android/Sources/JNIHelper.cpp',
	'Backends/Android/Sources/JNIHelper.h',
	'Backends/OpenGL2/Sources/GL/eglew.h',
	'Backends/OpenGL2/Sources/GL/glew.h',
	'Backends/OpenGL2/Sources/GL/glxew.h',
	'Backends/OpenGL2/Sources/GL/wglew.h',
	'Backends/Direct3D12/Sources/Kore/d3dx12.h',
	'Backends/Vulkan/Sources/vulkan/vk_platform.h',
	'Backends/Vulkan/Sources/vulkan/vulkan.h',
	'Sources/Kore/Audio/stb_vorbis.cpp',
	'Sources/Kore/Audio/stb_vorbis.h',
	'Sources/Kore/Graphics/stb_image.h',
	'Sources/Kore/IO/snappy/snappy-c.h',
	'Sources/Kore/IO/snappy/snappy-internal.h',
	'Sources/Kore/IO/snappy/snappy-sinksource.h',
	'Sources/Kore/IO/snappy/snappy-stubs-internal.h',
	'Sources/Kore/IO/snappy/snappy-stubs-public.h',
	'Sources/Kore/IO/snappy/snappy.h'
];

function isExcluded(filepath) {
	filepath = filepath.replace(/\\/g, '/');
	for (let exclude of excludes) {
		if (filepath === exclude) return true;
	}
	return false;
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
