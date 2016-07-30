"use strict";
const fs = require('fs');
const path = require('path');
const GraphicsApi_1 = require('./GraphicsApi');
const Options_1 = require('./Options');
const Platform_1 = require('./Platform');
const Project_1 = require('./Project');
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
let scriptdir = '.';
class Solution {
    constructor(name) {
        this.name = name;
        this.rotated = false;
        this.cmd = false;
        this.projects = [];
    }
    getName() {
        return this.name;
    }
    getProjects() {
        return this.projects;
    }
    addProject(project) {
        this.projects.push(project);
    }
    searchFiles() {
        for (let p of this.projects)
            p.searchFiles(undefined);
    }
    flatten() {
        for (let p of this.projects)
            p.flatten();
    }
    ;
    static createProject(filename) {
        let file = fs.readFileSync(path.resolve(Solution.scriptdir, filename, 'korefile.js'), { encoding: 'utf8' });
        let oldscriptdir = Solution.scriptdir;
        Solution.scriptdir = path.resolve(Solution.scriptdir, filename);
        let project = new Function('Project', 'Platform', 'platform', 'GraphicsApi', 'graphics', file)(Project_1.Project, Platform_1.Platform, Solution.platform, GraphicsApi_1.GraphicsApi, Options_1.Options.graphicsApi);
        Solution.scriptdir = oldscriptdir;
        return project;
    }
    static createSolution(filename, platform) {
        let file = fs.readFileSync(path.resolve(Solution.scriptdir, filename, 'korefile.js'), { encoding: 'utf8' });
        let oldscriptdir = Solution.scriptdir;
        Solution.scriptdir = path.resolve(Solution.scriptdir, filename);
        let solution = new Function('Solution', 'Project', 'Platform', 'platform', 'GraphicsApi', 'graphics', 'fs', 'path', file)(Solution, Project_1.Project, Platform_1.Platform, platform, GraphicsApi_1.GraphicsApi, Options_1.Options.graphicsApi, require('fs'), require('path'));
        Solution.scriptdir = oldscriptdir;
        if (fs.existsSync(path.join(Solution.scriptdir.toString(), 'Backends'))) {
            var libdirs = fs.readdirSync(path.join(Solution.scriptdir.toString(), 'Backends'));
            for (var ld in libdirs) {
                var libdir = path.join(Solution.scriptdir.toString().toString(), 'Backends', libdirs[ld]);
                if (fs.statSync(libdir).isDirectory()) {
                    var korefile = path.join(libdir, 'korefile.js');
                    if (fs.existsSync(korefile)) {
                        solution.projects[0].addSubProject(Solution.createProject(libdir));
                    }
                }
            }
        }
        return solution;
    }
    static evalProjectScript(script) {
    }
    static evalSolutionScript(script, platform) {
        this.platform = platform;
    }
    static create(directory, platform) {
        Solution.scriptdir = directory;
        Solution.platform = platform;
        var solution = Solution.createSolution('.', platform);
        var defines = getDefines(platform, solution.isRotated());
        for (var p in solution.projects) {
            for (var d in defines)
                solution.projects[p].addDefine(defines[d]);
        }
        return solution;
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
exports.Solution = Solution;
//# sourceMappingURL=Solution.js.map