import {Exporter} from './Exporter';
import {Project} from './Project';
import {Solution} from './Solution';
import * as Icon from './Icon';
import {execSync} from 'child_process';
import * as fs from 'fs-extra';
import * as os from 'os';
import * as path from 'path';

function filesDifferWin(file1: string, file2: string): boolean {
	// Treat them as different if one of them does not exist.
	if (!fs.existsSync(file1)) return true;
	if (!fs.existsSync(file2)) return true;

	let isDifferent = true;
	let output;
	try {
		output = execSync("fc " + file1 + " " + file2, { encoding: 'utf8' });
	}
	catch (error) {
		output = "";
	}
	if(output.indexOf("no differences encountered") > -1) {
		isDifferent = false;
	}
	return isDifferent;
}

function filesDifferUnix(file1: string, file2: string): boolean {
	// Treat them as different if one of them does not exist.
	if (!fs.existsSync(file1)) return true;
	if (!fs.existsSync(file2)) return true;

	let isDifferent = false;
	let output = "";
	try {
		output = execSync("diff " + file1 + " " + file2, { encoding: 'utf8' });
	}
	catch (error) {
		return true;
	}
	if(output != "") {
		isDifferent = true;
	}
	return isDifferent;
}

function copyIfDifferent(from: string, to: string, replace: boolean): void {
	fs.ensureDirSync(path.normalize(path.join(to, '..')));
	if (replace || !fs.existsSync(to)) {
		if (os.platform() === 'win32' && filesDifferWin(to, from)) {
			fs.writeFileSync(to, fs.readFileSync(from));
			//console.log("Copying differing file: " + from.path);
		}
		else if (os.platform() !== 'win32' && filesDifferUnix(to, from)) {
			fs.writeFileSync(to, fs.readFileSync(from));
			//console.log("Copying differing file: " + from.path);
		}
		else {
			//console.log("Skipped file: " + from.path);
		}
	}
}

export class ExporterAndroid extends Exporter {
	safename: string;

	constructor() {
		super();
	}

