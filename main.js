var child_process = require('child_process');
var os = require('os');
var Files = require('./Files.js');
var GraphicsApi = require('./GraphicsApi.js');
var Path = require('./Path.js');
var Paths = require('./Paths.js');
var Platform = require('./Platform.js');
var Solution = require('./Solution.js');
var ExporterVisualStudio = require('./ExporterVisualStudio.js');
	
var koreDir = new Path('');

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

function fromPlatform(platform) {
	switch (platform) {
		case Platform.Windows:
			return "Windows";
		case Platform.WindowsRT:
			return "WindowsRT";
		case Platform.PlayStation3:
			return "PlayStation 3";
		case Platform.iOS:
			return "iOS";
		case Platform.OSX:
			return "OS X";
		case Platform.Android:
			return "Android";
		case Platform.Xbox360:
			return "Xbox 360";
		case Platform.Linux:
			return "Linux";
		case Platform.HTML5:
			return "HTML5";
		case Platform.Tizen:
			return "Tizen";
		default:
			return "unknown";
	}
}

function shaderLang(platform) {
	switch (platform) {
		case Platform.Windows:
			switch (Options.getGraphicsApi()) {
				case GraphicsApi.OpenGL:
				case GraphicsApi.OpenGL2:
					return "glsl";
				case GraphicsApi.Direct3D9:
					return "d3d9";
				case GraphicsApi.Direct3D11:
					return "d3d11";
				default:
					return "d3d9";
			}
		case Platform.WindowsRT:
			return "d3d11";
		case Platform.PlayStation3:
			return "d3d9";
		case Platform.iOS:
			return "essl";
		case Platform.OSX:
			return "glsl";
		case Platform.Android:
			return "essl";
		case Platform.Xbox360:
			return "d3d9";
		case Platform.Linux:
			return "glsl";
		case Platform.HTML5:
			return "essl";
		case Platform.Tizen:
			return "essl";
		default:
			return "unknown";
	}
}

function compileShader(type, from, to, temp) {
	if (koreDir.path !== '') {
		if (os.platform() === "linux") {
			var path = koreDir.resolve(Paths.get("Tools", "kfx", "kfx-linux"));
		}
		else if (os.platform() === "win32") {
			var path = koreDir.resolve(Paths.get("Tools", "kfx", "kfx.exe"));
		}
		else {
			var path = koreDir.resolve(Paths.get("Tools", "kfx", "kfx-osx"));
		}
		child_process.spawn(path.toString(), [type, from, to, temp]);
	}
}

function exportKakeProject(from, to, platform) {
	console.log("kakefile.js found, generating build files.");
	console.log("Generating " + fromPlatform(platform) + " solution");

	var solution = Solution.create(from, platform);
	console.log(".");
	solution.searchFiles();
	console.log(".");
	solution.flatten();
	console.log(".");

	if (!Files.exists(to)) Files.createDirectories(to);

	var project = solution.getProjects()[0];
	var files = project.getFiles();
	for (f in files) {
		var file = files[f];
		if (file.endsWith(".glsl")) {
			var outfile = file;
			var index = outfile.lastIndexOf('/');
			if (index > 0) outfile = outfile.substr(index);
			outfile = outfile.substr(0, outfile.size() - 5);
			compileShader(shaderLang(platform), file, project.getDebugDir() + "/" + outfile, "build");
		}
	}

	var exporter = null;
	if (platform == Platform.iOS || platform == Platform.OSX) exporter = new ExporterXCode();
	else if (platform == Platform.Android) exporter = new ExporterAndroid();
	else if (platform == Platform.HTML5) exporter = new ExporterEmscripten();
	else if (platform == Platform.Linux) exporter = new ExporterCodeBlocks();
	else if (platform == Platform.Tizen) exporter = new ExporterTizen();
	else exporter = new ExporterVisualStudio();
	exporter.exportSolution(solution, from, to, platform);

	console.log(".done.");
	return solution.getName();
}

function isKakeProject(directory) {
	return Files.exists(directory.resolve("korefile.js"));
}

function exportProject(from, to, platform) {
	if (isKakeProject(from)) {
		return exportKakeProject(from, to, platform);
	}
	else {
		console.log("korefile.js not found.");
		return "";
	}
}

exports.main = function () {
	var args = process.argv;

	var from = '.';
	var to = 'build';

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

		if (arg == "pch") Options.setPrecompiledHeaders(true);
		else if (startsWith(arg, "intermediate=")) Options.setIntermediateDrive(arg.substr(13));
		else if (startsWith(arg, "gfx=")) Options.setGraphicsApi(arg.substr(4));
		else if (startsWith(arg, "vs=")) Options.setVisualStudioVersion(arg.substr(3));

		else if (startsWith(arg, "from=")) from = arg.substr(5);
		else if (startsWith(arg, "to=")) to = arg.substr(3);

		else {
			for (p in Platform) {
				if (arg === Platform[p]) {
					platform = Platform[p];
				}
			}
		}
	}
	exportProject(Paths.get(from), Paths.get(to), platform);
};
