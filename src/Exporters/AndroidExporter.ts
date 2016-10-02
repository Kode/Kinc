import {Exporter} from './Exporter';
import {Project} from '../Project';
import * as Icon from '../Icon';
import {execSync} from 'child_process';
import * as fs from 'fs-extra';
import * as os from 'os';
import * as path from 'path';

export class AndroidExporter extends Exporter {
	safename: string;

	constructor() {
		super();
	}

	exportSolution(project: Project, from: string, to: string, platform: string, vr: any) {
		let safename = project.getName().replace(/ /g, '-');
		this.safename = safename;
		let targetOptions = {
			package: 'com.ktxsoftware.kore',
			screenOrientation: 'sensor'
		};
		if (project.targetOptions != null && project.targetOptions.android != null) {
			let userOptions = project.targetOptions.android;
			if (userOptions.package != null) targetOptions.package = userOptions.package;
			if (userOptions.screenOrientation != null) targetOptions.screenOrientation = userOptions.screenOrientation;
		}

		const indir = path.join(__dirname, '..', '..', 'Data', 'android');
		const outdir = path.join(to.toString(), safename);

		fs.copySync(path.join(indir, 'gitignore'), path.join(outdir, '.gitignore'));
		fs.copySync(path.join(indir, 'build.gradle'), path.join(outdir, 'build.gradle'));
		fs.copySync(path.join(indir, 'gradle.properties'), path.join(outdir, 'gradle.properties'));
		fs.copySync(path.join(indir, 'gradlew'), path.join(outdir, 'gradlew'));
		fs.copySync(path.join(indir, 'gradlew.bat'), path.join(outdir, 'gradlew.bat'));
		fs.copySync(path.join(indir, 'settings.gradle'), path.join(outdir, 'settings.gradle'));

		fs.ensureDirSync(path.join(outdir, 'app'));

		let gradle = fs.readFileSync(path.join(indir, 'app', 'build.gradle'), {encoding: 'utf8'});
		gradle = gradle.replace(/{package}/g, targetOptions.package);

		let cppflags = '-frtti -fexceptions';
		if (project.cpp11) {
			cppflags = '-std=c++11 ' + cppflags;
		}
		gradle = gradle.replace(/{cppflags}/g, cppflags);

		let javasources = '';
		for (let dir of project.getJavaDirs()) {
			javasources += '\'' + path.relative(path.join(outdir, 'app'), path.resolve(from, dir)).replace(/\\/g, '/') + '\', ';
		}
		javasources += '\'' + path.relative(path.join(outdir, 'app'), path.join(Project.koreDir.toString(), 'Backends', 'Android', 'Java-Sources')).replace(/\\/g, '/') + '\'';
		gradle = gradle.replace(/{javasources}/g, javasources);

		fs.writeFileSync(path.join(outdir, 'app', 'build.gradle'), gradle, {encoding: 'utf8'});

		let cmake = fs.readFileSync(path.join(indir, 'app', 'CMakeLists.txt'), {encoding: 'utf8'});

		let defines = '';
		for (let def of project.getDefines()) {
			defines += '  -D' + def + '\n';
		}
		cmake = cmake.replace(/{defines}/g, defines);

		let includes = '';
		for (let inc of project.getIncludeDirs()) {
			includes += '  "' + path.resolve(inc).replace(/\\/g, '/') + '"\n';
		}
		cmake = cmake.replace(/{includes}/g, includes);

		let files = '';
		for (let file of project.getFiles()) {
			if (file.file.endsWith('.c') || file.file.endsWith('.cc') || file.file.endsWith('.cpp') || file.file.endsWith('.h')) {
				files += '  "' + path.resolve(file.file).replace(/\\/g, '/') + '"\n';
			}
		}
		cmake = cmake.replace(/{files}/g, files);

		let libraries1 = '';
		let libraries2 = '';
		for (let lib of project.getLibs()) {
			libraries1 += 'find_library(' + lib + '-lib ' + lib + ')\n';
			libraries2 += '  ${' + lib + '-lib}\n';
		}
		cmake = cmake.replace(/{libraries1}/g, libraries1).replace(/{libraries2}/g, libraries2);

		fs.writeFileSync(path.join(outdir, 'app', 'CMakeLists.txt'), cmake, {encoding: 'utf8'});

		fs.copySync(path.join(indir, 'app', 'proguard-rules.pro'), path.join(outdir, 'app', 'proguard-rules.pro'));

		fs.ensureDirSync(path.join(outdir, 'app', 'src'));
		// fs.emptyDirSync(path.join(outdir, 'app', 'src'));

		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main'));

		let manifest = fs.readFileSync(path.join(indir, 'main', 'AndroidManifest.xml'), {encoding: 'utf8'});
		manifest = manifest.replace(/{package}/g, targetOptions.package);
		manifest = manifest.replace(/{screenOrientation}/g, targetOptions.screenOrientation);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main'));
		fs.writeFileSync(path.join(outdir, 'app', 'src', 'main', 'AndroidManifest.xml'), manifest, {encoding: 'utf8'});

		let strings = fs.readFileSync(path.join(indir, 'main', 'res', 'values', 'strings.xml'), {encoding: 'utf8'});
		strings = strings.replace(/{name}/g, project.getName());
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'values'));
		fs.writeFileSync(path.join(outdir, 'app', 'src', 'main', 'res', 'values', 'strings.xml'), strings, {encoding: 'utf8'});

		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-hdpi'));
		Icon.exportPng(path.resolve(to, safename, 'app', 'src', 'main', 'res', 'mipmap-hdpi', 'ic_launcher.png'), 72, 72, undefined, from);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-mdpi'));
		Icon.exportPng(path.resolve(to, safename, 'app', 'src', 'main', 'res', 'mipmap-mdpi', 'ic_launcher.png'), 48, 48, undefined, from);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-xhdpi'));
		Icon.exportPng(path.resolve(to, safename, 'app', 'src', 'main', 'res', 'mipmap-xhdpi', 'ic_launcher.png'), 96, 96, undefined, from);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-xxhdpi'));
		Icon.exportPng(path.resolve(to, safename, 'app', 'src', 'main', 'res', 'mipmap-xxhdpi', 'ic_launcher.png'), 144, 144, undefined, from);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-xxhdpi'));
		Icon.exportPng(path.resolve(to, safename, 'app', 'src', 'main', 'res', 'mipmap-xxxhdpi', 'ic_launcher.png'), 192, 192, undefined, from);

		fs.copySync(path.join(indir, 'gradle', 'wrapper', 'gradle-wrapper.jar'), path.join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.jar'));
		fs.copySync(path.join(indir, 'gradle', 'wrapper', 'gradle-wrapper.properties'), path.join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.properties'));

		fs.copySync(path.join(indir, 'idea', 'compiler.xml'), path.join(outdir, '.idea', 'compiler.xml'));
		fs.copySync(path.join(indir, 'idea', 'gradle.xml'), path.join(outdir, '.idea', 'gradle.xml'));
		fs.copySync(path.join(indir, 'idea', 'misc.xml'), path.join(outdir, '.idea', 'misc.xml'));
		let modules = fs.readFileSync(path.join(indir, 'idea', 'modules.xml'), {encoding: 'utf8'});
		modules = modules.replace(/{name}/g, safename);
		fs.writeFileSync(path.join(outdir, '.idea', 'modules.xml'), modules, {encoding: 'utf8'});
		fs.copySync(path.join(indir, 'idea', 'runConfigurations.xml'), path.join(outdir, '.idea', 'runConfigurations.xml'));
		fs.copySync(path.join(indir, 'idea', 'copyright', 'profiles_settings.xml'), path.join(outdir, '.idea', 'copyright', 'profiles_settings.xml'));

		if (project.getDebugDir().length > 0) fs.copySync(path.resolve(from, project.getDebugDir()), path.resolve(to, safename, 'app', 'src', 'main', 'assets'));
	}
}
