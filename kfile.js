const fs = require('fs');
const path = require('path');

const project = new Project('Kinc');

function addKincDefine(name) {
	project.addDefine('KINC_' + name);
	project.addDefine('KORE_' + name);
}

const g1 = true;
addKincDefine('G1');

const g2 = true;
addKincDefine('G2');

const g3 = true;
addKincDefine('G3');

let g4 = false;

let g5 = false;

const a1 = true;
addKincDefine('A1');

const a2 = true;
addKincDefine('A2');

let a3 = false;

// Setting lz4x to false adds a BSD 2-Clause licensed component,
// which is a little more restrictive than Kinc's zlib license.
const lz4x = true;

project.addFile('Sources/**');

if (Options.kong) {
	project.addKongDir('KongShaders');
}
else {
	project.addFile('GLSLShaders/**');
}

if (lz4x) {
	addKincDefine('LZ4X');
	project.addExclude('Sources/kinc/io/lz4/**');
}
project.addIncludeDir('Sources');

function addBackend(name) {
	project.addFile('Backends/' + name + '/Sources/**');
	project.addIncludeDir('Backends/' + name + '/Sources');
}

let plugin = false;

if (platform === Platform.Windows) {
	addBackend('System/Windows');
	addBackend('System/Microsoft');
	project.addLib('dxguid');
	project.addLib('dsound');
	project.addLib('dinput8');

	project.addDefine('_CRT_SECURE_NO_WARNINGS');
	project.addDefine('_WINSOCK_DEPRECATED_NO_WARNINGS');
	project.addLib('ws2_32');
	project.addLib('Winhttp');

	const directshow = (graphics !== GraphicsApi.Direct3D12 && graphics !== GraphicsApi.Default);
	if (directshow) {
		project.addFile('Backends/System/Windows/Libraries/DirectShow/**');
		project.addIncludeDir('Backends/System/Windows/Libraries/DirectShow/BaseClasses');
		project.addLib('strmiids');
		project.addLib('winmm');
	}
	else {
		project.addDefine('KINC_NO_DIRECTSHOW');
	}

	project.addLib('wbemuuid');

	if (graphics === GraphicsApi.OpenGL1) {
		addBackend('Graphics3/OpenGL1');
		addKincDefine('OPENGL1');
		project.addDefine('GLEW_STATIC');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		addKincDefine('OPENGL');
		project.addDefine('GLEW_STATIC');
	}
	else if (graphics === GraphicsApi.Direct3D11) {
		g4 = true;
		addBackend('Graphics4/Direct3D11');
		addKincDefine('DIRECT3D');
		addKincDefine('DIRECT3D11');
		project.addLib('d3d11');
	}
	else if (graphics === GraphicsApi.Direct3D12 || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Direct3D12');
		addKincDefine('DIRECT3D');
		addKincDefine('DIRECT3D12');
		project.addLib('dxgi');
		project.addLib('d3d12');
	}
	else if (graphics === GraphicsApi.Vulkan) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		addKincDefine('VULKAN');
		project.addDefine('VK_USE_PLATFORM_WIN32_KHR');
		if (!process.env.VULKAN_SDK) {
			throw 'Could not find a Vulkan SDK';
		}
		project.addLibFor('x64', path.join(process.env.VULKAN_SDK, 'Lib', 'vulkan-1'));
		let libs = fs.readdirSync(path.join(process.env.VULKAN_SDK, 'Lib'));
		for (const lib of libs) {
			if (lib.startsWith('VkLayer_')) {
				project.addLibFor('x64', path.join(process.env.VULKAN_SDK, 'Lib', lib.substr(0, lib.length - 4)));
			}
		}
		project.addIncludeDir(path.join(process.env.VULKAN_SDK, 'Include'));
	}
	else if (graphics === GraphicsApi.Direct3D9) {
		g4 = true;
		addBackend('Graphics4/Direct3D9');
		addKincDefine('DIRECT3D');
		addKincDefine('DIRECT3D9');
		project.addLib('d3d9');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Windows.');
	}

	if (audio === AudioApi.DirectSound) {
		addBackend('Audio2/DirectSound');
	}
	else if (audio === AudioApi.WASAPI || audio === AudioApi.Default) {
		addBackend('Audio2/WASAPI');
	}
	else {
		throw new Error('Audio API ' + audio + ' is not available for Windows.');
	}

	if (vr === VrApi.Oculus) {
		addKincDefine('VR');
		addKincDefine('OCULUS');
		project.addLibFor('x64', 'Backends/System/Windows/Libraries/OculusSDK/LibOVR/Lib/Windows/x64/Release/VS2017/LibOVR');
		project.addLibFor('Win32', 'Backends/System/Windows/Libraries/OculusSDK/LibOVR/Lib/Windows/Win32/Release/VS2017/LibOVR');
		project.addFile('Backends/System/Windows/Libraries/OculusSDK/LibOVRKernel/Src/GL/**');
		project.addIncludeDir('Backends/System/Windows/Libraries/OculusSDK/LibOVR/Include/');
		project.addIncludeDir('Backends/System/Windows/Libraries/OculusSDK/LibOVRKernel/Src/');
	}
	else if (vr === VrApi.SteamVR) {
		addKincDefine('VR');
		addKincDefine('STEAMVR');
		project.addDefine('VR_API_PUBLIC');
		project.addFile('Backends/System/Windows/Libraries/SteamVR/src/**');
		project.addIncludeDir('Backends/System/Windows/Libraries/SteamVR/src');
		project.addIncludeDir('Backends/System/Windows/Libraries/SteamVR/src/vrcommon');
		project.addIncludeDir('Backends/System/Windows/Libraries/SteamVR/headers');
	}
	else if (vr === VrApi.None) {

	}
	else {
		throw new Error('VR API ' + vr + ' is not available for Windows.');
	}
}
else if (platform === Platform.WindowsApp) {
	g4 = true;
	addKincDefine('WINDOWSAPP');
	addBackend('System/WindowsApp');
	addBackend('System/Microsoft');
	addBackend('Graphics4/Direct3D11');
	addBackend('Audio2/WASAPI_WinRT');
	project.addDefine('_CRT_SECURE_NO_WARNINGS');
	project.vsdeploy = true;

	if (vr === VrApi.HoloLens) {
		addKincDefine('VR');
		addKincDefine('HOLOLENS');
	}
	else if (vr === VrApi.None) {

	}
	else {
		throw new Error('VR API ' + vr + ' is not available for Windows Universal.');
	}
}
else if (platform === Platform.OSX) {
	addBackend('System/Apple');
	addBackend('System/macOS');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Metal || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Metal');
		addKincDefine('METAL');
		project.addLib('Metal');
		project.addLib('MetalKit');
	}
	else if (graphics === GraphicsApi.OpenGL1) {
		addBackend('Graphics3/OpenGL1');
		addKincDefine('OPENGL1');
		project.addLib('OpenGL');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		addKincDefine('OPENGL');
		project.addLib('OpenGL');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for macOS.');
	}
	project.addLib('IOKit');
	project.addLib('Cocoa');
	project.addLib('AppKit');
	project.addLib('CoreAudio');
	project.addLib('CoreData');
	project.addLib('CoreMedia');
	project.addLib('CoreVideo');
	project.addLib('AVFoundation');
	project.addLib('Foundation');
}
else if (platform === Platform.iOS || platform === Platform.tvOS) {
	if (platform === Platform.tvOS) {
		addKincDefine('TVOS');
	}
	addBackend('System/Apple');
	addBackend('System/iOS');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Metal || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Metal');
		addKincDefine('METAL');
		project.addLib('Metal');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		addKincDefine('OPENGL');
		addKincDefine('OPENGL_ES');
		project.addLib('OpenGLES');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for iOS.');
	}
	project.addLib('UIKit');
	project.addLib('Foundation');
	project.addLib('CoreGraphics');
	project.addLib('QuartzCore');
	project.addLib('CoreAudio');
	project.addLib('AudioToolbox');
	project.addLib('CoreMotion');
	project.addLib('AVFoundation');
	project.addLib('CoreFoundation');
	project.addLib('CoreVideo');
	project.addLib('CoreMedia');
}
else if (platform === Platform.Android) {
	addKincDefine('ANDROID');
	addBackend('System/Android');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Vulkan || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		addKincDefine('VULKAN');
		project.addDefine('VK_USE_PLATFORM_ANDROID_KHR');
		project.addLib('vulkan');
		addKincDefine('ANDROID_API=24');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		addKincDefine('OPENGL');
		addKincDefine('OPENGL_ES');
		addKincDefine('ANDROID_API=19');
		project.addDefine('KINC_EGL');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Android.');
	}
	project.addLib('log');
	project.addLib('android');
	project.addLib('EGL');
	if (Options.kong) {
		project.addLib('GLESv3');
	}
	else {
		project.addLib('GLESv2');
	}
	project.addLib('OpenSLES');
	project.addLib('OpenMAXAL');
}
else if (platform === Platform.Emscripten) {
	addKincDefine('EMSCRIPTEN');
	//project.addLib('websocket.js -sPROXY_POSIX_SOCKETS -sUSE_PTHREADS -sPROXY_TO_PTHREAD');
	addBackend('System/Emscripten');
	project.addLib('USE_GLFW=2');
	if (graphics === GraphicsApi.WebGPU) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/WebGPU');
		addKincDefine('WEBGPU');
	}
	else if (graphics === GraphicsApi.OpenGL || graphics === GraphicsApi.Default) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addExclude('Backends/Graphics4/OpenGL/Sources/GL/**');
		addKincDefine('OPENGL');
		addKincDefine('OPENGL_ES');
		if (Options.kong) {
			project.addLib('USE_WEBGL2=1');
		}
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Emscripten.');
	}
}
else if (platform === Platform.Wasm) {
	addKincDefine('WASM');
	addBackend('System/Wasm');
	project.addIncludeDir('miniClib');
	project.addFile('miniClib/**');
	if (graphics === GraphicsApi.WebGPU) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/WebGPU');
		addKincDefine('WEBGPU');
	}
	else if (graphics === GraphicsApi.OpenGL || graphics === GraphicsApi.Default) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addExclude('Backends/Graphics4/OpenGL/Sources/GL/**');
		addKincDefine('OPENGL');
		addKincDefine('OPENGL_ES');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Wasm.');
	}
}
else if (platform === Platform.Linux || platform === Platform.FreeBSD) {
	if (platform === Platform.FreeBSD) { // TODO
		addKincDefine('LINUX');
	}
	addBackend('System/Linux');
	addBackend('System/POSIX');
	project.addLib('asound');
	project.addLib('dl');

	if (platform === Platform.Linux) {
		project.addLib('udev');

		try {
			if (!fs.existsSync(targetDirectory)) {
				fs.mkdirSync(targetDirectory);
			}
			if (!fs.existsSync(path.join(targetDirectory, 'wayland'))) {
				fs.mkdirSync(path.join(targetDirectory, 'wayland'));
			}
			const waylandDir = path.join(targetDirectory, 'wayland', 'wayland-generated');
			if (!fs.existsSync(waylandDir)) {
				fs.mkdirSync(waylandDir);
			}

			const child_process = require('child_process');

			let good_wayland = false;

			const wayland_call = child_process.spawnSync('wayland-scanner', ['--version'], {encoding: 'utf-8'});
			if (wayland_call.status !== 0) {
				throw 'Could not run wayland-scanner to ask for its version';
			}
			const wayland_version = wayland_call.stderr;

			try {
				const scanner_versions = wayland_version.split(' ')[1].split('.');
				const w_x = parseInt(scanner_versions[0]);
				const w_y = parseInt(scanner_versions[1]);
				const w_z = parseInt(scanner_versions[2]);

				if (w_x > 1) {
					good_wayland = true;
				}
				else if (w_x === 1) {
					if (w_y > 17) {
						good_wayland = true;
					}
					else if (w_y === 17) {
						if (w_z >= 91) {
							good_wayland = true;
						}
					}
				}
			}
			catch (err) {
				log.error('Could not parse wayland-version ' + wayland_version);
			}

			let c_ending = '.c';
			if (good_wayland) {
				c_ending = '.c.h';
			}

			let chfiles = [];

			function wl_protocol(protocol, file) {
				chfiles.push(file);
				const backend_path = path.resolve(waylandDir);
				const protocol_path = path.resolve('/usr/share/wayland-protocols', protocol);
				if (child_process.spawnSync('wayland-scanner', ['private-code', protocol_path, path.resolve(backend_path, file + c_ending)]).status !== 0) {
					throw 'Failed to generate wayland protocol files for' + protocol;
				}
				if (child_process.spawnSync('wayland-scanner', ['client-header', protocol_path, path.resolve(backend_path, file + '.h')]).status !== 0) {
					throw 'Failed to generate wayland protocol header for' + protocol;
				}
			}

			if (child_process.spawnSync('wayland-scanner', ['private-code', '/usr/share/wayland/wayland.xml', path.resolve(waylandDir, 'wayland-protocol' + c_ending)]).status !== 0) {
				throw 'Failed to generate wayland protocol files for /usr/share/wayland/wayland.xml';
			}
			if (child_process.spawnSync('wayland-scanner', ['client-header', '/usr/share/wayland/wayland.xml', path.resolve(waylandDir, 'wayland-protocol.h')]).status !== 0) {
				throw 'Failed to generate wayland protocol header for /usr/share/wayland/wayland.xml';
			}
			wl_protocol('stable/viewporter/viewporter.xml', 'wayland-viewporter');
			wl_protocol('stable/xdg-shell/xdg-shell.xml', 'xdg-shell');
			wl_protocol('unstable/xdg-decoration/xdg-decoration-unstable-v1.xml', 'xdg-decoration');
			wl_protocol('unstable/tablet/tablet-unstable-v2.xml', 'wayland-tablet');
			wl_protocol('unstable/pointer-constraints/pointer-constraints-unstable-v1.xml', 'wayland-pointer-constraint');
			wl_protocol('unstable/relative-pointer/relative-pointer-unstable-v1.xml', 'wayland-relative-pointer');

			if (good_wayland) {
				let cfile = '#include "wayland-protocol.c.h"\n';
				for (const chfile of chfiles) {
					cfile += '#include "' + chfile + '.c.h"\n';
				}
				fs.writeFileSync(path.resolve(waylandDir, 'waylandunit.c'), cfile);
			}

			project.addIncludeDir(path.join(targetDirectory, 'wayland'));
			project.addFile(path.resolve(waylandDir, '**'));
		}
		catch (err) {
			log.error('Failed to include wayland-support, setting KINC_NO_WAYLAND.');
			log.error('Wayland error was: ' + err);
			project.addDefine('KINC_NO_WAYLAND');
		}
	}
	else if (platform === Platform.FreeBSD) {
		addBackend('System/FreeBSD');
		project.addExclude('Backends/System/Linux/Sources/kinc/backend/input/gamepad.cpp');
		project.addExclude('Backends/System/Linux/Sources/kinc/backend/input/gamepad.h');
		project.addDefine('KINC_NO_WAYLAND');
	}

	if (graphics === GraphicsApi.Vulkan || (platform === Platform.Linux && graphics === GraphicsApi.Default)) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		project.addLib('vulkan');
		addKincDefine('VULKAN');
	}
	else if (graphics === GraphicsApi.OpenGL || (platform === Platform.FreeBSD && graphics === GraphicsApi.Default)) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addExclude('Backends/Graphics4/OpenGL/Sources/GL/glew.c');
		project.addLib('GL');
		addKincDefine('OPENGL');
		project.addLib('EGL');
		project.addDefine('KINC_EGL');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Linux.');
	}
	if (platform === Platform.FreeBSD) { // TODO
		addKincDefine('POSIX');
	}
	project.addDefine('_POSIX_C_SOURCE=200112L');
	project.addDefine('_XOPEN_SOURCE=600');
}
else if (platform === Platform.Pi) {
	g4 = true;
	addKincDefine('RASPBERRY_PI');
	addBackend('System/Pi');
	addBackend('System/POSIX');
	addBackend('Graphics4/OpenGL');
	project.addExclude('Backends/Graphics4/OpenGL/Sources/GL/**');
	addKincDefine('OPENGL');
	addKincDefine('OPENGL_ES');
	addKincDefine('POSIX');
	project.addDefine('_POSIX_C_SOURCE=200112L');
	project.addDefine('_XOPEN_SOURCE=600');
	project.addIncludeDir('/opt/vc/include');
	project.addIncludeDir('/opt/vc/include/interface/vcos/pthreads');
	project.addIncludeDir('/opt/vc/include/interface/vmcs_host/linux');
	project.addLib('dl');
	project.addLib('GLESv2');
	project.addLib('EGL');
	project.addLib('bcm_host');
	project.addLib('asound');
	project.addLib('X11');
}
else if (platform === Platform.Tizen) {
	g4 = true;
	addKincDefine('TIZEN');
	addBackend('System/Tizen');
	addBackend('System/POSIX');
	addBackend('Graphics4/OpenGL');
	project.addExclude('Backends/Graphics4/OpenGL/Sources/GL/**');
	addKincDefine('OPENGL');
	addKincDefine('OPENGL_ES');
	addKincDefine('POSIX');
}
else {
	plugin = true;
	g4 = true;
	g5 = true;
}

