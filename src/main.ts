import * as child_process from 'child_process';
import * as fs from 'fs-extra';
import * as os from 'os';
import * as path from 'path';
import * as log from './log';
import {GraphicsApi} from './GraphicsApi';
import {Options} from './Options';
import {Project} from './Project';
import {Platform} from './Platform';
import {Solution} from './Solution';
import * as exec from './exec';
import {VisualStudioVersion} from './VisualStudioVersion';
import {ExporterAndroid} from './ExporterAndroid';
import {ExporterCodeBlocks} from './ExporterCodeBlocks';
import {ExporterMakefile} from './ExporterMakefile';
import {ExporterEmscripten} from './ExporterEmscripten';
import {ExporterTizen} from './ExporterTizen';
import {ExporterVisualStudio} from './ExporterVisualStudio';
import {ExporterXCode} from './ExporterXCode';

function fromPlatform(platform: string): string {
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

function shaderLang(platform: string): string {
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
	
	if (Project.koreDir !== '') {
		if (nokrafix) {
			compiler = path.resolve(Project.koreDir, 'Tools', 'kfx', 'kfx' + exec.sys());
		}
		else {
			compiler = path.resolve(Project.koreDir, 'Tools', 'krafix', 'krafix' + exec.sys());
		}
	}

	if (fs.existsSync(path.join(projectDir.toString(), 'Backends'))) {
		let libdirs = fs.readdirSync(path.join(projectDir.toString(), 'Backends'));
		for (let ld in libdirs) {
			let libdir = path.join(projectDir.toString(), 'Backends', libdirs[ld]);
			if (fs.statSync(libdir).isDirectory()) {
				let exe = path.join(libdir, 'krafix', 'krafix-' + platform + '.exe');
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

function exportKoremakeProject(from: string, to: string, platform: string, options) {
	log.info('korefile found.');
	log.info('Creating ' + fromPlatform(platform) + ' project files.');

	let solution = Solution.create(from, platform);
	solution.searchFiles();
	solution.flatten();

	fs.ensureDirSync(to);

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
			let libsdir = path.join(from.toString(), 'Backends');
			if (fs.existsSync(libsdir) && fs.statSync(libsdir).isDirectory()) {
				let libdirs = fs.readdirSync(libsdir);
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
	}

	if (exporter === null) {
		throw 'No exporter found for platform ' + platform + '.';
	}

	exporter.exportSolution(solution, from, to, platform, options.vrApi, options.nokrafix, options);

	return solution;
}

function isKoremakeProject(directory: string): boolean {
	return fs.existsSync(path.resolve(directory, 'korefile.js'));
}

function exportProject(from: string, to: string, platform: string, options) {
	if (isKoremakeProject(from)) {
		return exportKoremakeProject(from, to, platform, options);
	}
	else {
		log.error("korefile.js not found.");
		return null;
	}
}

function compileProject(make, project, solutionName: string, options): Promise<void> {
	return new Promise<void>((resolve, reject) => {
		make.stdout.on('data', function (data) {
			log.info(data.toString());
		});

		make.stderr.on('data', function (data) {
			log.error(data.toString());
		});

		make.on('close', function (code) {
			if (code === 0) {
				if (options.target === Platform.Linux) {
					fs.copySync(path.join(options.to.toString(), solutionName), path.join(options.from.toString(), project.getDebugDir(), solutionName), { clobber: true });
				}
				else if (options.target === Platform.Windows) {
					fs.copySync(path.join(options.to.toString(), 'Debug', solutionName + '.exe'), path.join(options.from.toString(), project.getDebugDir(), solutionName + '.exe'), { clobber: true });
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
		});
	});
}

export let api = 2;

export async function run(options, loglog): Promise<string> {
	log.set(loglog);
	
	if (options.graphics !== undefined) {
		Options.graphicsApi = options.graphics;
	}
	
	if (options.visualstudio !== undefined) {
		Options.visualStudioVersion = options.visualstudio;	
	}
	
	//if (options.vr != undefined) {
	//	Options.vrApi = options.vr;
	//}
	
	let solution = exportProject(options.from, options.to, options.target, options);
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
				fs.writeFileSync(path.join(options.to, 'build.bat'), '@call "' + vsvars + '"\n' + '@MSBuild.exe "' + solutionName + '.vcxproj" /m /p:Configuration=Debug,Platform=Win32');
				make = child_process.spawn('build.bat', [], {cwd: options.to});
			}
			else {
				log.error('Visual Studio not found.');
			}
		}

		if (make !== null) {
			await compileProject(make, project, solutionName, options);
			return solutionName;
		}
		else {
			log.info('--compile not yet implemented for this platform');
			return solutionName;
		}
	}
	return solutionName;
};
