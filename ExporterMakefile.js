"use strict";

const Exporter = require('./Exporter.js');
const Files = require('./Files.js');
const Paths = require('./Paths.js');
const GraphicsApi = require('./GraphicsApi.js');
const Options = require('./Options.js');
const Platform = require('./Platform.js');
const fs = require('fs');

class ExporterMakefile extends Exporter {
	constructor() {
		super();
	}

	exportSolution(solution, from, to, platform, vrApi, nokrafix, options) {
		let project = solution.getProjects()[0];

		let objects = {};
		let ofiles = {};
		for (let file of project.getFiles()) {
			if (file.endsWith(".cpp") || file.endsWith(".c") || file.endsWith("cc")) {
				let name = file.toLowerCase();
				if (name.indexOf('/') >= 0) name = name.substr(name.lastIndexOf('/') + 1);
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
		for (let o in objects) {
			ofilelist += o + '.o ';
		}

		this.writeFile(to.resolve('makefile'));

		let incline = '';
		for (let inc of project.getIncludeDirs()) {
			inc = to.relativize(from.resolve(inc));
			incline += '-I' + inc + ' ';
		}
		this.p('INC=' + incline);

		let libsline = '-static-libgcc -static-libstdc++ -pthread';
		for (let lib of project.getLibs()) {
			libsline += ' -l' + lib;
		}
		this.p('LIB=' + libsline);

		let defline = '';
		for (let def of project.getDefines()) {
			defline += '-D' + def + ' ';
		}
		this.p('DEF=' + defline);
		this.p();

		let optimization = '';
		if (!options.debug) optimization = '-O3';

		this.p(project.getName() + ': ' + ofilelist);
        
        let cpp = '';
        if (project.cpp11) {
            cpp = '-std=c++11'
        }
        
		this.p('\tg++ ' + cpp + ' ' + optimization + ' ' + ofilelist + ' -o "' + project.getName() + '" $(LIB)');

		for (let file of project.getFiles()) {
			if (file.endsWith('.c') || file.endsWith('.cpp') || file.endsWith('cc')) {
				this.p();
				let name = ofiles[file];
				let realfile = to.relativize(from.resolve(file));
				this.p(name + '.o: ' + realfile);
				this.p('\tg++ ' + cpp + ' ' + optimization + ' $(INC) $(DEF) -c ' + realfile + ' -o ' + name + '.o $(LIB)');
			}
		}

		//project.getDefines()
		//project.getIncludeDirs()

		this.closeFile();
	}
}

module.exports = ExporterMakefile;
