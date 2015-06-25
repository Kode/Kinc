var fs = require('fs');
var path = require('path');
var GraphicsApi = require('./GraphicsApi.js');
var Options = require('./Options.js');
var Path = require('./Path.js');
var Paths = require('./Paths.js');
var Platform = require('./Platform.js');
var Project = require('./Project.js');

function getDefines(platform, rotated) {
	var defines = [];
	switch (platform) {
		case Platform.Windows:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_WINDOWS");
			break;
		case Platform.WindowsApp:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_WINDOWSAPP");
			break;
		case Platform.PlayStation3:
			defines.push("SYS_PS3");
			break;
		case Platform.iOS:
			if (rotated) defines.push("ROTATE90");
			defines.push("SYS_IOS");
			break;
		case Platform.OSX:
			defines.push("SYS_OSX");
			defines.push("SYS_64BIT");
			break;
		case Platform.Android:
			if (rotated) defines.push("ROTATE90");
			defines.push("SYS_ANDROID");
			break;
		case Platform.Xbox360:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_XBOX360");
			break;
		case Platform.HTML5:
			defines.push("SYS_HTML5");
			break;
		case Platform.Linux:
			defines.push("SYS_LINUX");
			break;
		case Platform.Tizen:
			defines.push("SYS_TIZEN");
			break;
	}
	return defines;
}

function Solution(name) {
	this.name = name;
	this.rotated = false;
	this.cmd = false;
	this.projects = [];
}

Solution.scriptdir = new Path('.');

Solution.prototype.getName = function () {
	return this.name;
};
	
Solution.prototype.getProjects = function () {
	return this.projects;
};

Solution.prototype.addProject = function (project) {
	this.projects.push(project);
};

Solution.prototype.searchFiles = function () {
	for (var p in this.projects) this.projects[p].searchFiles();
};

Solution.prototype.flatten = function () {
	for (var p in this.projects) this.projects[p].flatten();
};

Solution.createProject = function (filename) {
	var file = fs.readFileSync(Solution.scriptdir.resolve(Paths.get(filename, 'korefile.js')).toString(), { encoding: 'utf8' });
	var oldscriptdir = Solution.scriptdir;
	Solution.scriptdir = Solution.scriptdir.resolve(filename);
	var project = new Function(['Project', 'Platform', 'platform', 'GraphicsApi', 'graphics'], file)(Project, Platform, Solution.platform, GraphicsApi, Options.graphicsApi);
	Solution.scriptdir = oldscriptdir;
	return project;
};

Solution.createSolution = function (filename, platform) {
	var file = fs.readFileSync(Solution.scriptdir.resolve(Paths.get(filename, 'korefile.js')).toString(), {encoding: 'utf8'});
	var oldscriptdir = Solution.scriptdir;
	Solution.scriptdir = Solution.scriptdir.resolve(filename);
	var solution = new Function([
		'Solution',
		'Project',
		'Platform',
		'platform',
		'GraphicsApi',
		'graphics',
		'fs',
		'path'], file)
		(Solution,
			Project,
			Platform,
			platform,
			GraphicsApi,
			Options.graphicsApi,
			require('fs'),
			require('path'));
	Solution.scriptdir = oldscriptdir;

	var libdirs = fs.readdirSync(path.join(Solution.scriptdir.toString(), 'Libraries'));
	for (var ld in libdirs) {
		var libdir = path.join(Solution.scriptdir.toString().toString(), 'Libraries', libdirs[ld]);
		if (fs.statSync(libdir).isDirectory()) {
			var korefile = path.join(libdir, 'korefile.js');
			if (fs.existsSync(korefile)) {
				solution.projects[0].addSubProject(Solution.createProject(libdir));
			}
		}
	}

	return solution;
};

Solution.evalProjectScript = function (script) {

};

Solution.evalSolutionScript = function (script, platform) {
	this.platform = platform;
};

Solution.create = function (directory, platform) {
	Solution.scriptdir = directory;
	Solution.platform = platform;
	var solution = Solution.createSolution('.', platform);
	var defines = getDefines(platform, solution.isRotated());
	for (var p in solution.projects) {
		for (var d in defines) solution.projects[p].addDefine(defines[d]);
	}
	return solution;
};

Solution.prototype.isRotated = function () {
	return this.rotated;
};

Solution.prototype.isCmd = function () {
	return this.cmd;
};

Solution.prototype.setRotated = function () {
	this.rotated = true;
};

Solution.prototype.setCmd = function () {
	this.cmd = true;
};

module.exports = Solution;
