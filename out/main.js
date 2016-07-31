"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator.throw(value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : new P(function (resolve) { resolve(result.value); }).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments)).next());
    });
};
const child_process = require('child_process');
const fs = require('fs-extra');
const path = require('path');
const log = require('./log');
const GraphicsApi_1 = require('./GraphicsApi');
const Options_1 = require('./Options');
const Project_1 = require('./Project');
const Platform_1 = require('./Platform');
const Solution_1 = require('./Solution');
const exec = require('./exec');
const ExporterAndroid_1 = require('./ExporterAndroid');
const ExporterCodeBlocks_1 = require('./ExporterCodeBlocks');
const ExporterMakefile_1 = require('./ExporterMakefile');
const ExporterEmscripten_1 = require('./ExporterEmscripten');
const ExporterTizen_1 = require('./ExporterTizen');
const ExporterVisualStudio_1 = require('./ExporterVisualStudio');
const ExporterXCode_1 = require('./ExporterXCode');
function fromPlatform(platform) {
    switch (platform) {
        case Platform_1.Platform.Windows:
            return "Windows";
        case Platform_1.Platform.WindowsApp:
            return "Windows App";
        case Platform_1.Platform.PlayStation3:
            return "PlayStation 3";
        case Platform_1.Platform.iOS:
            return "iOS";
        case Platform_1.Platform.OSX:
            return "OS X";
        case Platform_1.Platform.Android:
            return "Android";
        case Platform_1.Platform.Xbox360:
            return "Xbox 360";
        case Platform_1.Platform.Linux:
            return "Linux";
        case Platform_1.Platform.HTML5:
            return "HTML5";
        case Platform_1.Platform.Tizen:
            return "Tizen";
        case Platform_1.Platform.Pi:
            return "Pi";
        case Platform_1.Platform.tvOS:
            return "tvOS";
        default:
            return "unknown";
    }
}
function shaderLang(platform) {
    switch (platform) {
        case Platform_1.Platform.Windows:
            switch (Options_1.Options.graphicsApi) {
                case GraphicsApi_1.GraphicsApi.OpenGL:
                case GraphicsApi_1.GraphicsApi.OpenGL2:
                    return "glsl";
                case GraphicsApi_1.GraphicsApi.Direct3D9:
                    return "d3d9";
                case GraphicsApi_1.GraphicsApi.Direct3D11:
                    return "d3d11";
                case GraphicsApi_1.GraphicsApi.Direct3D12:
                    return 'd3d11';
                case GraphicsApi_1.GraphicsApi.Vulkan:
                    return 'spirv';
                default:
                    return "d3d9";
            }
        case Platform_1.Platform.WindowsApp:
            return "d3d11";
        case Platform_1.Platform.PlayStation3:
            return "d3d9";
        case Platform_1.Platform.iOS:
        case Platform_1.Platform.tvOS:
            switch (Options_1.Options.graphicsApi) {
                case GraphicsApi_1.GraphicsApi.Metal:
                    return 'metal';
                default:
                    return 'essl';
            }
        case Platform_1.Platform.OSX:
            switch (Options_1.Options.graphicsApi) {
                case GraphicsApi_1.GraphicsApi.Metal:
                    return 'metal';
                default:
                    return 'glsl';
            }
        case Platform_1.Platform.Android:
            switch (Options_1.Options.graphicsApi) {
                case GraphicsApi_1.GraphicsApi.Vulkan:
                    return 'spirv';
                default:
                    return 'essl';
            }
        case Platform_1.Platform.Xbox360:
            return "d3d9";
        case Platform_1.Platform.Linux:
            switch (Options_1.Options.graphicsApi) {
                case GraphicsApi_1.GraphicsApi.Vulkan:
                    return 'spirv';
                default:
                    return 'glsl';
            }
        case Platform_1.Platform.HTML5:
            return "essl";
        case Platform_1.Platform.Tizen:
            return "essl";
        case Platform_1.Platform.Pi:
            return "essl";
        default:
            return platform;
    }
}
function compileShader(projectDir, type, from, to, temp, platform, nokrafix) {
    let compiler = '';
    if (Project_1.Project.koreDir !== '') {
        if (nokrafix) {
            compiler = path.resolve(Project_1.Project.koreDir, 'Tools', 'kfx', 'kfx' + exec.sys());
        }
        else {
            compiler = path.resolve(Project_1.Project.koreDir, 'Tools', 'krafix', 'krafix' + exec.sys());
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
function exportKoremakeProject(from, to, platform, options) {
    log.info('korefile found.');
    log.info('Creating ' + fromPlatform(platform) + ' project files.');
    let solution = Solution_1.Solution.create(from, platform);
    solution.searchFiles();
    solution.flatten();
    fs.ensureDirSync(to);
    let project = solution.getProjects()[0];
    let files = project.getFiles();
    for (let file of files) {
        if (file.file.endsWith(".glsl")) {
            let outfile = file.file;
            const index = outfile.lastIndexOf('/');
            if (index > 0)
                outfile = outfile.substr(index);
            outfile = outfile.substr(0, outfile.length - 5);
            compileShader(from, shaderLang(platform), file.file, path.join(project.getDebugDir(), outfile), "build", platform, options.nokrafix);
        }
    }
    let exporter = null;
    if (platform === Platform_1.Platform.iOS || platform === Platform_1.Platform.OSX || platform === Platform_1.Platform.tvOS)
        exporter = new ExporterXCode_1.ExporterXCode();
    else if (platform == Platform_1.Platform.Android)
        exporter = new ExporterAndroid_1.ExporterAndroid();
    else if (platform == Platform_1.Platform.HTML5)
        exporter = new ExporterEmscripten_1.ExporterEmscripten();
    else if (platform == Platform_1.Platform.Linux || platform === Platform_1.Platform.Pi) {
        if (options.compile)
            exporter = new ExporterMakefile_1.ExporterMakefile();
        else
            exporter = new ExporterCodeBlocks_1.ExporterCodeBlocks();
    }
    else if (platform == Platform_1.Platform.Tizen)
        exporter = new ExporterTizen_1.ExporterTizen();
    else {
        let found = false;
        for (var p in Platform_1.Platform) {
            if (platform === Platform_1.Platform[p]) {
                found = true;
                break;
            }
        }
        if (found) {
            exporter = new ExporterVisualStudio_1.ExporterVisualStudio();
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
    return fs.existsSync(path.resolve(directory, 'korefile.js'));
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
function compileProject(make, project, solutionName, options) {
    return new Promise((resolve, reject) => {
        make.stdout.on('data', function (data) {
            log.info(data.toString());
        });
        make.stderr.on('data', function (data) {
            log.error(data.toString());
        });
        make.on('close', function (code) {
            if (code === 0) {
                if (options.target === Platform_1.Platform.Linux) {
                    fs.copySync(path.join(options.to.toString(), solutionName), path.join(options.from.toString(), project.getDebugDir(), solutionName));
                }
                else if (options.target === Platform_1.Platform.Windows) {
                    fs.copySync(path.join(options.to.toString(), 'Debug', solutionName + '.exe'), path.join(options.from.toString(), project.getDebugDir(), solutionName + '.exe'));
                }
                if (options.run) {
                    if (options.target === Platform_1.Platform.OSX) {
                        child_process.spawn('open', ['build/Release/' + solutionName + '.app/Contents/MacOS/' + solutionName], { stdio: 'inherit', cwd: options.to });
                    }
                    else if (options.target === Platform_1.Platform.Linux || options.target === Platform_1.Platform.Windows) {
                        child_process.spawn(path.resolve(path.join(options.from.toString(), project.getDebugDir(), solutionName)), [], { stdio: 'inherit', cwd: path.join(options.from.toString(), project.getDebugDir()) });
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
exports.api = 2;
function run(options, loglog) {
    return __awaiter(this, void 0, Promise, function* () {
        log.set(loglog);
        if (options.graphics !== undefined) {
            Options_1.Options.graphicsApi = options.graphics;
        }
        if (options.visualstudio !== undefined) {
            Options_1.Options.visualStudioVersion = options.visualstudio;
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
            if (options.target === Platform_1.Platform.Linux) {
                make = child_process.spawn('make', [], { cwd: options.to });
            }
            else if (options.target === Platform_1.Platform.OSX) {
                make = child_process.spawn('xcodebuild', ['-project', solutionName + '.xcodeproj'], { cwd: options.to });
            }
            else if (options.target === Platform_1.Platform.Windows) {
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
                    make = child_process.spawn('build.bat', [], { cwd: options.to });
                }
                else {
                    log.error('Visual Studio not found.');
                }
            }
            if (make !== null) {
                yield compileProject(make, project, solutionName, options);
                return solutionName;
            }
            else {
                log.info('--compile not yet implemented for this platform');
                return solutionName;
            }
        }
        return solutionName;
    });
}
exports.run = run;
;
//# sourceMappingURL=main.js.map