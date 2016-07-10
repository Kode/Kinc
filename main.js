"use strict";

const child_process = require('child_process');
const fs = require('fs-extra');
const os = require('os');
const path = require('path');
const log = require('./log.js');
const Files = require('./Files.js');
const GraphicsApi = require('./GraphicsApi.js');
const Options = require('./Options.js');
const Paths = require('./Paths.js');
const Project = require('./Project.js');
const Platform = require('./Platform.js');
const Solution = require('./Solution.js');
const exec = require('./exec.js');
const VisualStudioVersion = require('./VisualStudioVersion.js');
const ExporterAndroid = require('./ExporterAndroid.js');
const ExporterCodeBlocks = require('./ExporterCodeBlocks.js');
const ExporterMakefile = require('./ExporterMakefile.js');
const ExporterEmscripten = require('./ExporterEmscripten.js');
const ExporterTizen = require('./ExporterTizen.js');
const ExporterVisualStudio = require('./ExporterVisualStudio.js');
const ExporterXCode = require('./ExporterXCode.js');

function escapeRegExp(string) {
	return string.replace(/([.*+?^=!:${}()|\[\]\/\\])/g, "\\$1");
}

String.prototype.replaceAll = function (find, replace) {
	return this.replace(new RegExp(escapeRegExp(find), 'g'), replace);
};

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
		case Platform.Pi:
			return "Pi";
		case Platform.tvOS:
			return "tvOS";
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
					return 'd3d11';
				case GraphicsApi.Vulkan:
					return 'spirv';
				default:
					return "d3d9";
			}
		case Platform.WindowsApp:
			return "d3d11";
		case Platform.PlayStation3:
			return "d3d9";
		case Platform.iOS:
		case Platform.tvOS:
			switch (Options.graphicsApi) {
				case GraphicsApi.Metal:
					return 'metal';
				default:
					return 'essl';
			}
		case Platform.OSX:
			switch (Options.graphicsApi) {
				case GraphicsApi.Metal:
					return 'metal';
				default:
					return 'glsl';
			}
		case Platform.Android:
			switch (Options.graphicsApi) {
				case GraphicsApi.Vulkan:
					return 'spirv';
				default:
					return 'essl';
			}
		case Platform.Xbox360:
			return "d3d9";
		case Platform.Linux:
			switch (Options.graphicsApi) {
				case GraphicsApi.Vulkan:
					return 'spirv';
				default:
					return 'glsl';
			}
		case Platform.HTML5:
			return "essl";
		case Platform.Tizen:
			return "essl";
		case Platform.Pi:
			return "essl";
		default:
			return platform;
	}
}

