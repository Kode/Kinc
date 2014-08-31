var Files = require('./Files.js');
var Path = require('./Path.js');
var Paths = require('./Paths.js');
var uuid = require('./uuid.js');

function Project(name) {
	this.name = name;
	this.debugDir = '';
	this.basedir = require('./Solution').scriptdir;
	if (name == 'Kore') Project.koreDir = this.basedir;
	this.uuid = uuid.v4();

	this.files = [];
	this.subProjects = [];
	this.includeDirs = [];
	this.defines = [];
	this.libs = [];
	this.systemDependendLibraries = {};
	this.includes = [];
	this.excludes = [];
}

function contains(array, value) {
	for (var index in array) {
		if (array[index] === value) return true;
	}
	return false;
}

Project.koreDir = new Path('.');

Project.prototype.flatten = function () {
	for (sub in this.subProjects) this.subProjects[sub].flatten();
	for (p in this.subProjects) {
		var sub = this.subProjects[p];
		var basedir = this.basedir;
		//if (basedir.startsWith("./")) basedir = basedir.substring(2);
		var subbasedir = sub.basedir;
		//if (subbasedir.startsWith("./")) subbasedir = subbasedir.substring(2);
		//if (subbasedir.startsWith(basedir)) subbasedir = subbasedir.substring(basedir.length());
		if (subbasedir.startsWith(basedir)) subbasedir = basedir.relativize(subbasedir);

		for (d in sub.defines) if (!contains(this.defines, sub.defines[d])) this.defines.push(sub.defines[d]);
		for (file in sub.files) this.files.push(subbasedir.resolve(sub.files[file]).toString());
		for (i in sub.includeDirs) if (!contains(this.includeDirs, subbasedir.resolve(sub.includeDirs[i]).toString())) this.includeDirs.push(subbasedir.resolve(sub.includeDirs[i]).toString());
		for (l in sub.libs) {
			var l = sub.libs[l];
			if (!contains(lib, '/') && !contains(lib, '\\')) {
				if (!contains(this.libs, lib)) this.libs.push_back(lib);
			}
			else {
				if (!contains(this.libs, subbasedir.resolve(lib).toString())) this.libs.push(subbasedir.resolve(lib).toString());
			}
		}
		for (system in sub.systemDependendLibraries) {
			for (lib in system.second) {
				if (systemDependendLibraries.find(system.first) == systemDependendLibraries.end()) systemDependendLibraries[system.first] = [];
				if (!contains(systemDependendLibraries[system.first], stringify(subbasedir.resolve(lib)))) {
					if (!contains(lib, '/') && !contains(lib, '\\')) systemDependendLibraries[system.first].push_back(lib);
					else systemDependendLibraries[system.first].push_back(stringify(subbasedir.resolve(lib)));
				}
			}
		}
	}
	this.subProjects = [];
};

Project.prototype.getName = function () {
	return this.name;
};

Project.prototype.getUuid = function () {
	return this.uuid;
};

Project.prototype.matches = function (text, pattern) {
	var regexstring = pattern.replace(/\./g, "\\.").replace(/\*\*/g, ".?").replace(/\*/g, "[^/]*").replace(/\?/g, '*');
	var regex = new RegExp('^' + regexstring + '$', 'g');
	return regex.test(text);
}

Project.prototype.matchesAllSubdirs = function (dir, pattern) {
	if (pattern.endsWith("/**")) {
		return this.matches(this.stringify(dir), pattern.substr(0, pattern.length - 3));
	}
	else return false;
};

Project.prototype.stringify = function (path) {
	return path.toString().replace(/\\/g, '/');
};

function isAbsolute(path) {
	return (path.length > 0 && path[0] == '/') || (path.length > 1 && path[1] == ':');
}

