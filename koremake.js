var os = require('os');
var Options = require('./Options.js');
var Platform = require('./Platform.js');
var GraphicsApi = require('./GraphicsApi.js');
var VisualStudioVersion = require('./VisualStudioVersion.js');

if (!String.prototype.startsWith) {
	Object.defineProperty(String.prototype, 'startsWith', {
		enumerable: false,
		configurable: false,
		writable: false,
		value: function (searchString, position) {
			position = position || 0;
			return this.indexOf(searchString, position) === position;
		}
	});
}

if (!String.prototype.endsWith) {
	Object.defineProperty(String.prototype, 'endsWith', {
		enumerable: false,
		configurable: false,
		writable: false,
		value: function (searchString, position) {
			position = position || this.length;
			position = position - searchString.length;
			var lastIndex = this.lastIndexOf(searchString);
			return lastIndex !== -1 && lastIndex === position;
		}
	});
}

var args = process.argv;

var from = '.';
var to = 'build';
var gfx = GraphicsApi.Direct3D9;
var vs = VisualStudioVersion.VS2013;
var compile = false;
var run = false;

if (os.platform() === "linux") {
	var platform = Platform.Linux;
}
else if (os.platform() === "win32") {
	var platform = Platform.Windows;
}
else {
	var platform = Platform.OSX;
}

for (var i = 2; i < args.length; ++i) {
	var arg = args[i];
	
	if (arg === "pch") Options.setPrecompiledHeaders(true);
	else if (arg === 'compile') compile = true;
	else if (arg === 'run') {
		compile = true;
		run = true;
	}
	else if (arg.startsWith("intermediate=")) Options.setIntermediateDrive(arg.substr(13));
	else if (arg.startsWith("gfx=")) gfx = arg.substr(4);
	else if (arg.startsWith("vs=")) vs = arg.substr(3);

	else if (arg.startsWith("from=")) from = arg.substr(5);
	else if (arg.startsWith("to=")) to = arg.substr(3);

	else {
		platform = arg;
	}
}

require('./main.js').run(
{
	from: from,
	to: to,
	platform: platform,
	graphicsApi: gfx,
	visualStudioVersion: vs,
	compile: compile,
	run: run
},
{
	info: console.log,
	error: console.log
}, function () { });