	exportSolution(solution: Solution, from: string, to: string, platform: string, vr) {
		let project = solution.getProjects()[0];
		let safename = solution.getName().replace(/ /g, '-');
		this.safename = safename;
		let targetOptions = {
			package: 'com.ktxsoftware.kha',
			screenOrientation: 'sensor'
		};
		if (project.targetOptions != null && project.targetOptions.android != null) {
			let userOptions = project.targetOptions.android;
			if (userOptions.package != null) targetOptions.package = userOptions.package;
			if (userOptions.screenOrientation != null) targetOptions.screenOrientation = userOptions.screenOrientation;
		}

		const indir = path.join(__dirname, 'Data', 'android');
		const outdir = path.join(to.toString(), safename);

		fs.copySync(path.join(indir, 'build.gradle'), path.join(outdir, 'build.gradle'));
		//fs.copySync(path.join(indir, 'gradle.properties'), path.join(outdir, 'gradle.properties'));
		fs.copySync(path.join(indir, 'gradlew'), path.join(outdir, 'gradlew'));
		fs.copySync(path.join(indir, 'gradlew.bat'), path.join(outdir, 'gradlew.bat'));
		//fs.copySync(path.join(indir, 'local.properties'), path.join(outdir, 'local.properties'));
		fs.copySync(path.join(indir, 'settings.gradle'), path.join(outdir, 'settings.gradle'));

		let nameiml = fs.readFileSync(path.join(indir, 'name.iml'), {encoding: 'utf8'});
		nameiml = nameiml.replace(/{Name}/g, safename);
		fs.writeFileSync(path.join(outdir, safename + '.iml'), nameiml, {encoding: 'utf8'});

		fs.ensureDirSync(path.join(outdir, 'app'));
		//fs.copySync(path.join(indir, 'app', 'proguard-rules.pro'), path.join(outdir, 'app', 'proguard-rules.pro'));

		let flags = '\n';
        flags += "            cppFlags.add('-std=c++11')\n";
		flags += "            cppFlags.add('-fexceptions')\n";
		flags += "            cppFlags.add('-frtti')\n";

		// Because of https://tls.mbed.org/kb/development/arm-thumb-error-r7-cannot-be-used-in-asm-here
		// TODO: Remove when clang works
		flags += "            cppFlags.add('-fomit-frame-pointer')\n";
		flags += "            CFlags.add('-fomit-frame-pointer')\n";

		for (let def of project.getDefines()) {
			flags += "            cppFlags.add('-D" + def + "')\n";
			flags += "            CFlags.add('-D" + def + "')\n";
		}
		for (let inc of project.getIncludeDirs()) {
			inc = inc.replace(/\\/g, '/');
			while (inc.startsWith('../')) inc = inc.substr(3);
			flags += '            cppFlags.add("-I${file("src/main/jni/' + inc + '")}".toString())\n';
			flags += '            CFlags.add("-I${file("src/main/jni/' + inc + '")}".toString())\n';
		}

		let gradle = fs.readFileSync(path.join(indir, 'app', 'build.gradle'), {encoding: 'utf8'});
		gradle = gradle.replace(/{package}/g, targetOptions.package);
		gradle = gradle.replace(/{flags}/g, flags);

		let javasources = '';
		for (let dir of project.getJavaDirs()) {
			javasources += "                        srcDir '" + path.relative(path.join(outdir, 'app'), path.resolve(from, dir).replace(/\\/g, '/') + "'\n");
		}
		javasources += "                        srcDir '" + path.relative(path.join(outdir, 'app'), path.join(Project.koreDir.toString(), 'Backends', 'Android', 'Java-Sources')).replace(/\\/g, '/') + "'\n";
		gradle = gradle.replace(/{javasources}/g, javasources);

		//gradle = gradle.replaceAll('{cppsources}', ''); // Currently at the default position
		fs.writeFileSync(path.join(outdir, 'app', 'build.gradle'), gradle, {encoding: 'utf8'});

		let appiml = fs.readFileSync(path.join(indir, 'app', 'app.iml'), {encoding: 'utf8'});
		appiml = appiml.replace(/{name}/g, safename);
		fs.writeFileSync(path.join(outdir, 'app', 'app.iml'), appiml, {encoding: 'utf8'});

		fs.ensureDirSync(path.join(outdir, 'app', 'src'));
		//fs.emptyDirSync(path.join(outdir, 'app', 'src'));

		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main'));

		let manifest = fs.readFileSync(path.join(indir, 'main', 'AndroidManifest.xml'), {encoding: 'utf8'});
		manifest = manifest.replace(/{package}/g, targetOptions.package);
		manifest = manifest.replace(/{screenOrientation}/g, targetOptions.screenOrientation);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main'));
		fs.writeFileSync(path.join(outdir, 'app', 'src', 'main', 'AndroidManifest.xml'), manifest, {encoding: 'utf8'});

		let strings = fs.readFileSync(path.join(indir, 'main', 'res', 'values', 'strings.xml'), {encoding: 'utf8'});
		strings = strings.replace(/{name}/g, solution.getName());
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

		fs.copySync(path.join(indir, 'gradle', 'wrapper', 'gradle-wrapper.jar'), path.join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.jar'));
		fs.copySync(path.join(indir, 'gradle', 'wrapper', 'gradle-wrapper.properties'), path.join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.properties'));

		fs.copySync(path.join(indir, 'idea', 'compiler.xml'), path.join(outdir, '.idea', 'compiler.xml'));
		fs.copySync(path.join(indir, 'idea', 'encodings.xml'), path.join(outdir, '.idea', 'encodings.xml'));
		fs.copySync(path.join(indir, 'idea', 'gradle.xml'), path.join(outdir, '.idea', 'gradle.xml'));
		fs.copySync(path.join(indir, 'idea', 'misc.xml'), path.join(outdir, '.idea', 'misc.xml'));
		fs.copySync(path.join(indir, 'idea', 'runConfigurations.xml'), path.join(outdir, '.idea', 'runConfigurations.xml'));
		//fs.copySync(path.join(indir, 'idea', 'vcs.xml'), path.join(outdir, '.idea', 'vcs.xml'));
		fs.copySync(path.join(indir, 'idea', 'copyright', 'profiles_settings.xml'), path.join(outdir, '.idea', 'copyright', 'profiles_settings.xml'));

		let namename = fs.readFileSync(path.join(indir, 'idea', 'name'), {encoding: 'utf8'});
		namename = namename.replace(/{name}/g, solution.getName());
		fs.writeFileSync(path.join(outdir, '.idea', '.name'), namename, {encoding: 'utf8'});

		let modules = fs.readFileSync(path.join(indir, 'idea', 'modules.xml'), {encoding: 'utf8'});
		modules = modules.replace(/{name}/g, safename);
		fs.writeFileSync(path.join(outdir, '.idea', 'modules.xml'), modules, {encoding: 'utf8'});

		if (project.getDebugDir().length > 0) fs.copySync(path.resolve(from, project.getDebugDir()), path.resolve(to, safename, 'app', 'src', 'main', 'assets'));

		for (let file of project.getFiles()) {
			let localFile = file.file;
			while (localFile.startsWith('../')) localFile = localFile.substr(3);
			let target = path.resolve(to, safename, 'app', 'src', 'main', 'jni', localFile);
			fs.ensureDirSync(path.join(target.substr(0, target.lastIndexOf('/'))));
			copyIfDifferent(path.resolve(from, file.file), target, true);
		}
	}

	exportSolutionEclipse(solution: Solution, from: string, to: string, platform: string, vr) {
		const nvpack = false;
		let project = solution.getProjects()[0];
		//String libname = solution.getName().toLowerCase().replace(' ', '-');

		if (project.getDebugDir().length > 0) fs.copySync(path.resolve(from, project.getDebugDir()), path.resolve(to, 'assets'));
		if (vr === 'cardboard') {
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'classpath.Cardboard'), path.resolve(to, '.classpath'), true);
		}
		else {
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'classpath'), path.resolve(to, '.classpath'), true);
		}

		let file = fs.readFileSync(path.resolve(__dirname, 'Data', 'android', 'project'), 'utf8');
		file = file.replace(/{ProjectName}/g, solution.getName());
		if (Project.koreDir.toString() != "") file = file.replace(/{Java-Sources}/g, path.resolve(Project.koreDir, 'Backends', 'Android', 'Java-Sources').replace(/\\/g, '/'));
		if (Project.koreDir.toString() != "") file = file.replace(/{Android-Backend-Sources}/g, path.resolve(Project.koreDir, 'Backends', 'Android', 'Sources').replace(/\\/g, '/'));
		if (Project.koreDir.toString() != "") file = file.replace(/{OpenGL-Backend-Sources}/g, path.resolve(Project.koreDir, 'Backends', 'OpenGL2', 'Sources').replace(/\\/g, '/'));
		if (Project.koreDir.toString() != "") file = file.replace(/{Kore-Sources}/g, path.resolve(Project.koreDir, 'Sources').replace(/\\/g, '/'));
		fs.writeFileSync(path.resolve(to, '.project'), file);

		file = fs.readFileSync(path.resolve(__dirname, 'Data', 'android', 'cproject'), 'utf8');
		file = file.replace(/{ProjectName}/g, solution.getName());
		fs.writeFileSync(path.resolve(to, '.cproject'), file);

		if (vr === 'gearvr') {
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'AndroidManifest.GearVr.xml'), path.resolve(to, 'AndroidManifest.xml'), true);
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'project.GearVr.properties'), path.resolve(to, 'project.properties'), true);
		}
		else if (vr === 'cardboard') {
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'AndroidManifest.Cardboard.xml'), path.resolve(to, 'AndroidManifest.xml'), true);
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'project.Cardboard.properties'), path.resolve(to, 'project.properties'), true);
		}
		else {
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'AndroidManifest.xml'), path.resolve(to, 'AndroidManifest.xml'), true);
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'project.properties'), path.resolve(to, 'project.properties'), true);
		}
		fs.ensureDirSync(path.resolve(to, '.settings'));
		if (nvpack) {
			fs.copySync(path.resolve(__dirname, 'Data', 'android', 'nvidia', 'org.eclipse.jdt.core.prefs'), path.resolve(to, '.settings', 'org.eclipse.jdt.core.prefs'), true);
		}
		else {
			copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'org.eclipse.jdt.core.prefs'), path.resolve(to, '.settings', 'org.eclipse.jdt.core.prefs'), true);
		}

		if (nvpack) {
			fs.copySync(path.resolve(__dirname, 'Data', 'android', 'nvidia', 'build.xml'), path.resolve(to, 'build.xml'));
		}

		fs.ensureDirSync(path.resolve(to, 'res'));
		fs.ensureDirSync(path.resolve(to, 'res', 'values'));
		file = fs.readFileSync(path.resolve(__dirname, 'Data', 'android', 'strings.xml'), 'utf8');
		file = file.replace(/{ProjectName}/g, solution.getName());
		fs.writeFileSync(path.resolve(to, 'res', 'values', 'strings.xml'), file);

		fs.ensureDirSync(path.resolve(to, 'jni'));

		this.writeFile(path.resolve(to, 'jni', 'Android_temp.mk'));
		this.p("LOCAL_PATH := $(call my-dir)");
		this.p();
		this.p("include $(CLEAR_VARS)");
		this.p();
		if (vr === 'gearvr') this.p("include ../../../VRLib/import_vrlib.mk		# import VRLib for this module.  Do NOT call $(CLEAR_VARS) until after building your module.");
		if (vr === 'gearvr') this.p("# use += instead of := when defining the following variables: LOCAL_LDLIBS, LOCAL_CFLAGS, LOCAL_C_INCLUDES, LOCAL_STATIC_LIBRARIES");

		this.p("LOCAL_MODULE    := Kore");
		let files = "";
		for (let fileobject of project.getFiles()) {
			let filename = fileobject.file; 
			if (filename.endsWith(".c") || filename.endsWith(".cpp") || filename.endsWith(".cc") || filename.endsWith(".s")) files += path.relative(path.resolve(to, 'jni'), path.resolve(from, filename)).replace(/\\/g, '/') + " ";
		}
		this.p("LOCAL_SRC_FILES := " + files);
		let defines = "";
		for (let def of project.getDefines()) defines += "-D" + def.replace(/\"/g, "\\\"") + " ";
		if (vr === 'gearvr') {
			this.p("LOCAL_CFLAGS += " + defines);
		}
		else {
			this.p("LOCAL_CFLAGS := " + defines);
		}
		let includes = "";
		for (let inc of project.getIncludeDirs()) includes += "$(LOCAL_PATH)/" + path.relative(path.resolve(to, 'jni'), path.resolve(from, inc)).replace(/\\/g, '/') + " ";
		if (vr === 'gearvr') {
			this.p("LOCAL_C_INCLUDES += " + includes);
			this.p("LOCAL_LDLIBS    += -llog -lGLESv2 -lOpenMAXAL -landroid");
			this.p("LOCAL_CPPFLAGS := -DVR_GEAR_VR");
		}
		else {
			this.p("LOCAL_C_INCLUDES := " + includes);
			this.p("LOCAL_LDLIBS    := -llog -lGLESv2 -lOpenMAXAL -landroid");
		}
		if (vr == "cardboard") {
			this.p("LOCAL_CPPFLAGS := -DVR_CARDBOARD");
		}
		this.p("#LOCAL_SHORT_COMMANDS := true");
		this.p();
		this.p("include $(BUILD_SHARED_LIBRARY)");
		this.p();
		this.closeFile();

		// Check if the file is different from the old one
		copyIfDifferent(path.resolve(to, 'jni', 'Android_temp.mk'), path.resolve(to, 'jni', 'Android.mk'), true);

		//writeFile(to.resolve(Paths::get("jni", "Application.mk")));
		//p("APP_CPPFLAGS += -fexceptions -frtti");
		//p("APP_STL := gnustl_static");
		////p("APP_ABI := all");
		//p("APP_ABI := armeabi-v7a");
		////p("LOCAL_ARM_NEON := true");
		//closeFile();

		copyIfDifferent(path.resolve(__dirname, 'Data', 'android', 'Application.mk'), path.resolve(to, 'jni', 'Application.mk'), true);

		//for (var f in project.getFiles()) {
		//	var file = project.getFiles()[f];
		//	var target = to.resolve("jni").resolve(file);
		//	this.createDirectory(Paths.get(target.path.substr(0, target.path.lastIndexOf('/'))));
		//	Files.copyIfDifferent(from.resolve(file), target, true);
		//}
	}
}
