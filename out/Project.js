"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator.throw(value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : new P(function (resolve) { resolve(result.value); }).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments)).next());
    });
};
const fs = require('fs-extra');
const path = require('path');
const GraphicsApi_1 = require('./GraphicsApi');
const Options_1 = require('./Options');
const Platform_1 = require('./Platform');
const uuid = require('uuid');
function getDefines(platform, rotated) {
    let defines = [];
    switch (platform) {
        case Platform_1.Platform.Windows:
            defines.push("_CRT_SECURE_NO_WARNINGS");
            defines.push("SYS_WINDOWS");
            break;
        case Platform_1.Platform.WindowsApp:
            defines.push("_CRT_SECURE_NO_WARNINGS");
            defines.push("SYS_WINDOWSAPP");
            break;
        case Platform_1.Platform.PlayStation3:
            defines.push("SYS_PS3");
            break;
        case Platform_1.Platform.iOS:
            if (rotated)
                defines.push("ROTATE90");
            defines.push("SYS_IOS");
            break;
        case Platform_1.Platform.tvOS:
            defines.push("SYS_TVOS");
            break;
        case Platform_1.Platform.OSX:
            defines.push("SYS_OSX");
            defines.push("SYS_64BIT");
            break;
        case Platform_1.Platform.Android:
            if (rotated)
                defines.push("ROTATE90");
            defines.push("SYS_ANDROID");
            break;
        case Platform_1.Platform.Xbox360:
            defines.push("_CRT_SECURE_NO_WARNINGS");
            defines.push("SYS_XBOX360");
            break;
        case Platform_1.Platform.HTML5:
            defines.push("SYS_HTML5");
            break;
        case Platform_1.Platform.Linux:
            defines.push("SYS_LINUX");
            break;
        case Platform_1.Platform.Tizen:
            defines.push("SYS_TIZEN");
            break;
    }
    return defines;
}
function contains(array, value) {
    for (let element of array) {
        if (element === value)
            return true;
    }
    return false;
}
function isAbsolute(path) {
    return (path.length > 0 && path[0] == '/') || (path.length > 1 && path[1] == ':');
}
let scriptdir = '.';
let koreDir = '.';
class Project {
    constructor(name, basedir) {
        this.name = name;
        this.debugDir = '';
        this.basedir = basedir;
        if (name == 'Kore')
            Project.koreDir = this.basedir;
        this.uuid = uuid.v4();
        this.files = [];
        this.javadirs = [];
        this.subProjects = [];
        this.includeDirs = [];
        this.defines = [];
        this.libs = [];
        this.systemDependendLibraries = {};
        this.includes = [];
        this.excludes = [];
        this.cpp11 = false;
        this.targetOptions = {
            android: {}
        };
        this.rotated = false;
        this.cmd = false;
    }
    flatten() {
        for (let sub of this.subProjects)
            sub.flatten();
        for (let sub of this.subProjects) {
            let subbasedir = sub.basedir;
            for (let d of sub.defines)
                if (!contains(this.defines, d))
                    this.defines.push(d);
            for (let file of sub.files) {
                this.files.push({ file: path.join(subbasedir, file.file).replace(/\\/g, '/'), options: file.options, projectDir: subbasedir, projectName: sub.name });
            }
            for (let i of sub.includeDirs)
                if (!contains(this.includeDirs, path.resolve(subbasedir, i)))
                    this.includeDirs.push(path.resolve(subbasedir, i));
            for (let j of sub.javadirs)
                if (!contains(this.javadirs, path.resolve(subbasedir, j)))
                    this.javadirs.push(path.resolve(subbasedir, j));
            for (let lib of sub.libs) {
                if (lib.indexOf('/') < 0 && lib.indexOf('\\') < 0) {
                    if (!contains(this.libs, lib))
                        this.libs.push(lib);
                }
                else {
                    if (!contains(this.libs, path.resolve(subbasedir, lib)))
                        this.libs.push(path.resolve(subbasedir, lib));
                }
            }
            for (let system in sub.systemDependendLibraries) {
                let libs = sub.systemDependendLibraries[system];
                for (let lib of libs) {
                    if (this.systemDependendLibraries[system] === undefined)
                        this.systemDependendLibraries[system] = [];
                    if (!contains(this.systemDependendLibraries[system], this.stringify(path.resolve(subbasedir, lib)))) {
                        if (!contains(lib, '/') && !contains(lib, '\\'))
                            this.systemDependendLibraries[system].push(lib);
                        else
                            this.systemDependendLibraries[system].push(this.stringify(path.resolve(subbasedir, lib)));
                    }
                }
            }
        }
        this.subProjects = [];
    }
    getName() {
        return this.name;
    }
    getUuid() {
        return this.uuid;
    }
    matches(text, pattern) {
        const regexstring = pattern.replace(/\./g, "\\.").replace(/\*\*/g, ".?").replace(/\*/g, "[^/]*").replace(/\?/g, '*');
        const regex = new RegExp('^' + regexstring + '$', 'g');
        return regex.test(text);
    }
    matchesAllSubdirs(dir, pattern) {
        if (pattern.endsWith("/**")) {
            return this.matches(this.stringify(dir), pattern.substr(0, pattern.length - 3));
        }
        else
            return false;
    }
    stringify(path) {
        return path.replace(/\\/g, '/');
    }
    addFileForReal(file, options) {
        for (let index in this.files) {
            if (this.files[index].file === file) {
                this.files[index] = { file: file, options: options, projectDir: this.basedir, projectName: this.name };
                return;
            }
        }
        this.files.push({ file: file, options: options, projectDir: this.basedir, projectName: this.name });
    }
    searchFiles(current) {
        if (current === undefined) {
            for (let sub of this.subProjects)
                sub.searchFiles(undefined);
            this.searchFiles(this.basedir);
            //std::set<std::string> starts;
            //for (std::string include : includes) {
            //	if (!isAbsolute(include)) continue;
            //	std::string start = include.substr(0, firstIndexOf(include, '*'));
            //	if (starts.count(start) > 0) continue;
            //	starts.insert(start);
            //	searchFiles(Paths::get(start));
            //}
            return;
        }
        let files = fs.readdirSync(current);
        nextfile: for (let f in files) {
            var file = path.join(current, files[f]);
            if (fs.statSync(file).isDirectory())
                continue;
            //if (!current.isAbsolute())
            file = path.relative(this.basedir, file);
            for (let exclude of this.excludes) {
                if (this.matches(this.stringify(file), exclude))
                    continue nextfile;
            }
            for (let includeobject of this.includes) {
                let include = includeobject.file;
                if (isAbsolute(include)) {
                    let inc = include;
                    inc = path.relative(this.basedir, inc);
                    include = inc;
                }
                if (this.matches(this.stringify(file), include)) {
                    this.addFileForReal(this.stringify(file), includeobject.options);
                }
            }
        }
        let dirs = fs.readdirSync(current);
        nextdir: for (let d of dirs) {
            let dir = path.join(current, d);
            if (d.startsWith('.'))
                continue;
            if (!fs.statSync(dir).isDirectory())
                continue;
            for (let exclude of this.excludes) {
                if (this.matchesAllSubdirs(path.relative(this.basedir, dir), exclude)) {
                    continue nextdir;
                }
            }
            this.searchFiles(dir);
        }
    }
    addFile(file, options) {
        this.includes.push({ file: file, options: options });
    }
    addFiles() {
        let options = undefined;
        for (let i = 0; i < arguments.length; ++i) {
            if (typeof arguments[i] !== 'string') {
                options = arguments[i];
            }
        }
        for (let i = 0; i < arguments.length; ++i) {
            if (typeof arguments[i] === 'string') {
                this.addFile(arguments[i], options);
            }
        }
    }
    addJavaDir(dir) {
        this.javadirs.push(dir);
    }
    addJavaDirs() {
        for (let i = 0; i < arguments.length; ++i) {
            this.addJavaDir(arguments[i]);
        }
    }
    addExclude(exclude) {
        this.excludes.push(exclude);
    }
    addExcludes() {
        for (let i = 0; i < arguments.length; ++i) {
            this.addExclude(arguments[i]);
        }
    }
    addDefine(define) {
        if (contains(this.defines, define))
            return;
        this.defines.push(define);
    }
    addDefines() {
        for (let i = 0; i < arguments.length; ++i) {
            this.addDefine(arguments[i]);
        }
    }
    addIncludeDir(include) {
        if (contains(this.includeDirs, include))
            return;
        this.includeDirs.push(include);
    }
    addIncludeDirs() {
        for (let i = 0; i < arguments.length; ++i) {
            this.addIncludeDir(arguments[i]);
        }
    }
    addSubProject(project) {
        this.subProjects.push(project);
    }
    addLib(lib) {
        this.libs.push(lib);
    }
    addLibs() {
        for (let i = 0; i < arguments.length; ++i) {
            this.addLib(arguments[i]);
        }
    }
    addLibFor(system, lib) {
        if (this.systemDependendLibraries[system] === undefined)
            this.systemDependendLibraries[system] = [];
        this.systemDependendLibraries[system].push(lib);
    }
    addLibsFor() {
        if (this.systemDependendLibraries[arguments[0]] === undefined)
            this.systemDependendLibraries[arguments[0]] = [];
        for (let i = 1; i < arguments.length; ++i) {
            this.systemDependendLibraries[arguments[0]].push(arguments[i]);
        }
    }
    getFiles() {
        return this.files;
    }
    getJavaDirs() {
        return this.javadirs;
    }
    getBasedir() {
        return this.basedir;
    }
    getSubProjects() {
        return this.subProjects;
    }
    getIncludeDirs() {
        return this.includeDirs;
    }
    getDefines() {
        return this.defines;
    }
    getLibs() {
        return this.libs;
    }
    getLibsFor(system) {
        if (this.systemDependendLibraries[system] === undefined)
            return [];
        return this.systemDependendLibraries[system];
    }
    getDebugDir() {
        return this.debugDir;
    }
    setDebugDir(debugDir) {
        this.debugDir = debugDir;
    }
    static createProject(filename, scriptdir) {
        return __awaiter(this, void 0, Promise, function* () {
            return new Promise((resolve, reject) => {
                let originalscriptdir = scriptdir;
                scriptdir = path.resolve(scriptdir, filename);
                let resolved = false;
                let resolver = (project) => __awaiter(this, void 0, void 0, function* () {
                    resolved = true;
                    //TODO: This accidentally finds Kha/Backends/KoreHL
                    /*if (fs.existsSync(path.join(scriptdir, 'Backends'))) {
                        var libdirs = fs.readdirSync(path.join(scriptdir, 'Backends'));
                        for (var ld in libdirs) {
                            var libdir = path.join(scriptdir, 'Backends', libdirs[ld]);
                            if (fs.statSync(libdir).isDirectory()) {
                                var korefile = path.join(libdir, 'korefile.js');
                                if (fs.existsSync(korefile)) {
                                    project.addSubProject(await Project.createProject(libdir, scriptdir));
                                }
                            }
                        }
                    }*/
                    resolve(project);
                });
                process.on('exit', (code) => {
                    if (!resolved) {
                        console.error('Error: korefile.js did not call resolve, no project created.');
                    }
                });
                let file = fs.readFileSync(path.resolve(scriptdir, 'korefile.js'), 'utf8');
                let project = new Function('Project', 'Platform', 'platform', 'GraphicsApi', 'graphics', 'require', 'resolve', 'reject', '__dirname', file)(Project, Platform_1.Platform, Project.platform, GraphicsApi_1.GraphicsApi, Options_1.Options.graphicsApi, require, resolver, reject, scriptdir);
            });
        });
    }
    static create(directory, platform) {
        return __awaiter(this, void 0, void 0, function* () {
            Project.platform = platform;
            let project = yield Project.createProject('.', directory);
            let defines = getDefines(platform, project.isRotated());
            for (let define of defines) {
                project.addDefine(define);
            }
            return project;
        });
    }
    isRotated() {
        return this.rotated;
    }
    isCmd() {
        return this.cmd;
    }
    setRotated() {
        this.rotated = true;
    }
    setCmd() {
        this.cmd = true;
    }
}
exports.Project = Project;
//# sourceMappingURL=Project.js.map