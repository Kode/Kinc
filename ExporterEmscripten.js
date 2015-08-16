var child_process = require('child_process');
var Exporter = require('./Exporter.js');
var Files = require('./Files.js');
var Paths = require('./Paths.js');
var os = require('os');
var fs = require('fs');

var emmccPath = '';

function ExporterEmscripten(emcc) {
	if (os.platform() === "linux") {
		emmccPath = emcc;
	} else
	if (os.platform() === "win32") {
		emmccPath = '"' + emcc + '"';
	}
	else {
		emmccPath = emcc;
	}
}

ExporterEmscripten.prototype = Object.create(Exporter.prototype);
ExporterEmscripten.constructor = ExporterEmscripten;

var defines = '';
var includes = '';
var definesArray = [];
var includesArray = [];

function compile(inFilename, outFilename) {
	if (Files.exists(Paths.get(outFilename)))
		return;
	
	//console.log("Compiling " + inFilename + " to " + outFilename);
	//console.log(emmccPath + " " + inFilename+ " " +includes+ " " +defines+ " " +"-c -o"+ " " +outFilename);

	//console.log(emmccPath + " " + inFilename+ " " +  includes+ " " +  defines+ " " + "-c -o"+ " " + outFilename);

	var params = [];
	params.push(inFilename);

	for(var i = 0; i < includesArray.length; i++) {
		params.push(includesArray[i]);
	}

	for(var i = 0; i < definesArray.length; i++) {
		params.push(definesArray[i]);
	}

	params.push("-c");
	params.push("-o");
	params.push(outFilename);

	//console.log(params);

	var res = child_process.spawnSync(emmccPath, params, { stdio: 'inherit', stderr: 'inherit' });	
	if (res != null) {
		if (res.stdout != null && res.stdout != '')
			console.log("stdout: " + res.stdout);
		if (res.stderr != null && res.stderr != '')
			console.log("stderr: " + res.stderr);
		if (res.error != null)
			console.log(res.error);
	}
}

function link(files, output) {
	var params = [];//-O2 ";
	for (var file in files) {
		//console.log(files[file]);
		params.push(files[file]);
	}
	params.push("-o");
	params.push(output);

	console.log("Linking " + output);

	var res = child_process.spawnSync(emmccPath, params);
	if (res != null) {
		if (res.stdout != null && res.stdout != '')
			console.log("stdout: " + res.stdout);
		if (res.stderr != null && res.stderr != '')
			console.log("stderr: " + res.stderr);
		if (res.error != null)
			console.log(res.error);
	}
}

ExporterEmscripten.prototype.exportSolution = function (solution, from, to, platform) {
	var project = solution.getProjects()[0];

	var assets = [];
	this.copyDirectory(from.resolve(project.getDebugDir()), to, assets);

	defines = "";
	definesArray = [];
	for (var def in project.getDefines()) {
		defines += "-D" + project.getDefines()[def] + " ";
		definesArray.push("-D" + project.getDefines()[def]);
	}

	includes = "";
	includesArray = [];
	for (var inc in project.getIncludeDirs()) {
		includes += "-I../" + from.resolve(project.getIncludeDirs()[inc]).toString() + " ";
		includesArray.push("-I../" + from.resolve(project.getIncludeDirs()[inc]).toString());
	}

	this.writeFile(to.resolve("makefile"));

	this.p();
	var oline = '';
	for (var f in project.getFiles()) {
		var filename = project.getFiles()[f];
		if (!filename.endsWith(".cpp") && !filename.endsWith(".c")) continue;
		var lastpoint = filename.lastIndexOf('.');
		var oname = filename.substr(0, lastpoint) + ".o";
		oname = oname.replaceAll("../", "");
		oline += " " + oname;
	}
	var assetline = '';
	for (var asset in assets) {
		assetline += " --preload-file " + assets[asset];
	}
	this.p("kore.html:" + oline);
		this.p("$(CC) " + oline + " -o kore.html" + assetline, 1);
	this.p();

	for (var f in project.getFiles()) {
		var filename = project.getFiles()[f];
		if (!filename.endsWith(".cpp") && !filename.endsWith(".c")) continue;
		var builddir = to;
		var dirs = filename.split('/');
		var name = '';
		for (var i = 0; i < dirs.length - 1; ++i) {
			var s = dirs[i];
			if (s == "" || s == "..") continue;
			name += s + "/";
			builddir = builddir.resolve(s);
			if (!Files.exists(builddir)) Files.createDirectories(builddir);
		}
		var lastpoint = filename.lastIndexOf('.');
		var oname = filename.substr(0, lastpoint) + ".o";
		oname = oname.replaceAll("../", "");
		this.p(oname + ": ../" + filename);
		this.p("$(CC) -c ../" + filename + " " + includes + " " + defines + " -o " + oname, 1);
	}

	this.closeFile();

	/*
	std::vector<std::string> objectFiles;
	for (std::string filename : project->getFiles()) if (endsWith(filename, ".c") || endsWith(filename, ".cpp")) {
		//files += "../../" + filename + " ";
		compile(directory.resolve(filename).toString(), directory.resolve(Paths::get("build", filename + ".o")).toString());
		objectFiles.push_back(directory.resolve(Paths::get("build", filename + ".o")).toString());
	}
	link(objectFiles, directory.resolve(Paths::get("build", "Kt.js")));
	*/


	console.log("Compiling files...");
	var objectFiles = [];
	var files = project.getFiles();
	for (var f in files) {
		var file = files[f];
		if (file.endsWith(".c") || file.endsWith(".cpp")) {
			//files += "../../" + filename + " ";
			compile(from.resolve(file).toString(), to.resolve(file + ".o").toString());
			objectFiles.push(to.resolve(file + ".o").toString());
		}
	}
	link(objectFiles, to.resolve(Paths.get("build", "Kt.js").toString()));
};

module.exports = ExporterEmscripten;