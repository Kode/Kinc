"use strict";

let Exporter = require('./Exporter.js');
let Files = require('./Files.js');
let Paths = require('./Paths.js');
let fs = require('fs');

class ExporterTizen extends Exporter {
	constructor() {
		super();
	}

	exportSolution(solution, from, to, platform) {
		let project = solution.getProjects()[0];

		if (project.getDebugDir() !== '') this.copyDirectory(from.resolve(project.getDebugDir()), to.resolve("data"));

		var dotcproject = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "tizen", ".cproject")).toString(), {encoding: 'utf8'});
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

		var dotproject = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "tizen", ".project")).toString(), {encoding: 'utf8'});
		dotproject = dotproject.replaceAll("{ProjectName}", solution.getName());
		fs.writeFileSync(to.resolve('.project').toString(), dotproject);

		var manifest = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "tizen", "manifest.xml")).toString(), {encoding: 'utf8'});
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

module.exports = ExporterTizen;