function compileShader(projectDir, type, from, to, temp, platform, nokrafix) {
	let compiler = '';
	
	if (Project.koreDir.path !== '') {
		if (nokrafix) {
			compiler = Project.koreDir.resolve(Paths.get("Tools", "kfx", "kfx" + exec.sys())).toString();
		}
		else {
			compiler = Project.koreDir.resolve(Paths.get("Tools", "krafix", "krafix" + exec.sys())).toString();
		}
	}

	if (fs.existsSync(path.join(projectDir.toString(), 'Backends'))) {
		let libdirs = fs.readdirSync(path.join(projectDir.toString(), 'Backends'));
		for (let ld in libdirs) {
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

function exportKoremakeProject(from, to, platform, options) {
	log.info('korefile found.');
	log.info('Creating ' + fromPlatform(platform) + ' project files.');

	let solution = Solution.create(from, platform);
	solution.searchFiles();
	solution.flatten();

	if (!Files.exists(to)) Files.createDirectories(to);

	let project = solution.getProjects()[0];
	let files = project.getFiles();
	for (let file of files) {
		if (file.file.endsWith(".glsl")) {
			let outfile = file.file;
			const index = outfile.lastIndexOf('/');
			if (index > 0) outfile = outfile.substr(index);
			outfile = outfile.substr(0, outfile.length - 5);
			compileShader(from, shaderLang(platform), file.file, path.join(project.getDebugDir(), outfile), "build", platform, options.nokrafix);
		}
	}

	let exporter = null;
	if (platform === Platform.iOS || platform === Platform.OSX || platform === Platform.tvOS) exporter = new ExporterXCode();
	else if (platform == Platform.Android) exporter = new ExporterAndroid();
	else if (platform == Platform.HTML5) exporter = new ExporterEmscripten();
	else if (platform == Platform.Linux || platform === Platform.Pi) {
		if (options.compile) exporter = new ExporterMakefile();
		else exporter = new ExporterCodeBlocks();
	}
	else if (platform == Platform.Tizen) exporter = new ExporterTizen();
	else {
		let found = false;
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
			let libdirs = fs.readdirSync(path.join(from.toString(), 'Backends'));
			for (let libdir of libdirs) {
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
	exporter.exportSolution(solution, from, to, platform, options.vrApi, options.nokrafix, options);

	return solution;
}

function isKoremakeProject(directory) {
	return Files.exists(directory.resolve("korefile.js"));
}

function exportProject(from, to, platform, options) {
	if (isKoremakeProject(from)) {
		return exportKoremakeProject(from, to, platform, options);
	}
	else {
		log.error("korefile.js not found.");
		return null;
	}
}

exports.api = 1;

exports.run = function (options, loglog, callback) {
	log.set(loglog);
	
	if (options.graphics !== undefined) {
		Options.graphicsApi = options.graphics;
	}
	
	if (options.visualstudio !== undefined) {
		Options.visualStudioVersion = options.visualstudio;	
	}
	
	if (options.vr != undefined) {
		Options.vrApi = options.vr;
	}
	
	let solution = exportProject(Paths.get(options.from), Paths.get(options.to), options.target, options);
	let project = solution.getProjects()[0];
	let solutionName = solution.getName();
	
	if (options.compile && solutionName != "") {
		log.info('Compiling...');

		let make = null;

		if (options.target === Platform.Linux) {
			make = child_process.spawn('make', [], { cwd: options.to });
		}
		else if (options.target === Platform.OSX) {
			make = child_process.spawn('xcodebuild', ['-project', solutionName + '.xcodeproj'], { cwd: options.to });
		}
		else if (options.target === Platform.Windows) {
			let vsvars = null;
			if (process.env.VS140COMNTOOLS) {
				vsvars = process.env.VS140COMNTOOLS + '\\vsvars32.bat';
			}
			else if (process.env.VS120COMNTOOLS) {
				vsvars = process.env.VS120COMNTOOLS + '\\vsvars32.bat';
			}
			else if (process.env.VS110COMNTOOLS) {
				vsvars = process.env.VS110COMNTOOLS + '\\vsvars32.bat';
			}
			if (vsvars !== null) {
				fs.writeFileSync(path.join(options.to, 'build.bat'), '@call "' + vsvars + '"\n' + '@MSBuild.exe ' + solutionName + '.vcxproj /m /p:Configuration=Debug,Platform=Win32');
				make = child_process.spawn('build.bat', {cwd: options.to});
			}
			else {
				log.error('Visual Studio not found.');
			}
		}

		if (make !== null) {
			make.stdout.on('data', function (data) {
				log.info(data.toString());
			});

			make.stderr.on('data', function (data) {
				log.error(data.toString());
			});

			make.on('close', function (code) {
				if (code === 0) {
					if (options.target === Platform.Linux) {
						fs.copySync(path.join(options.to.toString(), solutionName), path.join(options.from.toString(), project.getDebugDir(), solutionName));
					}
					else if (options.target === Platform.Windows) {
						fs.copySync(path.join(options.to.toString(), 'Debug', solutionName + '.exe'), path.join(options.from.toString(), project.getDebugDir(), solutionName + '.exe'));
					}
					if (options.run) {
						if (options.target === Platform.OSX) {
							child_process.spawn('open', ['build/Release/' + solutionName + '.app/Contents/MacOS/' + solutionName], {stdio: 'inherit', cwd: options.to});
						}
						else if (options.target === Platform.Linux || options.target === Platform.Windows) {
							child_process.spawn(path.resolve(path.join(options.from.toString(), project.getDebugDir(), solutionName)), [], {stdio: 'inherit', cwd: path.join(options.from.toString(), project.getDebugDir())});
						}
						else {
							log.info('--run not yet implemented for this platform');
						}
					}
				}
				else {
					log.error('Compilation failed.');
					process.exit(code);
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
