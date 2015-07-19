var child_process = require('child_process');
var fs = require('fs');
var os = require('os');
var path = require('path');
var log = require('./log.js');
var Files = require('./Files.js');
var GraphicsApi = require('./GraphicsApi.js');
var Options = require('./Options.js');
var Paths = require('./Paths.js');
var Project = require('./Project.js');
var Platform = require('./Platform.js');
var Solution = require('./Solution.js');
var exec = require('./exec.js');
var VisualStudioVersion = require('./VisualStudioVersion.js');
var ExporterAndroid = require('./ExporterAndroid.js');
var ExporterCodeBlocks = require('./ExporterCodeBlocks.js');
var ExporterMakefile = require('./ExporterMakefile.js');
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
		case Platform.WindowsApp:
			return "Windows App";
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
			switch (Options.graphicsApi) {
				case GraphicsApi.OpenGL:
				case GraphicsApi.OpenGL2:
					return "glsl";
				case GraphicsApi.Direct3D9:
					return "d3d9";
				case GraphicsApi.Direct3D11:
					return "d3d11";
				case GraphicsApi.Direct3D12:
					return 'd3d12';
				default:
					return "d3d9";
			}
		case Platform.WindowsApp:
			return "d3d11";
		case Platform.PlayStation3:
			return "d3d9";
		case Platform.iOS:
			switch (Options.graphicsApi) {
				case GraphicsApi.Metal:
					return 'metal';
				default:
					return 'essl';
			}
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
			return platform;
	}
}

function compileShader(projectDir, type, from, to, temp, platform) {
	var compiler = '';
	
	if (Project.koreDir.path !== '') {
		compiler = Project.koreDir.resolve(Paths.get("Tools", "krafix", "krafix" + exec.sys())).toString();
	}

	if (fs.existsSync(path.join(projectDir.toString(), 'Backends'))) {
		var libdirs = fs.readdirSync(path.join(projectDir.toString(), 'Backends'));
		for (var ld in libdirs) {
			var libdir = path.join(projectDir.toString(), 'Backends', libdirs[ld]);
			if (fs.statSync(libdir).isDirectory()) {
				var exe = path.join(libdir, 'krafix', 'krafix-' + platform + '.exe');
				if (fs.existsSync(exe)) {
					compiler = exe;
				}
			}
		}
	}

	if (compiler !== '') {
		child_process.spawnSync(compiler, [type, from, to, temp, platform]);
	}
}

function exportKakeProject(from, to, platform, options) {
	log.info("korefile found, generating build files.");
	log.info("Generating " + fromPlatform(platform) + " solution.");

	var solution = Solution.create(from, platform);
	solution.searchFiles();
	solution.flatten();

	if (!Files.exists(to)) Files.createDirectories(to);

	var project = solution.getProjects()[0];
	var files = project.getFiles();
	for (var f in files) {
		var file = files[f];
		if (file.endsWith(".glsl")) {
			var outfile = file;
			var index = outfile.lastIndexOf('/');
			if (index > 0) outfile = outfile.substr(index);
			outfile = outfile.substr(0, outfile.length - 5);
			compileShader(from, shaderLang(platform), file, path.join(project.getDebugDir(), outfile), "build", platform);
		}
	}

	var exporter = null;
	if (platform == Platform.iOS || platform == Platform.OSX) exporter = new ExporterXCode();
	else if (platform == Platform.Android) exporter = new ExporterAndroid();
	else if (platform == Platform.HTML5) exporter = new ExporterEmscripten();
	else if (platform == Platform.Linux) {
		if (options.compile) exporter = new ExporterMakefile();
		else exporter = new ExporterCodeBlocks();
	}
	else if (platform == Platform.Tizen) exporter = new ExporterTizen();
	else {
		var found = false;
		for (var p in Platform) {
			if (platform === Platform[p]) {
				found = true;
				break;
			}
		}
		if (found) {
			exporter = new ExporterVisualStudio();
		}
		else {
			var libdirs = fs.readdirSync(path.join(from.toString(), 'Backends'));
			for (var ld in libdirs) {
				var libdir = libdirs[ld];
				if (fs.statSync(path.join(from.toString(), 'Backends', libdir)).isDirectory()) {
					var libfiles = fs.readdirSync(path.join(from.toString(), 'Backends', libdir));
					for (var lf in libfiles) {
						var libfile = libfiles[lf];
						if (libfile.startsWith('Exporter') && libfile.endsWith('.js')) {
							var Exporter = require(path.relative(__dirname, path.join(from.toString(), 'Backends', libdir, libfile)));
							exporter = new Exporter();
							break;
						}
					}
				}
			}
		}
	}
	exporter.exportSolution(solution, from, to, platform, options.vrApi);

	return solution.getName();
}

function isKakeProject(directory) {
	return Files.exists(directory.resolve("korefile.js"));
}

function exportProject(from, to, platform, options) {
	if (isKakeProject(from)) {
		return exportKakeProject(from, to, platform, options);
	}
	else {
		log.error("korefile.js not found.");
		return "";
	}
}

exports.api = 1;

exports.run = function (options, loglog, callback) {
	log.set(loglog);
	
	if (options.graphicsApi !== undefined) {
		Options.graphicsApi = options.graphicsApi;
	}
	
	if (options.visualStudioVersion !== undefined) {
		Options.visualStudioVersion = options.visualStudioVersion;
	}
	
	if (options.vrApi !== undefined) {
    	Options.vrApi = options.vrApi;
	}
	
	var solutionName = exportProject(Paths.get(options.from), Paths.get(options.to), options.platform, options);

	if (options.compile && solutionName != "") {
		log.info('Compiling...');

		var make = null;

		if (options.platform === Platform.Linux) {
			make = child_process.spawn('make', [], { cwd: options.to });
		}
		else if (options.platform == Platform.OSX) {
			make = child_process.spawn('xcodebuild', ['-project', solutionName + '.xcodeproj'], { cwd: options.to });
		}

		if (make != null) {
			make.stdout.on('data', function (data) {
				log.info(data.toString());
			});

			make.stderr.on('data', function (data) {
				log.error(data.toString());
			});

			make.on('close', function (code) {
				if (options.run) {
					if (options.platform == Platform.OSX) {
						child_process.spawn('open', ['build/Release/' + solutionName + '.app/Contents/MacOS/' + solutionName], { cwd: options.to });
					}
					else {
						log.info('--run not yet implemented for this platform');
					}
				}

				callback();
			});
		}
		else {
			log.info('--compile not yet implemented for this platform');
			callback();
		}
	}
	else callback();
};

