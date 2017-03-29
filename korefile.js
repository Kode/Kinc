var project = new Project('Kore', __dirname);

const g1 = true;
const g2 = true;
const g3 = true;
let g4 = false;
let g5 = false;

project.addFile('Sources/**');
project.addExclude('Sources/Kore/IO/snappy/**');
project.addIncludeDir('Sources');

function addBackend(name) {
	project.addFile('Backends/' + name + '/Sources/**');
	project.addIncludeDir('Backends/' + name + '/Sources');
}

if (platform === Platform.Windows) {
	addBackend('Windows');
	project.addLib('dxguid');
	project.addLib('dsound');
	project.addLib('dinput8');

	project.addDefine('_WINSOCK_DEPRECATED_NO_WARNINGS');
	project.addLib('ws2_32');

	if (graphics === GraphicsApi.OpenGL1) {
		addBackend('OpenGL');
		project.addDefine('OPENGL');
		project.addDefine('OPENGL_1_X');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('OpenGL2');
		project.addDefine('OPENGL');
		project.addDefine('GLEW_STATIC');
	}
	else if (graphics === GraphicsApi.Direct3D11) {
		g4 = true;
		addBackend('Direct3D11');
		project.addDefine('DIRECT3D');
		project.addLib('d3d11');
	}
	else if (graphics === GraphicsApi.Direct3D12) {
		g4 = true;
		g5 = true;
		addBackend('Direct3D12');
		project.addDefine('DIRECT3D');
		project.addLib('dxgi');
		project.addLib('d3d12');
	}
	else if (graphics === GraphicsApi.Vulkan) {
		g4 = true;
		g5 = true;
		addBackend('Vulkan');
		project.addDefine('SYS_VULKAN');
		project.addDefine('VK_USE_PLATFORM_WIN32_KHR');
		project.addLibFor('Win32', 'Backends/Vulkan/Libraries/win32/vulkan-1');
		project.addLibFor('x64', 'Backends/Vulkan/Libraries/win64/vulkan-1');
	}
	else {
		g4 = true;
		addBackend('Direct3D9');
		project.addDefine('DIRECT3D');
		project.addLib('d3d9');
	}
	
	if (vr === VrApi.Oculus) {
		project.addDefine('VR_RIFT');
		project.addLibFor('x64', 'Backends/Windows/Libraries/OculusSDK/Lib/x64/LibOVR');
		project.addLibFor('Win32', 'Backends/Windows/Libraries/OculusSDK/Lib/Win32/LibOVR');
		project.addFile('Backends/Windows/Libraries/OculusSDK/**');
		project.addIncludeDir('Backends/Windows/Libraries/OculusSDK/LibOVR/Include');
		project.addIncludeDir('Backends/Windows/Libraries/OculusSDK/LibOVR/Src');
		project.addIncludeDir('Backends/Windows/Libraries/OculusSDK/LibOVRKernel/Src');
		project.addIncludeDir('Backends/Windows/Libraries/OculusSDK/Logging/include');
	}
}
else if (platform === Platform.WindowsApp) {
	g4 = true;
	addBackend('WindowsApp');
	addBackend('Direct3D11');
}
else if (platform === Platform.Xbox360) {
	g4 = true;
	addBackend('Xbox360');
	addBackend('Direct3D9');
	project.addDefine('DIRECT3D');
}
else if (platform === Platform.PlayStation3) {
	g4 = true;
	addBackend('PlayStation3');
}
else if (platform === Platform.OSX) {
	addBackend('OSX');
	if (graphics === GraphicsApi.Metal) {
		g4 = true;
		g5 = true;
		addBackend('Metal');
		project.addDefine('SYS_METAL');
		project.addLib('Metal');
		project.addLib('MetalKit');
	}
	else if (graphics === GraphicsApi.OpenGL1) {
		addBackend('OpenGL');
		project.addDefine('OPENGL');
		project.addDefine('OPENGL_1_X');
		project.addLib('OpenGL');
	}
	else {
		g4 = true;
		addBackend('OpenGL2');
		project.addDefine('OPENGL');
		project.addLib('OpenGL');
	}
	project.addLib('IOKit');
	project.addLib('Cocoa');
	project.addLib('AppKit');
	project.addLib('CoreAudio');
	project.addLib('CoreData');
	project.addLib('Foundation');
	project.addDefine('SYS_UNIXOID');
}
else if (platform === Platform.iOS || platform === Platform.tvOS) {
	addBackend('iOS');
	if (graphics === GraphicsApi.Metal) {
		g4 = true;
		g5 = true;
		addBackend('Metal');
		project.addDefine('SYS_METAL');
		project.addLib('Metal');
	}
	else {
		g4 = true;
		addBackend('OpenGL2');
		project.addDefine('OPENGL');
		project.addLib('OpenGLES');
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
	project.addDefine('SYS_UNIXOID');
}
else if (platform === Platform.Android) {
	addBackend('Android');
	if (graphics === GraphicsApi.Vulkan) {
		g4 = true;
		g5 = true;
		addBackend('Vulkan');
		project.addDefine('SYS_VULKAN');
	}
	else {
		g4 = true;
		addBackend('OpenGL2');
		project.addDefine('OPENGL');
	}
	project.addDefine('SYS_ANDROID_API=15');
	project.addDefine('SYS_UNIXOID');
	project.addLib('log');
	project.addLib('android');
	project.addLib('EGL');
	project.addLib('GLESv2');
	project.addLib('OpenSLES');
	project.addLib('OpenMAXAL');
}
else if (platform === Platform.HTML5) {
	g4 = true;
	addBackend('HTML5');
	addBackend('OpenGL2');
	project.addExclude('Backends/OpenGL2/Sources/GL/**');
	project.addDefine('OPENGL');
}
else if (platform === Platform.Linux) {
	addBackend('Linux');
	project.addLib('asound');
	project.addLib('dl');
	if (graphics === GraphicsApi.Vulkan) {
		g4 = true;
		g5 = true;
		addBackend('Vulkan');
		project.addLib('vulkan');
		project.addLib('xcb');
		project.addDefine('SYS_VULKAN');
		project.addDefine('VK_USE_PLATFORM_XCB_KHR');
	}
	else {
		g4 = true;
		addBackend('OpenGL2');
		project.addLib('GL');
		project.addLib('X11');
		project.addLib('Xinerama');
		project.addDefine('OPENGL');
	}
	project.addDefine('SYS_UNIXOID');
}
else if (platform === Platform.Pi) {
	g4 = true;
	addBackend('Pi');
	addBackend('OpenGL2');
	project.addExclude('Backends/OpenGL2/Sources/GL/**');
	project.addDefine('OPENGL');
	project.addDefine('SYS_UNIXOID');
	project.addDefine('SYS_PI');
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
	addBackend('Tizen');
	addBackend('OpenGL2');
	project.addExclude('Backends/OpenGL2/Sources/GL/**');
	project.addDefine('OPENGL');
	project.addDefine('SYS_UNIXOID');
}

if (g4) {
	project.addDefine('SYS_G4');
}

if (g5) {
	project.addDefine('SYS_G5');
	addBackend('G4onG5');
}
else {
	project.addExclude('Sources/Kore/Graphics5/**');
}

resolve(project);
