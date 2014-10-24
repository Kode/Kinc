var Exporter = require('./Exporter.js');
var Files = require('./Files.js');
var Paths = require('./Paths.js');
var fs = require('fs');

function ExporterCodeBlocks() {

}

ExporterCodeBlocks.prototype = Object.create(Exporter.prototype);
ExporterCodeBlocks.constructor = ExporterCodeBlocks;

ExporterCodeBlocks.prototype.exportSolution = function (solution, from, to, platform) {
	var project = solution.getProjects()[0];

	var objects = {};
	var ofiles = {};
	for (var f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (file.endsWith(".cpp") || file.endsWith(".c") || file.endsWith("cc")) {
			var name = file.toLowerCase();
			if (name.contains('/')) name = name.substr(name.lastIndexOf('/') + 1);
			name = name.substr(0, name.lastIndexOf('.'));
			if (!objects[name]) {
				objects[name] = true;
				ofiles[file] = name;
			}
			else {
				while (objects[name]) {
					name = name + "_";
				}
				objects[name] = true;
				ofiles[file] = name;
			}
		}
	}

	var ofilelist = '';
	for (var o in objects) {
		ofilelist += o + '.o ';
	}

	this.writeFile(to.resolve('makefile'));

	var incline = '';
	for (var i in project.getIncludeDirs()) {
		var inc = project.getIncludeDirs()[i];
		incline += '-I' + inc + ' ';
	}
	this.p('INC=' + incline);
	
	var libsline = '-pthread -LGL -LX11 -Lasound -Ldl';
	this.p('LIB=' + libsline);
	
	var defline = '';
	for (var d in project.getDefines()) {
		var def = project.getDefines()[d];
		defline += '-D' + def + ' ';
	}
	this.p('DEF=' + defline);
	this.p();

	this.p(project.getName() + ': ' + ofilelist);
	this.p('\tg++ $(LIB) ' + ofilelist + ' -o ' + project.getName());

	for (var f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (file.endsWith('.c') || file.endsWith('.cpp') || file.endsWith('cc')) {
			this.p();
			var name = ofiles[file];
			this.p(name + '.o: ' + file);
			this.p('g++ $(INC) $(DEF) -c ' + file);
		}
	}

	//project.getDefines()
	//project.getIncludeDirs()

	this.closeFile();
};

module.exports = ExporterCodeBlocks;
