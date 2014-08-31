var child_process = require('child_process');
var os = require('os');
var log = require('./log.js');
var Files = require('./Files.js');
var GraphicsApi = require('./GraphicsApi.js');
var Path = require('./Path.js');
var Paths = require('./Paths.js');
var Project = require('./Project.js');
var Platform = require('./Platform.js');
var Solution = require('./Solution.js');
var VisualStudioVersion = require('./VisualStudioVersion.js');
var ExporterAndroid = require('./ExporterAndroid.js');
var ExporterCodeBlocks = require('./ExporterCodeBlocks.js');
var ExporterEmscripten = require('./ExporterEmscripten.js');
var ExporterTizen = require('./ExporterTizen.js');
var ExporterVisualStudio = require('./ExporterVisualStudio.js');
var ExporterXCode = require('./ExporterXCode.js');
	
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

function escapeRegExp(string) {
	return string.replace(/([.*+?^=!:${}()|\[\]\/\\])/g, "\\$1");
}

String.prototype.replaceAll = function (find, replace) {
	return this.replace(new RegExp(escapeRegExp(find), 'g'), replace);
};

if (!String.prototype.contains) {
	String.prototype.contains = function () {
		return String.prototype.indexOf.apply(this, arguments) !== -1;
	};
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
	if (Project.koreDir.path !== '') {
		if (os.platform() === "linux") {
			var path = Project.koreDir.resolve(Paths.get("Tools", "kfx", "kfx-linux"));
		}
		else if (os.platform() === "win32") {
			var path = Project.koreDir.resolve(Paths.get("Tools", "kfx", "kfx.exe"));
		}
		else {
			var path = Project.koreDir.resolve(Paths.get("Tools", "kfx", "kfx-osx"));
		}
		child_process.spawn(path.toString(), [type, from, to, temp]);
	}
}

function exportKakeProject(from, to, platform) {
	log.info("kakefile found, generating build files.");
	log.info("Generating " + fromPlatform(platform) + " solution");

	var solution = Solution.create(from, platform);
	log.info(".");
	solution.searchFiles();
	log.info(".");
	solution.flatten();
	log.info(".");

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

	log.info(".done.");
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
		log.error("korefile.js not found.");
		return "";
	}
}

exports.run = function (options, loglog, callback) {
	log.set(loglog);
	
	if (options.graphicsApi !== undefined) {
		Options.graphicsApi = options.graphicsApi;
	}
	
	if (options.visualStudioVersion !== undefined) {
		Options.visualStudioVersion = options.visualStudioVersion;
	}
	
	exportProject(Paths.get(options.from), Paths.get(options.to), options.platform);
	callback();
};
