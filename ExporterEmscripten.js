var Exporter = require('./Exporter.js');
var Files = require('./Files.js');
var Paths = require('./Paths.js');
var fs = require('fs');

function ExporterEmscripten() {

}

ExporterEmscripten.prototype = Object.create(Exporter.prototype);
ExporterEmscripten.constructor = ExporterEmscripten;

var defines = '';
var includes = '';

function compile(inFilename, outFilename) {
	if (Files.exists(Paths.get(outFilename))) return;
	//if (!new File(outFilename).getParentFile().exists()) new File(outFilename).getParentFile().mkdirs();
	var exe = "/Users/robert/Projekte/emscripten/emcc " + inFilename + " " + includes + " " + defines + " -c -o " + outFilename;
	//std::cout << "Compiling " << inFilename << std::endl;
	execute(exe);
}

function link(files, output) {
	var exe = "/Users/robert/Projekte/emscripten/emcc ";//-O2 ";
	for (var file in files) exe += files[file] + " ";
	exe += "-o " + output.toString();
	//std::cout << "Linking " << output.toString() << std::endl;
	execute(exe);
}

ExporterEmscripten.prototype.exportSolution = function (solution, from, to, platform) {
	var project = solution.getProjects()[0];

	var assets = [];
	this.copyDirectory(from.resolve(project.getDebugDir()), to, assets);

	defines = "";
	for (var def in project.getDefines()) defines += "-D" + project.getDefines()[def] + " ";
	includes = "";
	for (var inc in project.getIncludeDirs()) includes += "-I../" + from.resolve(project.getIncludeDirs()[inc]).toString() + " ";

	this.writeFile(to.resolve("makefile"));

	//p("CC = ~/Projekte/emscripten/emcc");
	this.p();
	var oline = '';
	for (var f in project.getFiles()) {
		var filename = project.getFiles()[f];
		if (!filename.endsWith(".cpp") && !filename.endsWith(".c")) continue;
		var lastpoint = filename.lastIndexOf('.');
		var oname = filename.substr(0, lastpoint) + ".o";
		oname = oname.replace("../", "");
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
		oname = oname.replace("../", "");
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
	link(objectFiles, directory.resolve(Paths::get("build", "Kt.js")));*/
};

module.exports = ExporterEmscripten;
