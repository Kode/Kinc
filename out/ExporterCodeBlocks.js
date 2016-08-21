"use strict";
const Exporter_1 = require('./Exporter');
const Platform_1 = require('./Platform');
const path = require('path');
class ExporterCodeBlocks extends Exporter_1.Exporter {
    constructor() {
        super();
    }
    exportSolution(solution, from, to, platform) {
        let project = solution.getProjects()[0];
        this.writeFile(path.resolve(to, project.getName() + '.cbp'));
        this.p("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>");
        this.p("<CodeBlocks_project_file>");
        this.p("<FileVersion major=\"1\" minor=\"6\" />", 1);
        this.p("<Project>", 1);
        this.p("<Option title=\"" + project.getName() + "\" />", 2);
        this.p("<Option pch_mode=\"2\" />", 2);
        this.p("<Option compiler=\"gcc\" />", 2);
        this.p("<Build>", 2);
        this.p("<Target title=\"Debug\">", 3);
        this.p("<Option output=\"bin/Debug/" + project.getName() + "\" prefix_auto=\"1\" extension_auto=\"1\" />", 4);
        if (project.getDebugDir().length > 0)
            this.p("<Option working_dir=\"" + path.resolve(from, project.getDebugDir()) + "\" />", 4);
        this.p("<Option object_output=\"obj/Debug/\" />", 4);
        this.p("<Option type=\"1\" />", 4);
        this.p("<Option compiler=\"gcc\" />", 4);
        this.p("<Compiler>", 4);
        if (project.cpp11) {
            this.p("<Add option=\"-std=c++11\" />", 5);
        }
        this.p("<Add option=\"-g\" />", 5);
        this.p("</Compiler>", 4);
        this.p("</Target>", 3);
        this.p("<Target title=\"Release\">", 3);
        this.p("<Option output=\"bin/Release/" + project.getName() + "\" prefix_auto=\"1\" extension_auto=\"1\" />", 4);
        if (project.getDebugDir().length > 0)
            this.p("<Option working_dir=\"" + path.resolve(from, project.getDebugDir()) + "\" />", 4);
        this.p("<Option object_output=\"obj/Release/\" />", 4);
        this.p("<Option type=\"0\" />", 4);
        this.p("<Option compiler=\"gcc\" />", 4);
        this.p("<Compiler>", 4);
        if (project.cpp11) {
            this.p("<Add option=\"-std=c++11\" />", 5);
        }
        this.p("<Add option=\"-O2\" />", 5);
        this.p("</Compiler>", 4);
        this.p("<Linker>", 4);
        this.p("<Add option=\"-s\" />", 5);
        this.p("</Linker>", 4);
        this.p("</Target>", 3);
        this.p("</Build>", 2);
        this.p("<Compiler>", 2);
        if (project.cpp11) {
            this.p("<Add option=\"-std=c++11\" />", 3);
        }
        this.p("<Add option=\"-Wall\" />", 3);
        for (let def of project.getDefines()) {
            this.p("<Add option=\"-D" + def.replace(/\"/g, "\\\"") + "\" />", 3);
        }
        for (let inc of project.getIncludeDirs()) {
            this.p("<Add directory=\"" + path.resolve(from, inc) + "\" />", 3);
        }
        this.p("</Compiler>", 2);
        this.p("<Linker>", 2);
        this.p("<Add option=\"-pthread\" />", 3);
        this.p("<Add option=\"-static-libgcc\" />", 3);
        this.p("<Add option=\"-static-libstdc++\" />", 3);
        for (let lib of project.getLibs()) {
            this.p('<Add library="' + lib + '" />', 3);
        }
        if (platform === Platform_1.Platform.Pi) {
            this.p("<Add directory=\"/opt/vc/lib\" />", 3);
        }
        this.p("</Linker>", 2);
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
            if (file.file.endsWith(".c") || file.file.endsWith(".cc") || file.file.endsWith(".cpp")) {
                this.p("<Unit filename=\"" + path.resolve(from, file.file) + "\">", 2);
                this.p("<Option compilerVar=\"CC\" />", 3);
                this.p("</Unit>", 2);
            }
            else if (file.file.endsWith('.h')) {
                this.p("<Unit filename=\"" + path.resolve(from, file.file) + "\">", 2);
                if (precompiledHeader !== null) {
                    this.p('<Option compile="1" />', 3);
                    this.p('<Option weight="0" />', 3);
                }
                this.p("</Unit>", 2);
            }
        }
        this.p("<Extensions>", 2);
        this.p("<code_completion />", 3);
        this.p("<debugger />", 3);
        this.p("</Extensions>", 2);
        this.p("</Project>", 1);
        this.p("</CodeBlocks_project_file>");
        this.closeFile();
    }
}
exports.ExporterCodeBlocks = ExporterCodeBlocks;
//# sourceMappingURL=ExporterCodeBlocks.js.map