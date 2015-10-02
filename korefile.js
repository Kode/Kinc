var project = new Project('Kore');

project.addFile('Sources/**');
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

	if (graphics === GraphicsApi.OpenGL) {
		addBackend('OpenGL');
		project.addDefine('OPENGL');
	}
	else if (graphics === GraphicsApi.OpenGL2) {
		addBackend('OpenGL2');
		project.addDefine('OPENGL');
	}
	else if (graphics === GraphicsApi.Direct3D11) {
		addBackend('Direct3D11');
		project.addDefine('DIRECT3D');
	}
	else if (graphics === GraphicsApi.Direct3D12) {
		addBackend('Direct3D12');
		project.addDefine('DIRECT3D');
		project.addLib('dxgi');
	}
	else {
		addBackend('Direct3D9');
		project.addDefine('DIRECT3D');
	}

	if (graphics !== GraphicsApi.OpenGL) {
		if (graphics === GraphicsApi.Direct3D12) {
			project.addLib('d3d12');
		}
		else if (graphics === GraphicsApi.Direct3D11) {
			project.addLib('d3d11');
		}
		else {
			project.addLib('d3d9');
		}
	}
}
else if (platform === Platform.WindowsApp) {
	addBackend('WindowsApp');
	addBackend('Direct3D11');
}
else if (platform === Platform.Xbox360) {
	addBackend('Xbox360');
	addBackend('Direct3D9');
	project.addDefine('DIRECT3D');
}
else if (platform === Platform.PlayStation3) {
	addBackend('PlayStation3');
}
else if (platform === Platform.OSX) {
	addBackend('OSX');
	if (graphics === GraphicsApi.Metal) {
		addBackend('Metal');
		project.addDefine('SYS_METAL');
		project.addLib('Metal');
	}
	else {
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
else if (platform === Platform.iOS) {
	addBackend('iOS');
	if (graphics === GraphicsApi.Metal) {
		addBackend('Metal');
		project.addDefine('SYS_METAL');
		project.addLib('Metal');
	}
	else {
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
	addBackend('OpenGL2');
	project.addDefine('OPENGL');
	project.addDefine('SYS_ANDROID_API=15');
	project.addDefine('HXCPP_ANDROID_PLATFORM=23');
	project.addDefine('SYS_UNIXOID');
}
else if (platform === Platform.HTML5) {
	addBackend('HTML5');
	addBackend('OpenGL2');
	project.addExclude('Backends/OpenGL2/Sources/GL/**');
	project.addDefine('OPENGL');
}
else if (platform === Platform.Linux) {
	addBackend('Linux');
	addBackend('OpenGL2');
	project.addDefine('OPENGL');
	project.addDefine('SYS_UNIXOID');
}
else if (platform === Platform.Tizen) {
	addBackend('Tizen');
	addBackend('OpenGL2');
	project.addExclude('Backends/OpenGL2/Sources/GL/**');
	project.addDefine('OPENGL');
	project.addDefine('SYS_UNIXOID');
}

return project;
