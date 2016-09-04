import {Exporter} from './Exporter';
import {Project} from '../Project';
import * as child_process from 'child_process';
import * as fs from 'fs-extra';
import * as os from 'os';
import * as path from 'path';

let emmccPath = 'emcc';
let defines = '';
let includes = '';
let definesArray: string[] = [];
let includesArray: string[] = [];

function link(files: string[], output: string) {
	var params: string[] = [];//-O2 ";
	for (let file of files) {
		//console.log(files[file]);
		params.push(file);
	}
	params.push("-o");
	params.push(output);

	console.log("Linking " + output);

	let res = child_process.spawnSync(emmccPath, params);
	if (res != null) {
		if (res.stdout != null && res.stdout.toString() != '')
			console.log("stdout: " + res.stdout);
		if (res.stderr != null && res.stderr.toString() != '')
			console.log("stderr: " + res.stderr);
		if (res.error != null)
			console.log(res.error);
	}
}

export class EmscriptenExporter extends Exporter {
	constructor() {
		super();
	}

	compile(inFilename: string, outFilename: string) {
		if (fs.existsSync(outFilename)) return;

		//console.log("Compiling " + inFilename + " to " + outFilename);
		//console.log(emmccPath + " " + inFilename+ " " +includes+ " " +defines+ " " +"-c -o"+ " " +outFilename);

		//console.log(emmccPath + " " + inFilename+ " " +  includes+ " " +  defines+ " " + "-c -o"+ " " + outFilename);

		let params: string[] = [];
		params.push(inFilename);

		for (let i = 0; i < includesArray.length; i++) {
			params.push(includesArray[i]);
		}

		for (let i = 0; i < definesArray.length; i++) {
			params.push(definesArray[i]);
		}

		params.push("-c");
		params.push("-o");
		params.push(outFilename);

		//console.log(params);

		let res = child_process.spawnSync(emmccPath, params, {stdio: 'inherit'});
		if (res != null) {
			if (res.stdout != null && res.stdout.toString() != '')
				console.log("stdout: " + res.stdout);
			if (res.stderr != null && res.stderr.toString() != '')
				console.log("stderr: " + res.stderr);
			if (res.error != null)
				console.log(res.error);
		}
	}

	exportSolution(project: Project, from: string, to: string, platform: string) {
		let debugDirName = project.getDebugDir();
		debugDirName = debugDirName.replace(/\\/g, '/');
		if (debugDirName.endsWith('/')) debugDirName = debugDirName.substr(0, debugDirName.length - 1);
		if (debugDirName.lastIndexOf('/') >= 0) debugDirName = debugDirName.substr(debugDirName.lastIndexOf('/') + 1);
		
		fs.copySync(path.resolve(from, debugDirName), path.resolve(to, debugDirName), { clobber: true });
		
		defines = "";
		definesArray = [];
		for (let def in project.getDefines()) {
			defines += "-D" + project.getDefines()[def] + " ";
			definesArray.push("-D" + project.getDefines()[def]);
		}
		defines += '-D KORE_DEBUGDIR="\\"' + debugDirName + '\\""' + ' ';
		definesArray.push('-D KORE_DEBUGDIR="\\"' + debugDirName + '\\""');

		includes = "";
		includesArray = [];
		for (let inc in project.getIncludeDirs()) {
			includes += "-I../" + path.resolve(from, project.getIncludeDirs()[inc]) + " ";
			includesArray.push("-I../" + path.resolve(from, project.getIncludeDirs()[inc]));
		}

		this.writeFile(path.resolve(to, 'makefile'));

		this.p();
		let oline = '';
		for (let fileobject of project.getFiles()) {
			let filename = fileobject.file;
			if (!filename.endsWith(".cpp") && !filename.endsWith(".c")) continue;
			let lastpoint = filename.lastIndexOf('.');
			let oname = filename.substr(0, lastpoint) + ".o";
			oname = oname.replace(/..\//, '');
			oline += " " + oname;
		}
		
		this.p('kore.html:' + oline);
		this.p('emcc ' + oline + ' -o kore.html --preload-file ' + debugDirName, 1);
		this.p();

		for (let fileobject of project.getFiles()) {
			let filename = fileobject.file;
			if (!filename.endsWith(".cpp") && !filename.endsWith(".c")) continue;
			let builddir = to;
			let dirs = filename.split('/');
			let name = '';
			for (let i = 0; i < dirs.length - 1; ++i) {
				let s = dirs[i];
				if (s == "" || s == "..") continue;
				name += s + "/";
				builddir = path.resolve(builddir, s);
				if (!fs.existsSync(builddir)) fs.ensureDirSync(builddir);
			}
			let lastpoint = filename.lastIndexOf('.');
			let oname = filename.substr(0, lastpoint) + ".o";
			oname = oname.replace(/..\//, '');
			this.p(oname + ": ../" + filename);
			this.p("emcc -c ../" + filename + " " + includes + " " + defines + " -o " + oname, 1);
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


		/*console.log("Compiling files...");
		var objectFiles = [];
		var files = project.getFiles();
		for (let file of files) {
			if (file.endsWith(".c") || file.endsWith(".cpp")) {
				//files += "../../" + filename + " ";
				compile(from.resolve(file).toString(), to.resolve(file + ".o").toString());
				objectFiles.push(to.resolve(file + ".o").toString());
			}
		}
		link(objectFiles, to.resolve(Paths.get("build", "Kt.js").toString()));*/
	}
}
