"use strict";
const fs = require('fs-extra');
const path = require('path');
const Solution_1 = require('./Solution');
const uuid = require('uuid');
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
let koreDir = '.';
class Project {
    constructor(name) {
        this.name = name;
        this.debugDir = '';
        this.basedir = Solution_1.Solution.scriptdir;
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
    }
    flatten() {
        for (let sub of this.subProjects)
            sub.flatten();
        for (let sub of this.subProjects) {
            let basedir = this.basedir;
            //if (basedir.startsWith("./")) basedir = basedir.substring(2);
            let subbasedir = sub.basedir;
            //if (subbasedir.startsWith("./")) subbasedir = subbasedir.substring(2);
            //if (subbasedir.startsWith(basedir)) subbasedir = subbasedir.substring(basedir.length());
            if (subbasedir.startsWith(basedir))
                subbasedir = path.relative(basedir, subbasedir);
            for (let d of sub.defines)
                if (!contains(this.defines, d))
                    this.defines.push(d);
            for (let file of sub.files) {
                this.files.push({ file: path.join(subbasedir, file.file).replace(/\\/g, '/'), options: file.options });
            }
            for (let i of sub.includeDirs)
                if (!contains(this.includeDirs, path.resolve(subbasedir, i)))
                    this.includeDirs.push(path.resolve(subbasedir, i));
            for (let j of sub.javadirs)
                if (!contains(this.javadirs, path.resolve(subbasedir, j)))
                    this.javadirs.push(path.resolve(subbasedir, j));
            for (let lib of sub.libs) {
                if (!contains(lib, '/') && !contains(lib, '\\')) {
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
                this.files[index] = { file: file, options: options };
                return;
            }
        }
        this.files.push({ file: file, options: options });
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
}
exports.Project = Project;
//# sourceMappingURL=Project.js.map