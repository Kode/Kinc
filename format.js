const child_process = require('child_process');
const fs = require('fs');
const path = require('path');

const excludes = [
	'Backends/Graphics3/OpenGL1/Sources/GL',
	'Backends/Graphics4/OpenGL/Sources/GL',
	'Backends/Graphics5/Direct3D12/Sources/d3dx12.h',
	'Backends/Graphics5/Direct3D12/pix',
	'Backends/Graphics5/Vulkan/Sources/vulkan/vk_platform.h',
	'Backends/Graphics5/Vulkan/Sources/vulkan/vulkan.h',
	'Backends/System/Android/Sources/Android',
	'Backends/System/Linux/Sources/kinc/backend/wayland',
	'Sources/kope/util/offalloc',
	'Tests/Shader/Sources/stb_image_write.h'
];

function excludeMatches(filepath) {
	for (const exclude of excludes) {
		if (filepath.startsWith(exclude)) {
			return true;
		}
	}
	return false;
}

function isExcluded(filepath) {
	filepath = filepath.replace(/\\/g, '/');
	return excludeMatches(filepath)
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
			if (file.endsWith('.c') || file.endsWith('.cpp') || file.endsWith('.h') || file.endsWith('.m') || file.endsWith('.mm')) {
				console.log('Format ' + filepath);
				child_process.execFileSync('clang-format', ['-style=file', '-i', filepath]);
			}
		}
	}
}

format('.');
