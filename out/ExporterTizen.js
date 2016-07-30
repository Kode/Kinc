"use strict";
const Exporter_1 = require('./Exporter');
const fs = require('fs-extra');
class ExporterTizen extends Exporter_1.Exporter {
    constructor() {
        super();
    }
    exportSolution(solution, from, to, platform) {
        let project = solution.getProjects()[0];
        if (project.getDebugDir() !== '')
            fs.copyDirectorySync(from.resolve(project.getDebugDir()), to.resolve("data"));
        var dotcproject = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "tizen", ".cproject")).toString(), { encoding: 'utf8' });
        dotcproject = dotcproject.replaceAll("{ProjectName}", solution.getName());
        var includes = '';
        for (var i in project.getIncludeDirs()) {
            var include = project.getIncludeDirs()[i];
            includes += "<listOptionValue builtIn=\"false\" value=\"&quot;${workspace_loc:/${ProjName}/CopiedSources/" + include + "}&quot;\"/>";
        }
        dotcproject = dotcproject.replaceAll("{includes}", includes);
        var defines = '';
        for (var d in project.getDefines()) {
            var define = project.getDefines()[d];
            defines += "<listOptionValue builtIn=\"false\" value=\"" + define + "\"/>";
        }
        dotcproject = dotcproject.replaceAll("{defines}", defines);
        fs.writeFileSync(to.resolve('.cproject').toString(), dotcproject);
        var dotproject = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "tizen", ".project")).toString(), { encoding: 'utf8' });
        dotproject = dotproject.replaceAll("{ProjectName}", solution.getName());
        fs.writeFileSync(to.resolve('.project').toString(), dotproject);
        var manifest = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "tizen", "manifest.xml")).toString(), { encoding: 'utf8' });
        manifest = manifest.replaceAll("{ProjectName}", solution.getName());
        fs.writeFileSync(to.resolve('manifest.xml').toString(), manifest);
        for (var f in project.getFiles()) {
            var file = project.getFiles()[f].file;
            var target = to.resolve("CopiedSources").resolve(file);
            this.createDirectory(Paths.get(target.path.substr(0, target.path.lastIndexOf('/'))));
            Files.copy(from.resolve(file), target, true);
        }
    }
}
exports.ExporterTizen = ExporterTizen;
//# sourceMappingURL=ExporterTizen.js.map