if (g4) {
	addKincDefine('G4');
}
else {
	project.addExclude('Sources/Kore/Graphics4/**');
	project.addExclude('Sources/kinc/graphics4/**');
}

if (g5) {
	addKincDefine('G5');
	addKincDefine('G4ONG5');
	addBackend('Graphics4/G4onG5');
}
else {
	addKincDefine('G5');
	addKincDefine('G5ONG4');
	addBackend('Graphics5/G5onG4');
}

if (!a3) {
	if (cpp) {
		a3 = true;
		addKincDefine('A3');
		addBackend('Audio3/A3onA2');
	}
}

project.setDebugDir('Deployment');
project.kincProcessed = true;

if (plugin) {
	let backend = 'Unknown';
	if (platform === Platform.PS4) {
		backend = 'PlayStation4';
	}
	else if (platform === Platform.XboxOne) {
		backend = 'Xbox';
	}
	else if (platform === Platform.Switch) {
		backend = 'Switch';
	}
	else if (platform === Platform.XboxSeries) {
		backend = 'Xbox';
	}
	else if (platform === Platform.PS5) {
		backend = 'PlayStation5'
	}

	let ancestor = project;
	while (ancestor.parent != null) {
		ancestor = ancestor.parent;
	}

	let pluginPath = path.join(ancestor.basedir, 'Backends', backend);
	if (!fs.existsSync(pluginPath)) {
		pluginPath = path.join(__dirname, '..', 'Backends', backend);
	}
	if (!fs.existsSync(pluginPath)) {
		log.error('Was looking for a backend in ' + pluginPath + ' but it wasn\'t there.');
		throw 'backend not found';
	}
	await project.addProject(pluginPath);
	resolve(project);
}
else {
	resolve(project);
}
