project = Project.new("Kore")

project:addFile("Sources/**");
project:addIncludeDir("Sources");

function addBackend(name)
	project:addFile("Backends/" .. name .. "/Sources/**")
	project:addIncludeDir("Backends/" .. name .. "/Sources")
end

if platform == Platform.Windows then
	addBackend("Windows")
	
	project:addIncludeDir("Backends/Windows/Libraries/directx/Include")

	if gfx == Graphics.OpenGL then
		addBackend("OpenGL")
		project:addDefine("OPENGL")
	elseif gfx == Graphics.OpenGL2 then
		addBackend("OpenGL2")
		project:addDefine("OPENGL")
	elseif gfx == Graphics.Direct3D11 then
		addBackend("Direct3D11")
		project:addDefine("DIRECT3D")
	else
		addBackend("Direct3D9")
		project:addDefine("DIRECT3D")
	end

	project:addLibsFor("Win32", "Backends/Windows/Libraries/directx/Lib/x86/dxguid", "Backends/Windows/Libraries/directx/Lib/x86/DxErr", "Backends/Windows/Libraries/directx/Lib/x86/dsound", "Backends/Windows/Libraries/directx/Lib/x86/dinput8")
	project:addLibsFor("x64", "Backends/Windows/Libraries/directx/Lib/x64/dxguid", "Backends/Windows/Libraries/directx/Lib/x64/DxErr", "Backends/Windows/Libraries/directx/Lib/x64/dsound")
	if gfx ~= Graphics.OpenGL then
		if gfx == Graphics.Direct3D11 then
			project:addLibFor("Win32", "Backends/Windows/Libraries/directx/Lib/x86/d3d11")
			project:addLibFor("x64", "Backends/Windows/Libraries/directx/Lib/x64/d3d11")
		else
			project:addLibFor("Win32", "Backends/Windows/Libraries/directx/Lib/x86/d3d9")
			project:addLibFor("x64", "Backends/Windows/Libraries/directx/Lib/x64/d3d9")
		end
	end
elseif platform == Platform.WindowsRT then
	addBackend("WindowsRT")
	addBackend("Direct3D11")
elseif platform == Platform.Xbox360 then
	addBackend("Xbox360")
	addBackend("Direct3D9")
	project:addDefine("DIRECT3D")
elseif platform == Platform.PlayStation3 then
	addBackend("PlayStation3")
elseif platform == Platform.OSX then
	addBackend("OSX")
	addBackend("OpenGL2")
	project:addDefine("OPENGL")
	project:addLib("Cocoa")
	project:addLib("AppKit")
	project:addLib("CoreAudio")
	project:addLib("CoreData")
	project:addLib("Foundation")
	project:addLib("OpenGL")
elseif platform == Platform.iOS then
	addBackend("iOS")
	addBackend("OpenGL2")
	project:addDefine("OPENGL")
	project:addLib("UIKit")
	project:addLib("Foundation")
	project:addLib("CoreGraphics")
	project:addLib("QuartzCore")
	project:addLib("OpenGLES")
	project:addLib("CoreAudio")
	project:addLib("AudioToolbox")
	project:addLib("CoreMotion")
elseif platform == Platform.Android then
	addBackend("Android")
	addBackend("OpenGL2")
	project:addDefine("OPENGL")
elseif platform == Platform.HTML5 then
	addBackend("HTML5")
	addBackend("OpenGL2")
	project:addExclude("Backends/OpenGL2/Sources/GL/**")
	project:addDefine("OPENGL")
elseif platform == Platform.Linux then
	addBackend("Linux")
	addBackend("OpenGL2")
	project:addDefine("OPENGL")
elseif platform == Platform.Tizen then
	addBackend("Tizen")
	addBackend("OpenGL2")
	project:addExclude("Backends/OpenGL2/Sources/GL/**")
	project:addDefine("OPENGL")
end