Project.prototype.searchFiles = function (current) {
	if (current === undefined) {
		for (sub in this.subProjects) this.subProjects[sub].searchFiles();
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

	var files = Files.newDirectoryStream(current);
	nextfile: for (f in files) {
		var file = Paths.get(current, files[f]);
		if (Files.isDirectory(file)) continue;
		//if (!current.isAbsolute())
		file = this.basedir.relativize(file);
		for (exclude in this.excludes) {
			if (this.matches(this.stringify(file), this.excludes[exclude])) continue nextfile;
		}
		for (i in this.includes) {
			var include = this.includes[i];
			if (isAbsolute(include)) {
				var inc = Paths.get(include);
				inc = this.basedir.relativize(inc);
				include = inc.path;
			}
			if (this.matches(this.stringify(file), include)) {
				this.files.push(this.stringify(file));
			}
		}
	}
	var dirs = Files.newDirectoryStream(current);
	nextdir: for (d in dirs) {
		var dir = Paths.get(current, dirs[d]);
		if (!Files.isDirectory(dir)) continue;
		for (exclude in this.excludes) {
			if (this.matchesAllSubdirs(this.basedir.relativize(dir), this.excludes[exclude])) {
				continue nextdir;
			}
		}
		this.searchFiles(dir);
	}
};

Project.prototype.addFile = function (file) {
	this.includes.push(file);
};

Project.prototype.addFiles = function () {
	for (var i = 0; i < arguments.length; ++i) {
		this.addFile(arguments[i]);
	}
};

Project.prototype.addExclude = function (exclude) {
	this.excludes.push(exclude);
};

Project.prototype.addExcludes = function () {
	for (var i = 0; i < arguments.length; ++i) {
		this.addExclude(arguments[i]);
	}
};

function contains(array, element) {
	for (index in array) {
		if (array[index] === element) return true;
	}
	return false;
}

Project.prototype.addDefine = function (define) {
	if (contains(this.defines, define)) return;
	this.defines.push(define);
};

Project.prototype.addDefines = function () {
	for (var i = 0; i < arguments.length; ++i) {
		this.addDefine(arguments[i]);
	}
};

Project.prototype.addIncludeDir = function (include) {
	if (contains(this.includeDirs, include)) return;
	this.includeDirs.push(include);
};

Project.prototype.addIncludeDirs = function () {
	for (var i = 0; i < arguments.length; ++i) {
		this.addIncludeDir(arguments[i]);
	}
};

Project.prototype.addSubProject = function (project) {
	this.subProjects.push(project);
};

Project.prototype.addLib = function (lib) {
	this.libs.push(lib);
};

Project.prototype.addLibs = function () {
	for (var i = 0; i < arguments.length; ++i) {
		this.addLib(arguments[i]);
	}
};

Project.prototype.addLibFor = function (system, lib) {
	if (this.systemDependendLibraries[system] === undefined) this.systemDependendLibraries[system] = [];
	this.systemDependendLibraries[system].push(lib);
};

Project.prototype.addLibsFor = function () {
	if (this.systemDependendLibraries[arguments[0]] === undefined) this.systemDependendLibraries[arguments[0]] = [];
	for (var i = 1; i < arguments.length; ++i) {
		this.systemDependendLibraries[arguments[0]].push(arguments[i]);
	}
};

Project.prototype.getFiles = function () {
	return this.files;
};

Project.prototype.getBasedir = function () {
	return this.basedir;
};

Project.prototype.getSubProjects = function () {
	return this.subProjects;
};

Project.prototype.getIncludeDirs = function () {
	return this.includeDirs;
};

Project.prototype.getDefines = function () {
	return this.defines;
};

Project.prototype.getLibs = function () {
	return this.libs;
};

Project.prototype.getLibsFor = function (system) {
	if (this.systemDependendLibraries[system] === undefined) return [];
	return this.systemDependendLibraries[system];
};

Project.prototype.getDebugDir = function () {
	return this.debugDir;
};

Project.prototype.setDebugDir = function (debugDir) {
	this.debugDir = debugDir;
};

module.exports = Project;
