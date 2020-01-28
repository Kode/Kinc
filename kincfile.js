const fs = require('fs');
const path = require('path');

const project = new Project('Kore');

const g1 = true;
project.addDefine('KORE_G1');

const g2 = true;
project.addDefine('KORE_G2');

const g3 = true;
project.addDefine('KORE_G3');

let g4 = false;

let g5 = false;

const a1 = true;
project.addDefine('KORE_A1');

const a2 = true;
project.addDefine('KORE_A2');

let a3 = false;

// Setting lz4x to false adds a BSD 2-Clause licensed component,
// which is a little more restrictive than Kore's zlib license.
const lz4x = true;

project.addFile('Sources/**');
if (lz4x) {
	project.addDefine('KORE_LZ4X');
	project.addExclude('Sources/kinc/io/lz4/**');
}
else {
	project.addExclude('Sources/kinc/libs/lz4x.cpp');	
}
project.addIncludeDir('Sources');

function addBackend(name) {
	project.addFile('Backends/' + name + '/Sources/**');
	project.addIncludeDir('Backends/' + name + '/Sources');
}

let plugin = false;

if (platform === Platform.Windows) {
	project.addDefine('KORE_WINDOWS');
	project.addDefine('KORE_MICROSOFT');
	addBackend('System/Windows');
	addBackend('System/Microsoft');
	project.addLib('dxguid');
	project.addLib('dsound');
	project.addLib('dinput8');

	project.addDefine('_CRT_SECURE_NO_WARNINGS');
	project.addDefine('_WINSOCK_DEPRECATED_NO_WARNINGS');
	project.addLib('ws2_32');
	project.addLib('Winhttp');

	project.addFile('Backends/System/Windows/Libraries/DirectShow/**');
	project.addIncludeDir('Backends/System/Windows/Libraries/DirectShow/BaseClasses');
	project.addLib('strmiids');
	project.addLib('winmm');

	if (graphics === GraphicsApi.OpenGL1) {
		addBackend('Graphics3/OpenGL1');
		project.addDefine('KORE_OPENGL1');
		project.addDefine('GLEW_STATIC');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addDefine('KORE_OPENGL');
		project.addDefine('GLEW_STATIC');
	}
	else if (graphics === GraphicsApi.Direct3D11 || graphics === GraphicsApi.Default) {
		g4 = true;
		addBackend('Graphics4/Direct3D11');
		project.addDefine('KORE_DIRECT3D');
		project.addDefine('KORE_DIRECT3D11');
		project.addLib('d3d11');
	}
	else if (graphics === GraphicsApi.Direct3D12) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Direct3D12');
		project.addDefine('KORE_DIRECT3D');
		project.addDefine('KORE_DIRECT3D12');
		project.addLib('dxgi');
		project.addLib('d3d12');

		if (raytrace === RayTraceApi.DXR) {
			project.addDefine('KORE_DXR');
			project.addIncludeDir('Backends/Graphics5/Direct3D12/Libraries/D3D12Raytracing/Include/');
		}
	}
	else if (graphics === GraphicsApi.Vulkan) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		project.addDefine('KORE_VULKAN');
		project.addDefine('VK_USE_PLATFORM_WIN32_KHR');
		project.addLibFor('Win32', path.join(process.env.VULKAN_SDK, 'Lib32', 'vulkan-1'));
		project.addLibFor('x64', path.join(process.env.VULKAN_SDK, 'Lib', 'vulkan-1'));
		let libs = fs.readdirSync(path.join(process.env.VULKAN_SDK, 'Lib32'));
		for (const lib of libs) {
			if (lib.startsWith('VkLayer_')) {
				project.addLibFor('Win32', path.join(process.env.VULKAN_SDK, 'Lib32', lib.substr(0, lib.length - 4)));
			}
		}
		libs = fs.readdirSync(path.join(process.env.VULKAN_SDK, 'Lib'));
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
		project.addDefine('KORE_DIRECT3D');
		project.addDefine('KORE_DIRECT3D9');
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
		project.addDefine('KORE_VR');
		project.addDefine('KORE_OCULUS');
		project.addLibFor('x64', 'Backends/System/Windows/Libraries/OculusSDK/Lib/x64/LibOVR');
		project.addLibFor('Win32', 'Backends/System/Windows/Libraries/OculusSDK/Lib/Win32/LibOVR');
		//project.addFile('Backends/System/Windows/Libraries/OculusSDK/LibOVRKernel/Src');
		project.addFile('Backends/System/Windows/Libraries/OculusSDK/LibOVRKernel/Src/GL/**');
		project.addIncludeDir('Backends/System/Windows/Libraries/OculusSDK/LibOVR/Include/');
		project.addIncludeDir('Backends/System/Windows/Libraries/OculusSDK/LibOVRKernel/Src/');
	}
	else if (vr === VrApi.SteamVR) {
		project.addDefine('KORE_VR');
		project.addDefine('KORE_STEAMVR');
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
	project.addDefine('KORE_WINDOWSAPP');
	project.addDefine('KORE_MICROSOFT');
	addBackend('System/WindowsApp');
	addBackend('System/Microsoft');
	addBackend('Graphics4/Direct3D11');
	addBackend('Audio2/WASAPI');
	project.addDefine('_CRT_SECURE_NO_WARNINGS');
	
	if (vr === VrApi.Hololens) {
		project.addDefine('KORE_VR');
		project.addDefine('KORE_HOLOLENS');
	}
	else if (vr === VrApi.None) {

	}
	else {
		throw new Error('VR API ' + vr + ' is not available for Windows Universal.');
	}
}
else if (platform === Platform.OSX) {
	project.addDefine('KORE_MACOS');
	addBackend('System/Apple');
	addBackend('System/macOS');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Metal || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Metal');
		project.addDefine('KORE_METAL');
		project.addLib('Metal');
		project.addLib('MetalKit');
	}
	else if (graphics === GraphicsApi.OpenGL1) {
		addBackend('Graphics3/OpenGL1');
		project.addDefine('KORE_OPENGL1');
		project.addLib('OpenGL');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addDefine('KORE_OPENGL');
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
	project.addDefine('KORE_POSIX');
}
else if (platform === Platform.iOS || platform === Platform.tvOS) {
	if (platform === Platform.tvOS) {
		project.addDefine('KORE_TVOS');
	}
	else {
		project.addDefine('KORE_IOS');
	}
	addBackend('System/Apple');
	addBackend('System/iOS');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Metal || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Metal');
		project.addDefine('KORE_METAL');
		project.addLib('Metal');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addDefine('KORE_OPENGL');
		project.addDefine('KORE_OPENGL_ES');
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
	project.addDefine('KORE_POSIX');
}
else if (platform === Platform.Android) {
	project.addDefine('KORE_ANDROID');
	addBackend('System/Android');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Vulkan) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		project.addDefine('KORE_VULKAN');
	}
	else if (graphics === GraphicsApi.OpenGL || graphics === GraphicsApi.Default) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addDefine('KORE_OPENGL');
		project.addDefine('KORE_OPENGL_ES');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Android.');
	}
	project.addDefine('KORE_ANDROID_API=15');
	project.addDefine('KORE_POSIX');
	project.addLib('log');
	project.addLib('android');
	project.addLib('EGL');
	project.addLib('GLESv2');
	project.addLib('OpenSLES');
	project.addLib('OpenMAXAL');
}
else if (platform === Platform.HTML5) {
	g4 = true;
	project.addDefine('KORE_HTML5');
	addBackend('System/HTML5');
	addBackend('Graphics4/OpenGL');
	project.addExclude('Backends/Graphics4/OpenGL/Sources/GL/**');
	project.addDefine('KORE_OPENGL');
	project.addDefine('KORE_OPENGL_ES');
}
else if (platform === Platform.Linux) {
	project.addDefine('KORE_LINUX');
	addBackend('System/Linux');
	addBackend('System/POSIX');
	project.addLib('asound');
	project.addLib('dl');
	if (graphics === GraphicsApi.Vulkan) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		project.addLib('vulkan');
		project.addLib('xcb');
		project.addDefine('KORE_VULKAN');
		project.addDefine('VK_USE_PLATFORM_XCB_KHR');
	}
	else if (graphics === GraphicsApi.OpenGL || graphics === GraphicsApi.Default) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addLib('GL');
		project.addLib('X11');
		project.addLib('Xinerama');
		project.addLib('Xi');
		project.addDefine('KORE_OPENGL');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Linux.');
	}
	project.addDefine('KORE_POSIX');
}
else if (platform === Platform.Pi) {
	g4 = true;
	project.addDefine('KORE_PI');
	addBackend('System/Pi');
	addBackend('System/POSIX');
	addBackend('Graphics4/OpenGL');
	project.addExclude('Backends/Graphics4/OpenGL/Sources/GL/**');
	project.addDefine('KORE_OPENGL');
	project.addDefine('KORE_OPENGL_ES');
	project.addDefine('KORE_POSIX');
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
	project.addDefine('KORE_TIZEN');
	addBackend('System/Tizen');
	addBackend('System/POSIX');
	addBackend('Graphics4/OpenGL');
	project.addExclude('Backends/Graphics4/OpenGL/Sources/GL/**');
	project.addDefine('KORE_OPENGL');
	project.addDefine('KORE_OPENGL_ES');
	project.addDefine('KORE_POSIX');
}
else {
	plugin = true;
	g4 = true;
	g5 = true;
	if (platform === Platform.XboxOne) {
		addBackend('System/Microsoft');
		addBackend('Graphics5/Direct3D12');
		addBackend('Audio2/WASAPI');
		project.addDefine('KORE_MICROSOFT');
		project.addDefine('KORE_DIRECT3D');
		project.addDefine('KORE_DIRECT3D12');
	}
}

if (g4) {
	project.addDefine('KORE_G4');
}
else {
	project.addExclude('Sources/Kore/Graphics4/**');
}

if (g5) {
	project.addDefine('KORE_G5');
	project.addDefine('KORE_G4ONG5');
	addBackend('Graphics4/G4onG5');
}
else {
	project.addDefine('KORE_G5');
	project.addDefine('KORE_G5ONG4');
	addBackend('Graphics5/G5onG4');
}

if (!a3) {
	a3 = true;
	project.addDefine('KORE_A3');
	addBackend('Audio3/A3onA2');
}

if (plugin) {
	let backend = 'Unknown';
	if (platform === Platform.PS4) {
		backend = 'PlayStation4';
	}
	else if (platform === Platform.XboxOne) {
		backend = 'XboxOne';
	}
	else if (platform === Platform.Switch) {
		backend = 'Switch';
	}
	else if (platform === Platform.XboxScarlett) {
		backend = 'XboxScarlett';
	}
	else if (platform === Platform.PS5) {
		backend = 'PlayStation5'
	}
	await project.addProject(path.join(Project.root, 'Backends', backend));
	resolve(project);
}
else {
	resolve(project);
}
