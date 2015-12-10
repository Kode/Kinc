"use strict";

const Exporter = require('./Exporter.js');
const Files = require('./Files.js');
const Paths = require('./Paths.js');
const Project = require('./Project.js');
const Icon = require('./Icon.js');
const fs = require('fs-extra');
const path = require('path');

class ExporterAndroid extends Exporter {
	constructor() {
		super();
	}

	exportSolution(solution, from, to, platform, vr) {
		let project = solution.getProjects()[0];
		let safename = solution.getName().replaceAll(' ', '-');
		this.safename = safename;

		const indir = path.join(__dirname, 'Data', 'android');
		const outdir = path.join(to.toString(), safename);

		fs.copySync(path.join(indir, 'build.gradle'), path.join(outdir, 'build.gradle'));
		fs.copySync(path.join(indir, 'gradle.properties'), path.join(outdir, 'gradle.properties'));
		fs.copySync(path.join(indir, 'gradlew'), path.join(outdir, 'gradlew'));
		fs.copySync(path.join(indir, 'gradlew.bat'), path.join(outdir, 'gradlew.bat'));
		fs.copySync(path.join(indir, 'settings.gradle'), path.join(outdir, 'settings.gradle'));

		let nameiml = fs.readFileSync(path.join(indir, 'name.iml'), {encoding: 'utf8'});
		nameiml = nameiml.replaceAll('{name}', safename);
		fs.writeFileSync(path.join(outdir, safename + '.iml'), nameiml, {encoding: 'utf8'});

		fs.copySync(path.join(indir, 'app', 'proguard-rules.pro'), path.join(outdir, 'app', 'proguard-rules.pro'));

		let flags = '\n';
		flags += '        cppFlags += "-fexceptions"\n';
		flags += '        cppFlags += "-frtti"\n';
		for (let def of project.getDefines()) {
			flags += '        cppFlags += "-D' + def + '"\n';
			flags += '        CFlags += "-D' + def + '"\n';
		}
		for (let inc of project.getIncludeDirs()) {
			inc = inc.replaceAll('\\', '/');
			while (inc.startsWith('../')) inc = inc.substr(3);
			flags += '        cppFlags += "-I${file("src/main/jni/' + inc + '")}".toString()\n';
			flags += '        CFlags += "-I${file("src/main/jni/' + inc + '")}".toString()\n';
		}

		let gradle = fs.readFileSync(path.join(indir, 'app', 'build.gradle'), {encoding: 'utf8'});
		gradle = gradle.replaceAll('{name}', safename);
		gradle = gradle.replaceAll('{flags}', flags);

		let javasources = '';
		for (let dir of project.getJavaDirs()) {
			javasources += "                    srcDir '" + path.relative(path.join(outdir, 'app'), from.resolve(dir).toString()).replaceAll('\\', '/') + "'\n";
		}
		javasources += "                    srcDir '" + path.relative(path.join(outdir, 'app'), path.join(Project.koreDir.toString(), 'Backends', 'Android', 'Java-Sources')).replaceAll('\\', '/') + "'\n";
		gradle = gradle.replaceAll('{javasources}', javasources);

		//gradle = gradle.replaceAll('{cppsources}', ''); // Currently at the default position
		fs.writeFileSync(path.join(outdir, 'app', 'build.gradle'), gradle, {encoding: 'utf8'});

		let appiml = fs.readFileSync(path.join(indir, 'app', 'app.iml'), {encoding: 'utf8'});
		appiml = appiml.replaceAll('{name}', safename);
		fs.writeFileSync(path.join(outdir, 'app', 'app.iml'), appiml, {encoding: 'utf8'});

		fs.ensureDirSync(path.join(outdir, 'app', 'src'));
		//fs.emptyDirSync(path.join(outdir, 'app', 'src'));

		fs.copySync(path.join(indir, 'main', 'AndroidManifest.xml'), path.join(outdir, 'app', 'src', 'main', 'AndroidManifest.xml'));

		let strings = fs.readFileSync(path.join(indir, 'main', 'res', 'values', 'strings.xml'), {encoding: 'utf8'});
		strings = strings.replaceAll('{name}', solution.getName());
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'values'));
		fs.writeFileSync(path.join(outdir, 'app', 'src', 'main', 'res', 'values', 'strings.xml'), strings, {encoding: 'utf8'});

		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-hdpi'));
		Icon.exportPng(to.resolve(Paths.get(safename, 'app', 'src', 'main', 'res', 'mipmap-hdpi', "ic_launcher.png")), 72, 72, undefined, from);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-mdpi'));
		Icon.exportPng(to.resolve(Paths.get(safename, 'app', 'src', 'main', 'res', 'mipmap-mdpi', "ic_launcher.png")), 48, 48, undefined, from);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-xhdpi'));
		Icon.exportPng(to.resolve(Paths.get(safename, 'app', 'src', 'main', 'res', 'mipmap-xhdpi', "ic_launcher.png")), 96, 96, undefined, from);
		fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-xxhdpi'));
		Icon.exportPng(to.resolve(Paths.get(safename, 'app', 'src', 'main', 'res', 'mipmap-xxhdpi', "ic_launcher.png")), 144, 144, undefined, from);

		fs.copySync(path.join(indir, 'gradle', 'wrapper', 'gradle-wrapper.jar'), path.join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.jar'));
		fs.copySync(path.join(indir, 'gradle', 'wrapper', 'gradle-wrapper.properties'), path.join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.properties'));

		fs.copySync(path.join(indir, 'idea', 'compiler.xml'), path.join(outdir, '.idea', 'compiler.xml'));
		fs.copySync(path.join(indir, 'idea', 'encodings.xml'), path.join(outdir, '.idea', 'encodings.xml'));
		fs.copySync(path.join(indir, 'idea', 'gradle.xml'), path.join(outdir, '.idea', 'gradle.xml'));
		fs.copySync(path.join(indir, 'idea', 'misc.xml'), path.join(outdir, '.idea', 'misc.xml'));
		fs.copySync(path.join(indir, 'idea', 'runConfigurations.xml'), path.join(outdir, '.idea', 'runConfigurations.xml'));
		fs.copySync(path.join(indir, 'idea', 'vcs.xml'), path.join(outdir, '.idea', 'vcs.xml'));
		fs.copySync(path.join(indir, 'idea', 'copyright', 'profiles_settings.xml'), path.join(outdir, '.idea', 'copyright', 'profiles_settings.xml'));

		let namename = fs.readFileSync(path.join(indir, 'idea', 'name'), {encoding: 'utf8'});
		namename = namename.replaceAll('{name}', solution.getName());
		fs.writeFileSync(path.join(outdir, '.idea', '.name'), namename, {encoding: 'utf8'});

		let modules = fs.readFileSync(path.join(indir, 'idea', 'modules.xml'), {encoding: 'utf8'});
		modules = modules.replaceAll('{name}', safename);
		fs.writeFileSync(path.join(outdir, '.idea', 'modules.xml'), modules, {encoding: 'utf8'});

		if (project.getDebugDir().length > 0) this.copyDirectory(from.resolve(project.getDebugDir()), to.resolve(Paths.get(safename, 'app', 'src', 'main', 'assets')));

		for (let file of project.getFiles()) {
			let localFile = file;
			while (localFile.startsWith('../')) localFile = localFile.substr(3);
			let target = to.resolve(Paths.get(safename, 'app', 'src', 'main', 'jni')).resolve(localFile);
			this.createDirectory(Paths.get(target.path.substr(0, target.path.lastIndexOf('/'))));
			Files.copyIfDifferent(from.resolve(file), target, true);
		}
	}

	exportSolutionEclipse(solution, from, to, platform, vr) {
		const nvpack = false;
		let project = solution.getProjects()[0];
		//String libname = solution.getName().toLowerCase().replace(' ', '-');

		if (project.getDebugDir().length > 0) this.copyDirectory(from.resolve(project.getDebugDir()), to.resolve("assets"));
		if (vr === 'cardboard') {
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "classpath.Cardboard")), to.resolve(".classpath"), true);
		}
		else {
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "classpath")), to.resolve(".classpath"), true);
		}

		let file = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "android", "project")).toString(), {encoding: 'utf8'});
		file = file.replaceAll("{ProjectName}", solution.getName());
		if (Project.koreDir.toString() != "") file = file.replaceAll("{Java-Sources}", Project.koreDir.resolve(Paths.get("Backends", "Android", "Java-Sources")).toAbsolutePath().toString().replaceAll('\\', '/'));
		if (Project.koreDir.toString() != "") file = file.replaceAll("{Android-Backend-Sources}", Project.koreDir.resolve(Paths.get("Backends", "Android", "Sources")).toAbsolutePath().toString().replaceAll('\\', '/'));
		if (Project.koreDir.toString() != "") file = file.replaceAll("{OpenGL-Backend-Sources}", Project.koreDir.resolve(Paths.get("Backends", "OpenGL2", "Sources")).toAbsolutePath().toString().replaceAll('\\', '/'));
		if (Project.koreDir.toString() != "") file = file.replaceAll("{Kore-Sources}", Project.koreDir.resolve(Paths.get("Sources")).toAbsolutePath().toString().replaceAll('\\', '/'));
		fs.writeFileSync(to.resolve('.project').toString(), file);

		file = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "android", "cproject")).toString(), {encoding: 'utf8'});
		file = file.replaceAll("{ProjectName}", solution.getName());
		fs.writeFileSync(to.resolve('.cproject').toString(), file);

		if (vr === 'gearvr') {
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "AndroidManifest.GearVr.xml")), to.resolve("AndroidManifest.xml"), true);
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "project.GearVr.properties")), to.resolve("project.properties"), true);
		}
		else if (vr === 'cardboard') {
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "AndroidManifest.Cardboard.xml")), to.resolve("AndroidManifest.xml"), true);
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "project.Cardboard.properties")), to.resolve("project.properties"), true);
		}
		else {
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "AndroidManifest.xml")), to.resolve("AndroidManifest.xml"), true);
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "project.properties")), to.resolve("project.properties"), true);
		}
		this.createDirectory(to.resolve(".settings"));
		if (nvpack) {
			Files.copy(Paths.executableDir().resolve(Paths.get("Data", "android", "nvidia", "org.eclipse.jdt.core.prefs")), to.resolve(Paths.get(".settings", "org.eclipse.jdt.core.prefs")), true);
		}
		else {
			Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "org.eclipse.jdt.core.prefs")), to.resolve(Paths.get(".settings", "org.eclipse.jdt.core.prefs")), true);
		}

		if (nvpack) {
			Files.copy(Paths.executableDir().resolve(Paths.get("Data", "android", "nvidia", "build.xml")), to.resolve("build.xml"), true);
		}

		this.createDirectory(to.resolve("res"));
		this.createDirectory(to.resolve(Paths.get("res", "values")));
		file = fs.readFileSync(Paths.executableDir().resolve(Paths.get("Data", "android", "strings.xml")).toString(), {encoding: 'utf8'});
		file = file.replaceAll("{ProjectName}", solution.getName());
		fs.writeFileSync(to.resolve(Paths.get("res", "values", "strings.xml")).toString(), file);

		this.createDirectory(to.resolve("jni"));

		this.writeFile(to.resolve(Paths.get("jni", "Android_temp.mk")));
		this.p("LOCAL_PATH := $(call my-dir)");
		this.p();
		this.p("include $(CLEAR_VARS)");
		this.p();
		if (vr === 'gearvr') this.p("include ../../../VRLib/import_vrlib.mk		# import VRLib for this module.  Do NOT call $(CLEAR_VARS) until after building your module.");
		if (vr === 'gearvr') this.p("# use += instead of := when defining the following variables: LOCAL_LDLIBS, LOCAL_CFLAGS, LOCAL_C_INCLUDES, LOCAL_STATIC_LIBRARIES");

		this.p("LOCAL_MODULE    := Kore");
		let files = "";
		for (let filename of project.getFiles()) {
			if (filename.endsWith(".c") || filename.endsWith(".cpp") || filename.endsWith(".cc") || filename.endsWith(".s")) files += to.resolve('jni').relativize(from.resolve(filename)).toString().replaceAll('\\', '/') + " ";
		}
		this.p("LOCAL_SRC_FILES := " + files);
		let defines = "";
		for (let def of project.getDefines()) defines += "-D" + def.replaceAll('\"', "\\\"") + " ";
		if (vr === 'gearvr') {
			this.p("LOCAL_CFLAGS += " + defines);
		}
		else {
			this.p("LOCAL_CFLAGS := " + defines);
		}
		let includes = "";
		for (let inc of project.getIncludeDirs()) includes += "$(LOCAL_PATH)/" + to.resolve('jni').relativize(from.resolve(inc)).toString().replaceAll('\\', '/') + " ";
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
		Files.copyIfDifferent(to.resolve(Paths.get("jni", "Android_temp.mk")), to.resolve(Paths.get("jni", "Android.mk")), true);

		//writeFile(to.resolve(Paths::get("jni", "Application.mk")));
		//p("APP_CPPFLAGS += -fexceptions -frtti");
		//p("APP_STL := gnustl_static");
		////p("APP_ABI := all");
		//p("APP_ABI := armeabi-v7a");
		////p("LOCAL_ARM_NEON := true");
		//closeFile();

		Files.copyIfDifferent(Paths.executableDir().resolve(Paths.get("Data", "android", "Application.mk")), to.resolve(Paths.get("jni", "Application.mk")), true);

		//for (var f in project.getFiles()) {
		//	var file = project.getFiles()[f];
		//	var target = to.resolve("jni").resolve(file);
		//	this.createDirectory(Paths.get(target.path.substr(0, target.path.lastIndexOf('/'))));
		//	Files.copyIfDifferent(from.resolve(file), target, true);
		//}
	}
}

module.exports = ExporterAndroid;
