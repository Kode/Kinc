"use strict";
const Exporter_1 = require('./Exporter');
const fs = require('fs-extra');
const path = require('path');
class TizenExporter extends Exporter_1.Exporter {
    constructor() {
        super();
    }
    exportSolution(project, from, to, platform) {
        if (project.getDebugDir() !== '')
            fs.copySync(path.resolve(from, project.getDebugDir()), path.resolve(to, 'data'), { clobber: true });
        let dotcproject = fs.readFileSync(path.resolve(__dirname, 'Data', 'tizen', '.cproject'), 'utf8');
        dotcproject = dotcproject.replace(/{ProjectName}/g, project.getName());
        let includes = '';
        for (let include of project.getIncludeDirs()) {
            includes += '<listOptionValue builtIn="false" value="&quot;${workspace_loc:/${ProjName}/CopiedSources/' + include + '}&quot;"/>';
        }
        dotcproject = dotcproject.replace(/{includes}/g, includes);
        let defines = '';
        for (let define of project.getDefines()) {
            defines += '<listOptionValue builtIn="false" value="' + define + '"/>';
        }
        dotcproject = dotcproject.replace(/{defines}/g, defines);
        fs.writeFileSync(path.resolve(to, '.cproject'), dotcproject);
        let dotproject = fs.readFileSync(path.resolve(__dirname, 'Data', 'tizen', '.project'), 'utf8');
        dotproject = dotproject.replace(/{ProjectName}/g, project.getName());
        fs.writeFileSync(path.resolve(to, '.project'), dotproject);
        let manifest = fs.readFileSync(path.resolve(__dirname, 'Data', 'tizen', 'manifest.xml'), 'utf8');
        manifest = manifest.replace(/{ProjectName}/g, project.getName());
        fs.writeFileSync(path.resolve(to, 'manifest.xml'), manifest);
        for (let file of project.getFiles()) {
            let target = path.resolve(to, 'CopiedSources', file);
            fs.ensureDirSync(path.join(target.substr(0, target.lastIndexOf('/'))));
            fs.copySync(path.resolve(from, file), target, { clobber: true });
        }
    }
}
exports.TizenExporter = TizenExporter;
//# sourceMappingURL=TizenExporter.js.map