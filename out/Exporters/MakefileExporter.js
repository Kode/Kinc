"use strict";
const Exporter_1 = require('./Exporter');
const fs = require('fs-extra');
const path = require('path');
class MakefileExporter extends Exporter_1.Exporter {
    constructor() {
        super();
    }
    exportSolution(project, from, to, platform, vrApi, nokrafix, options) {
        let objects = {};
        let ofiles = {};
        let outputPath = path.resolve(to, options.buildPath);
        fs.ensureDirSync(outputPath);
        for (let fileobject of project.getFiles()) {
            let file = fileobject.file;
            if (file.endsWith('.cpp') || file.endsWith('.c') || file.endsWith('.cc')) {
                let name = file.toLowerCase();
                if (name.indexOf('/') >= 0)
                    name = name.substr(name.lastIndexOf('/') + 1);
                name = name.substr(0, name.lastIndexOf('.'));
                if (!objects[name]) {
                    objects[name] = true;
                    ofiles[file] = name;
                }
                else {
                    while (objects[name]) {
                        name = name + '_';
                    }
                    objects[name] = true;
                    ofiles[file] = name;
                }
            }
        }
        let gchfilelist = '';
        let precompiledHeaders = [];
        for (let file of project.getFiles()) {
            if (file.options && file.options.pch && precompiledHeaders.indexOf(file.options.pch) < 0) {
                precompiledHeaders.push(file.options.pch);
            }
        }
        for (let file of project.getFiles()) {
            let precompiledHeader = null;
            for (let header of precompiledHeaders) {
                if (file.file.endsWith(header)) {
                    precompiledHeader = header;
                    break;
                }
            }
            if (precompiledHeader !== null) {
                // let realfile = path.relative(outputPath, path.resolve(from, file.file));
                gchfilelist += path.basename(file.file) + '.gch ';
            }
        }
        let ofilelist = '';
        for (let o in objects) {
            ofilelist += o + '.o ';
        }
        this.writeFile(path.resolve(outputPath, 'makefile'));
        let incline = '-I./ '; // local directory to pick up the precompiled header hxcpp.h.gch
        for (let inc of project.getIncludeDirs()) {
            inc = path.relative(outputPath, path.resolve(from, inc));
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
        if (!options.debug)
            optimization = '-O2';
        this.p(project.getName() + ': ' + gchfilelist + ofilelist);
        let cpp = '';
        if (project.cpp11) {
            cpp = '-std=c++11';
        }
        this.p('\tg++ ' + cpp + ' ' + optimization + ' ' + ofilelist + ' -o "' + project.getName() + '" $(LIB)');
        for (let file of project.getFiles()) {
            let precompiledHeader = null;
            for (let header of precompiledHeaders) {
                if (file.file.endsWith(header)) {
                    precompiledHeader = header;
                    break;
                }
            }
            if (precompiledHeader !== null) {
                let realfile = path.relative(outputPath, path.resolve(from, file.file));
                this.p(path.basename(realfile) + '.gch: ' + realfile);
                let compiler = 'g++';
                this.p('\t' + compiler + ' ' + cpp + ' ' + optimization + ' $(INC) $(DEF) -c ' + realfile + ' -o ' + path.basename(file.file) + '.gch $(LIB)');
            }
        }
        for (let fileobject of project.getFiles()) {
            let file = fileobject.file;
            if (file.endsWith('.c') || file.endsWith('.cpp') || file.endsWith('cc')) {
                this.p();
                let name = ofiles[file];
                let realfile = path.relative(outputPath, path.resolve(from, file));
                this.p(name + '.o: ' + realfile);
                let compiler = 'g++';
                if (file.endsWith('.c'))
                    compiler = 'gcc';
                this.p('\t' + compiler + ' ' + cpp + ' ' + optimization + ' $(INC) $(DEF) -c ' + realfile + ' -o ' + name + '.o $(LIB)');
            }
        }
        // project.getDefines()
        // project.getIncludeDirs()
        this.closeFile();
    }
}
exports.MakefileExporter = MakefileExporter;
//# sourceMappingURL=MakefileExporter.js.map