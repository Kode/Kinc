"use strict";

import * as fs from 'fs';
import * as path from 'path';
import {GraphicsApi} from './GraphicsApi';
import {Options} from './Options';
import {Platform} from './Platform';
import {Project} from './Project';

function getDefines(platform, rotated) {
	let defines = [];
	switch (platform) {
		case Platform.Windows:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_WINDOWS");
			break;
		case Platform.WindowsApp:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_WINDOWSAPP");
			break;
		case Platform.PlayStation3:
			defines.push("SYS_PS3");
			break;
		case Platform.iOS:
			if (rotated) defines.push("ROTATE90");
			defines.push("SYS_IOS");
			break;
		case Platform.tvOS:
			defines.push("SYS_TVOS");
			break;
		case Platform.OSX:
			defines.push("SYS_OSX");
			defines.push("SYS_64BIT");
			break;
		case Platform.Android:
			if (rotated) defines.push("ROTATE90");
			defines.push("SYS_ANDROID");
			break;
		case Platform.Xbox360:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_XBOX360");
			break;
		case Platform.HTML5:
			defines.push("SYS_HTML5");
			break;
		case Platform.Linux:
			defines.push("SYS_LINUX");
			break;
		case Platform.Tizen:
			defines.push("SYS_TIZEN");
			break;
	}
	return defines;
}

let scriptdir = '.';

export class Solution {
	static scriptdir: string;
	static platform: string;
	name: string;
	rotated: boolean;
	cmd: boolean;
	projects: Project[];

	constructor(name: string) {
		this.name = name;
		this.rotated = false;
		this.cmd = false;
		this.projects = [];
	}

	getName() {
		return this.name;
	}

	getProjects() {
		return this.projects;
	}

	addProject(project) {
		this.projects.push(project);
	}

	searchFiles() {
		for (let p of this.projects) p.searchFiles(undefined);
	}

	flatten() {
		for (let p of this.projects) p.flatten();
	};

	static createProject(filename: string) {
		let file = fs.readFileSync(path.resolve(Solution.scriptdir, filename, 'korefile.js'), {encoding: 'utf8'});
		let oldscriptdir = Solution.scriptdir;
		Solution.scriptdir = path.resolve(Solution.scriptdir, filename);
		let project = new Function('Project', 'Platform', 'platform', 'GraphicsApi', 'graphics', file)(Project, Platform, Solution.platform, GraphicsApi, Options.graphicsApi);
		Solution.scriptdir = oldscriptdir;
		return project;
	}

	static createSolution(filename, platform) {
		let file = fs.readFileSync(path.resolve(Solution.scriptdir, filename, 'korefile.js'), {encoding: 'utf8'});
		let oldscriptdir = Solution.scriptdir;
		Solution.scriptdir = path.resolve(Solution.scriptdir, filename);
		let solution = new Function(
			'Solution',
			'Project',
			'Platform',
			'platform',
			'GraphicsApi',
			'graphics',
			'fs',
			'path', file)
		(Solution,
			Project,
			Platform,
			platform,
			GraphicsApi,
			Options.graphicsApi,
			require('fs'),
			require('path'));
		Solution.scriptdir = oldscriptdir;

		if (fs.existsSync(path.join(Solution.scriptdir.toString(), 'Backends'))) {
			var libdirs = fs.readdirSync(path.join(Solution.scriptdir.toString(), 'Backends'));
			for (var ld in libdirs) {
				var libdir = path.join(Solution.scriptdir.toString().toString(), 'Backends', libdirs[ld]);
				if (fs.statSync(libdir).isDirectory()) {
					var korefile = path.join(libdir, 'korefile.js');
					if (fs.existsSync(korefile)) {
						solution.projects[0].addSubProject(Solution.createProject(libdir));
					}
				}
			}
		}

		return solution;
	}

	static evalProjectScript(script) {

	}

	static evalSolutionScript(script, platform) {
		this.platform = platform;
	}

	static create(directory: string, platform: string) {
		Solution.scriptdir = directory;
		Solution.platform = platform;
		var solution = Solution.createSolution('.', platform);
		var defines = getDefines(platform, solution.isRotated());
		for (var p in solution.projects) {
			for (var d in defines) solution.projects[p].addDefine(defines[d]);
		}
		return solution;
	}

	isRotated() {
		return this.rotated;
	}

	isCmd() {
		return this.cmd;
	}

	setRotated() {
		this.rotated = true;
	}

	setCmd() {
		this.cmd = true;
	}
}
