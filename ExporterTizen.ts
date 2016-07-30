import {Exporter} from './Exporter';
import {Solution} from './Solution';
import * as fs from 'fs-extra';
import * as path from 'path';

export class ExporterTizen extends Exporter {
	constructor() {
		super();
	}

	exportSolution(solution: Solution, from: string, to: string, platform: string) {
		let project = solution.getProjects()[0];

		if (project.getDebugDir() !== '') fs.copySync(path.resolve(from, project.getDebugDir()), path.resolve(to, 'data'));

		var dotcproject = fs.readFileSync(path.resolve(__dirname, 'Data', 'tizen', '.cproject'), 'utf8');
		dotcproject = dotcproject.replace(/{ProjectName}/g, solution.getName());
		var includes = '';
		for (var i in project.getIncludeDirs()) {
			var include = project.getIncludeDirs()[i];
			includes += "<listOptionValue builtIn=\"false\" value=\"&quot;${workspace_loc:/${ProjName}/CopiedSources/" + include + "}&quot;\"/>";
		}
		dotcproject = dotcproject.replace(/{includes}/g, includes);
		var defines = '';
		for (var d in project.getDefines()) {
			var define = project.getDefines()[d];
			defines += "<listOptionValue builtIn=\"false\" value=\"" + define + "\"/>";
		}
		dotcproject = dotcproject.replace(/{defines}/g, defines);
		fs.writeFileSync(path.resolve(to, '.cproject'), dotcproject);

		var dotproject = fs.readFileSync(path.resolve(__dirname, 'Data', 'tizen', '.project'), 'utf8');
		dotproject = dotproject.replace(/{ProjectName}/g, solution.getName());
		fs.writeFileSync(path.resolve(to, '.project'), dotproject);

		var manifest = fs.readFileSync(path.resolve(__dirname, 'Data', 'tizen', 'manifest.xml'), 'utf8');
		manifest = manifest.replace(/{ProjectName}/g, solution.getName());
		fs.writeFileSync(path.resolve(to, 'manifest.xml'), manifest);

		for (var f in project.getFiles()) {
			var file = project.getFiles()[f].file;
			var target = path.resolve(to, 'CopiedSources', file);
			fs.ensureDirSync(path.join(target.substr(0, target.lastIndexOf('/'))));
			fs.copySync(path.resolve(from, file), target);
		}
	}
}
