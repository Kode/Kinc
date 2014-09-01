var Exporter = require('./Exporter.js');
var Files = require('./Files.js');
var Icon = require('./Icon.js');
var Paths = require('./Paths.js');
var Platform = require('./Platform.js');
var fs = require('fs');
var uuid = require('./uuid.js');

function contains(a, b) {
	return a.indexOf(b) !== -1;
}

function newId() {
	return uuid.v4().toUpperCase();
}

var Directory = function (dirname) {
	this.dirname = dirname;
	this.id = newId();
};

Directory.prototype.getName = function () {
	return this.dirname;
};

Directory.prototype.getLastName = function () {
	if (!contains(this.dirname, '/')) return this.dirname;
	return this.dirname.substr(this.dirname.lastIndexOf('/') + 1);
};

Directory.prototype.getId = function () {
	return this.id;
};

var File = function (filename, dir) {
	this.filename = filename;
	this.dir = dir;
	this.buildid = newId();
	this.fileid = newId();
};

File.prototype.getBuildId = function () {
	return this.buildid;
};

File.prototype.getFileId = function () {
	return this.fileid;
};

File.prototype.isBuildFile = function () {
	return this.filename.endsWith(".c") || this.filename.endsWith(".cpp") || this.filename.endsWith(".m") || this.filename.endsWith(".mm") || this.filename.endsWith(".cc") || this.filename.endsWith(".s");
};

File.prototype.getName = function () {
	return this.filename;
};

File.prototype.getLastName = function () {
	if (!contains(this.filename, '/')) return this.filename;
	return this.filename.substr(this.filename.lastIndexOf('/') + 1);
};

File.prototype.getDir = function () {
	return this.dir;
};

File.prototype.toString = function () {
	return this.getName();
};

var Framework = function (name) {
	this.name = name;
	this.buildid = newId();
	this.fileid = newId();
};

Framework.prototype.toString = function () {
	return this.name + ".framework";
};

Framework.prototype.getBuildId = function () {
	return this.buildid.toString().toUpperCase();
};

Framework.prototype.getFileId = function () {
	return this.fileid.toString().toUpperCase();
};

function findDirectory(dirname, directories) {
	for (var d in directories) {
		var dir = directories[d];
		if (dir.getName() === dirname) {
			return dir;
		}
	}
	return null;
}

function addDirectory(dirname, directories) {
	var dir = findDirectory(dirname, directories);
	if (dir === null) {
		dir = new Directory(dirname);
		directories.push(dir);
		while (contains(dirname, '/')) {
			dirname = dirname.substr(0, dirname.lastIndexOf('/'));
			addDirectory(dirname, directories);
		}
	}
	return dir;
}

function ExporterXCode() {

}

ExporterXCode.prototype = Object.create(Exporter.prototype);
ExporterXCode.constructor = ExporterXCode;

ExporterXCode.prototype.exportWorkspace = function (to, solution) {
	var dir = to.resolve(Paths.get(solution.getName() + ".xcodeproj", "project.xcworkspace"));
	if (!Files.exists(dir)) Files.createDirectories(dir);
	
	this.writeFile(to.resolve(Paths.get(solution.getName() + ".xcodeproj", "project.xcworkspace", "contents.xcworkspacedata")));
	
	this.p("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	this.p("<Workspace");
	this.p("version = \"1.0\">");
	this.p("<FileRef");
	this.p("location = \"self:" + solution.getName() + ".xcodeproj\">");
	this.p("</FileRef>");
	this.p("</Workspace>");
	
	this.closeFile();
};

ExporterXCode.prototype.exportSolution = function (solution, from, to, platform) {
	var xdir = to.resolve(solution.getName() + ".xcodeproj");
	if (!Files.exists(xdir)) Files.createDirectories(xdir);

	this.exportWorkspace(to, solution);

	var iosIconNames = [];
	iosIconNames.push("iPhone.png");
	iosIconNames.push("iPhoneRetina.png");
	iosIconNames.push("Default.png");
	iosIconNames.push("Default@2x.png");
	iosIconNames.push("Default-568h@2x.png");
	iosIconNames.push("iPad.png");
	iosIconNames.push("iPadRetina.png");
	iosIconNames.push("Default-Portrait~ipad.png");
	iosIconNames.push("Default-Portrait@2x~ipad.png");
	iosIconNames.push("Default-Landscape~ipad.png");
	iosIconNames.push("Default-Landscape@2x~ipad.png");

	if (platform === Platform.iOS) {
		var black = 0xff;
		Icon.exportPng(to.resolve("iPhone.png"                   ),   57,   57, undefined, from);
		Icon.exportPng(to.resolve("iPhoneRetina.png"             ),  114,  114, undefined, from);
		Icon.exportPng(to.resolve("Default.png"                  ),  320,  480, black,       from);
		Icon.exportPng(to.resolve("Default@2x.png"               ),  640,  960, black,       from);
		Icon.exportPng(to.resolve("Default-568h@2x.png"          ),  640, 1136, black,       from);
		Icon.exportPng(to.resolve("iPad.png"                     ),   72,   72, undefined, from);
		Icon.exportPng(to.resolve("iPadRetina.png"               ),  144,  144, undefined, from);
		Icon.exportPng(to.resolve("Default-Portrait~ipad.png"    ),  768, 1024, black,       from);
		Icon.exportPng(to.resolve("Default-Portrait@2x~ipad.png" ), 1536, 2048, black,       from);
		Icon.exportPng(to.resolve("Default-Landscape~ipad.png"   ), 1024,  768, black,       from);
		Icon.exportPng(to.resolve("Default-Landscape@2x~ipad.png"), 2048, 1536, black,       from);
	}
	else {
		Icon.exportIcns(to.resolve("icon.icns"), from);
	}

	var project = solution.getProjects()[0];
	var plistname = '';
	var files = [];
	var directories = [];
	for (var f in project.getFiles()) {
		var filename = project.getFiles()[f];
		if (filename.endsWith(".plist")) plistname = filename;

		var dirname = '';
		if (contains(filename, '/')) dirname = solution.getName() + "/" + filename.substr(0, filename.lastIndexOf('/'));
		else dirname = solution.getName();
		var dir = addDirectory(dirname, directories);

		var file = new File(filename, dir);
		files.push(file);
	}
	if (plistname.length === 0) throw "no plist found";

	var frameworks = [];
	for (var lib in project.getLibs()) {
		frameworks.push(new Framework(project.getLibs()[lib]));
	}

	var projectId = newId();
	var appFileId = newId();
	var frameworkBuildId = newId();
	var sourceBuildId = newId();
	var frameworksGroupId = newId();
	var productsGroupId = newId();
	var mainGroupId = newId();
	var targetId = newId();
	var nativeBuildConfigListId = newId();
	var projectBuildConfigListId = newId();
	var debugId = newId();
	var releaseId = newId();
	var nativeDebugId = newId();
	var nativeReleaseId = newId();
	var debugDirFileId = newId();
	var debugDirBuildId = newId();
	var resourcesBuildId = newId();
	var iconFileId = newId();
	var iconBuildId = newId();
	var iosIconFileIds = [];
	var iosIconBuildIds = [];
	for (var i = 0; i < iosIconNames.length; ++i) {
		iosIconFileIds.push(newId());
		iosIconBuildIds.push(newId());
	}

	this.writeFile(to.resolve(Paths.get(solution.getName() + ".xcodeproj", "project.pbxproj")));

	this.p("// !$*UTF8*$!");
	this.p("{");
	this.p("archiveVersion = 1;", 1);
	this.p("classes = {", 1);
	this.p("};", 1);
	this.p("objectVersion = 46;", 1);
	this.p("objects = {", 1);
	this.p();
	this.p("/* Begin PBXBuildFile section */");
	for (var f in frameworks) {
		var framework = frameworks[f];
		this.p(framework.getBuildId() + " /* " + framework.toString() + " in Frameworks */ = {isa = PBXBuildFile; fileRef = " + framework.getFileId() + " /* " + framework.toString() + " */; };", 2);
	}
	this.p(debugDirBuildId + " /* Deployment in Resources */ = {isa = PBXBuildFile; fileRef = " + debugDirFileId + " /* Deployment */; };", 2);
	for (var f in files) {
		var file = files[f];
		if (file.isBuildFile()) {
			this.p(file.getBuildId() + " /* " + file.toString() + " in Sources */ = {isa = PBXBuildFile; fileRef = " + file.getFileId() + " /* " + file.toString() + " */; };", 2);
		}
	}
	if (platform === Platform.OSX)
		this.p(iconBuildId + " /* icon.icns in Resources */ = {isa = PBXBuildFile; fileRef = " + iconFileId + " /* icon.icns */; };", 2);
	else {
		for (var i = 0; i < iosIconNames.length; ++i) {
			this.p(iosIconBuildIds[i] + " /* " + iosIconNames[i] + " in Resources */ = {isa = PBXBuildFile; fileRef = " + iosIconFileIds[i] + " /* " + iosIconNames[i] + " */; };", 2);
		}
	}
	this.p("/* End PBXBuildFile section */");
	this.p();
	this.p("/* Begin PBXFileReference section */");
	this.p(appFileId + " /* " + solution.getName() + ".app */ = {isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = " + solution.getName() + ".app; sourceTree = BUILT_PRODUCTS_DIR; };", 2);

	for (var f in frameworks) {
		var framework = frameworks[f];
		this.p(framework.getFileId() + " /* " + framework.toString() + " */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = " + framework.toString() + "; path = System/Library/Frameworks/" + framework.toString() + "; sourceTree = SDKROOT; };", 2);
	}
	this.p(debugDirFileId + " /* Deployment */ = {isa = PBXFileReference; lastKnownFileType = folder; name = Deployment; path = \"" + from.resolve(project.getDebugDir()).toAbsolutePath().toString() + "\"; sourceTree = \"<group>\"; };", 2);
	for (var f in files) {
		var file = files[f];
		var filetype = "unknown";
		if (file.getName().endsWith(".plist")) filetype = "text.plist.xml";
		if (file.getName().endsWith(".h")) filetype = "sourcecode.c.h";
		if (file.getName().endsWith(".m")) filetype = "sourcecode.c.objc";
		if (file.getName().endsWith(".c")) filetype = "sourcecode.c.c";
		if (file.getName().endsWith(".cpp")) filetype = "sourcecode.c.cpp";
		if (file.getName().endsWith(".cc")) filetype = "sourcecode.c.cpp";
		if (file.getName().endsWith(".mm")) filetype = "sourcecode.c.objcpp";
		if (file.getName().endsWith(".s")) filetype = "sourcecode.asm";
		this.p(file.getFileId() + " /* " + file.toString() + " */ = {isa = PBXFileReference; lastKnownFileType = " + filetype + "; name = \"" + file.getLastName() + "\"; path = \"" + from.resolve(file.toString()).toAbsolutePath().toString() + "\"; sourceTree = \"<group>\"; };", 2);
	}
	if (platform === Platform.OSX)
		this.p(iconFileId + " /* icon.icns */ = {isa = PBXFileReference; lastKnownFileType = image.icns; path = icon.icns; sourceTree = \"<group>\"; };", 2);
	else {
		for (var i = 0; i < iosIconNames.length; ++i) {
			this.p(iosIconFileIds[i] + " /* " + iosIconNames[i] + " */ = {isa = PBXFileReference; lastKnownFileType = image.icns; path = \"" + iosIconNames[i] + "\"; sourceTree = \"<group>\"; };", 2);
		}
	}
	this.p("/* End PBXFileReference section */");
	this.p();
	this.p("/* Begin PBXFrameworksBuildPhase section */");
	this.p(frameworkBuildId + " /* Frameworks */ = {", 2);
		this.p("isa = PBXFrameworksBuildPhase;", 3);
		this.p("buildActionMask = 2147483647;", 3);
		this.p("files = (", 3);
			for (var f in frameworks) {
				var framework = frameworks[f];
				this.p(framework.getBuildId() + " /* " + framework.toString() + " in Frameworks */,", 4);
			}
		this.p(");", 3);
		this.p("runOnlyForDeploymentPostprocessing = 0;", 3);
	this.p("};", 2);
	this.p("/* End PBXFrameworksBuildPhase section */");
	this.p();
	this.p("/* Begin PBXGroup section */");
	this.p(mainGroupId + " = {", 2);
		this.p("isa = PBXGroup;", 3);
		this.p("children = (", 3);
			if (platform === Platform.OSX) this.p(iconFileId + " /* icon.icns */,", 4);
			else {
				for (var i = 0; i < iosIconNames.length; ++i) {
					this.p(iosIconFileIds[i] + " /* " + iosIconNames[i] + " */,", 4);
				}
			}
			this.p(debugDirFileId + " /* Deployment */,", 4);
			//p(solutionGroupId + " /* " + solution.getName() + " */,", 4);
			for (var d in directories) {
				var dir = directories[d];
				if (!contains(dir.getName(), '/')) this.p(dir.getId() + " /* " + dir.getName() + " */,", 4);
			}
			this.p(frameworksGroupId + " /* Frameworks */,", 4);
			this.p(productsGroupId + " /* Products */,", 4);
		this.p(");", 3);
		this.p("sourceTree = \"<group>\";", 3);
	this.p("};", 2);
	this.p(productsGroupId + " /* Products */ = {", 2);
		this.p("isa = PBXGroup;", 3);
		this.p("children = (", 3);
			this.p(appFileId + " /* " + solution.getName() + ".app */,", 4);
		this.p(");", 3);
		this.p("name = Products;", 3);
		this.p("sourceTree = \"<group>\";", 3);
	this.p("};", 2);
	this.p(frameworksGroupId + " /* Frameworks */ = {", 2);
		this.p("isa = PBXGroup;", 3);
		this.p("children = (", 3);
			for (var f in frameworks) {
				var framework = frameworks[f];
				this.p(framework.getFileId() + " /* " + framework.toString() + " */,", 4);
			}
		this.p(");", 3);
		this.p("name = Frameworks;", 3);
		this.p("sourceTree = \"<group>\";", 3);
	this.p("};", 2);
	for (var d in directories) {
		var dir = directories[d];
		this.p(dir.getId() + " /* " + dir.getName() + " */ = {", 2);
			this.p("isa = PBXGroup;", 3);
			this.p("children = (", 3);
				for (var d2 in directories) {
					var dir2 = directories[d2];
					if (dir2 == dir) continue;
					if (dir2.getName().startsWith(dir.getName())) {
						if (!contains(dir2.getName().substr(dir.getName().length + 1), '/'))
							this.p(dir2.getId() + " /* " + dir2.getName() + " */,", 4);
					}
				}
				for (var f in files) {
					var file = files[f];
					if (file.getDir() === dir) this.p(file.getFileId() + " /* " + file.toString() + " */,", 4);
				}
			this.p(");", 3);
			if (!contains(dir.getName(), '/')) {
				this.p("path = ../;", 3);
				this.p("name = " + dir.getLastName() + ";", 3);
			}
			else this.p("name = " + dir.getLastName() + ";", 3);
			this.p("sourceTree = \"<group>\";", 3);
		this.p("};", 2);
	}
	this.p("/* End PBXGroup section */");
	this.p();
	this.p("/* Begin PBXNativeTarget section */");
	this.p(targetId + " /* " + solution.getName() + " */ = {", 2);
		this.p("isa = PBXNativeTarget;", 3);
		this.p("buildConfigurationList = " + nativeBuildConfigListId + " /* Build configuration list for PBXNativeTarget \"" + solution.getName() + "\" */;", 3);
		this.p("buildPhases = (", 3);
			this.p(sourceBuildId + " /* Sources */,", 4);
			this.p(frameworkBuildId + " /* Frameworks */,", 4);
			this.p(resourcesBuildId + " /* Resources */,", 4);
		this.p(");", 3);
		this.p("buildRules = (", 3);
		this.p(");", 3);
		this.p("dependencies = (", 3);
		this.p(");", 3);
		this.p("name = " + solution.getName() + ";", 3);
		this.p("productName = " + solution.getName() + ";", 3);
		this.p("productReference = " + appFileId + " /* " + solution.getName() + ".app */;", 3);
		this.p("productType = \"com.apple.product-type." + (solution.isCmd() ? "tool" : "application") + "\";", 3);
	this.p("};", 2);
	this.p("/* End PBXNativeTarget section */");
	this.p();
	this.p("/* Begin PBXProject section */");
	this.p(projectId + " /* Project object */ = {", 2);
		this.p("isa = PBXProject;", 3);
		this.p("attributes = {", 3);
			this.p("LastUpgradeCheck = 0430;", 4);
			this.p("ORGANIZATIONNAME = \"KTX Software Development\";", 4);
		this.p("};", 3);
		this.p("buildConfigurationList = " + projectBuildConfigListId + " /* Build configuration list for PBXProject \"" + solution.getName() + "\" */;", 3);
		this.p("compatibilityVersion = \"Xcode 3.2\";", 3);
		this.p("developmentRegion = English;", 3);
		this.p("hasScannedForEncodings = 0;", 3);
		this.p("knownRegions = (", 3);
			this.p("en,", 4);
		this.p(");", 3);
		this.p("mainGroup = " + mainGroupId + ";", 3);
		this.p("productRefGroup = " + productsGroupId + " /* Products */;", 3);
		this.p("projectDirPath = \"\";", 3);
		this.p("projectRoot = \"\";", 3);
		this.p("targets = (", 3);
			this.p(targetId + " /* " + solution.getName() + " */,", 4);
		this.p(");", 3);
	this.p("};", 2);
	this.p("/* End PBXProject section */");
	this.p();
	this.p("/* Begin PBXResourcesBuildPhase section */");
	this.p(resourcesBuildId + " /* Resources */ = {", 2);
		this.p("isa = PBXResourcesBuildPhase;", 3);
		this.p("buildActionMask = 2147483647;", 3);
		this.p("files = (", 3);
			this.p(debugDirBuildId + " /* Deployment in Resources */,", 4);
			if (platform === Platform.OSX) this.p(iconBuildId + " /* icon.icns in Resources */,", 4);
			else {
				for (var i = 0; i < iosIconNames.length; ++i) {
					this.p(iosIconBuildIds[i] + " /* " + iosIconNames[i] + " in Resources */,", 4);
				}
			}
		this.p(");", 3);
		this.p("runOnlyForDeploymentPostprocessing = 0;", 3);
	this.p("};", 2);
	this.p("/* End PBXResourcesBuildPhase section */");
	this.p();
	this.p("/* Begin PBXSourcesBuildPhase section */");
	this.p(sourceBuildId + " /* Sources */ = {", 2);
		this.p("isa = PBXSourcesBuildPhase;", 3);
		this.p("buildActionMask = 2147483647;", 3);
		this.p("files = (", 3);
			for (var f in files) {
				var file = files[f];
				if (file.isBuildFile())
					this.p(file.getBuildId() + " /* " + file.toString() + " in Sources */,", 4);
			}
		this.p(");", 3);
		this.p("runOnlyForDeploymentPostprocessing = 0;");
	this.p("};");
	this.p("/* End PBXSourcesBuildPhase section */");
	this.p();
	//p("/* Begin PBXVariantGroup section */");
	//p("E1FC77F013DAA40000D635AE /* InfoPlist.strings */ = {", 2);
	//	p("isa = PBXVariantGroup;", 3);
	//	p("children = (", 3);
	//		p("E1FC77F113DAA40000D635AE /* en */,", 4);
	//	p(");", 3);
	//	p("name = InfoPlist.strings;", 3);
	//	p("sourceTree = \"<group>\";", 3);
	//p("};", 2);
	//p("E1FC77F913DAA40000D635AE /* MainWindow.xib */ = {", 2);
	//	p("isa = PBXVariantGroup;", 3);
	//	p("children = (", 3);
	//		p("E1FC77FA13DAA40000D635AE /* en */,", 4);
	//	p(");", 3);
	//	p("name = MainWindow.xib;", 3);
	//	p("sourceTree = \"<group>\";", 3);
	//p("};", 2);
	//p("E1FC780613DAA40000D635AE /* TestViewController.xib */ = {", 2);
	//	p("isa = PBXVariantGroup;", 3);
	//	p("children = (", 3);
	//		p("E1FC780713DAA40000D635AE /* en */,", 4);
	//	p(");", 3);
	//	p("name = TestViewController.xib;", 3);
	//	p("sourceTree = \"<group>\";", 3);
	//p("};", 2);
	//p("/* End PBXVariantGroup section */");
	//p();
	this.p("/* Begin XCBuildConfiguration section */");
	this.p(debugId + " /* Debug */ = {", 2);
		this.p("isa = XCBuildConfiguration;", 3);
		this.p("buildSettings = {", 3);
			this.p("ALWAYS_SEARCH_USER_PATHS = NO;", 4);
			if (platform === Platform.iOS) {
				this.p("CLANG_CXX_LANGUAGE_STANDARD = \"gnu++0x\";", 4);
				this.p("CLANG_CXX_LIBRARY = \"libc++\";", 4);
				this.p("CLANG_WARN_EMPTY_BODY = YES;", 4);
				this.p("CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;", 4);
			}
			else this.p("ARCHS = \"$(ARCHS_STANDARD_64_BIT)\";", 4);
			//p("CLANG_ENABLE_OBJC_ARC = YES;", 4); //default, but incompatible with MacOS < 10.6 and iOS < 4.0
			if (platform === Platform.iOS) this.p("\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\" = \"iPhone Developer\";", 4);
			this.p("COPY_PHASE_STRIP = NO;", 4);
			this.p("GCC_C_LANGUAGE_STANDARD = gnu99;", 4);
			this.p("GCC_DYNAMIC_NO_PIC = NO;", 4);
			if (platform === Platform.OSX) this.p("GCC_ENABLE_OBJC_EXCEPTIONS = YES;", 4);
			this.p("GCC_OPTIMIZATION_LEVEL = 0;", 4);
			this.p("GCC_PREPROCESSOR_DEFINITIONS = (", 4);
				this.p("\"DEBUG=1\",", 5);
				for (var d in project.getDefines()) {
					var define = project.getDefines()[d];
					if (contains(define, '=')) p("\"" + define.replaceAll('\"', "\\\\\\\"") + "\",", 5);
					else this.p(define + ",", 5);
				}
				this.p("\"$(inherited)\",", 5);
			this.p(");", 4);
			this.p("GCC_SYMBOLS_PRIVATE_EXTERN = NO;", 4);
			this.p("GCC_VERSION = com.apple.compilers.llvm.clang.1_0;", 4);
			if (platform === Platform.OSX) this.p("GCC_WARN_64_TO_32_BIT_CONVERSION = YES;", 4);
			this.p("GCC_WARN_ABOUT_RETURN_TYPE = YES;", 4);
			this.p("GCC_WARN_UNINITIALIZED_AUTOS = YES;", 4);
			this.p("GCC_WARN_UNUSED_VARIABLE = YES;", 4);
			if (platform === Platform.iOS) {
				this.p("IPHONEOS_DEPLOYMENT_TARGET = 5.0;", 4);
				this.p("ONLY_ACTIVE_ARCH = YES;", 4);
				this.p("SDKROOT = iphoneos;", 4);
				this.p("TARGETED_DEVICE_FAMILY = \"1,2\";", 4);
			}
			else {
				this.p("MACOSX_DEPLOYMENT_TARGET = 10.5;", 4);
				this.p("ONLY_ACTIVE_ARCH = YES;", 4);
				this.p("SDKROOT = macosx;", 4);
			}
		this.p("};", 3);
		this.p("name = Debug;", 3);
	this.p("};", 2);
	this.p(releaseId + " /* Release */ = {", 2);
		this.p("isa = XCBuildConfiguration;", 3);
		this.p("buildSettings = {", 3);
			this.p("ALWAYS_SEARCH_USER_PATHS = NO;", 4);
			if (platform === Platform.iOS) {
				this.p("CLANG_CXX_LANGUAGE_STANDARD = \"gnu++0x\";", 4);
				this.p("CLANG_CXX_LIBRARY = \"libc++\";", 4);
				this.p("CLANG_WARN_EMPTY_BODY = YES;", 4);
				this.p("CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;", 4);
			}
			else this.p("ARCHS = \"$(ARCHS_STANDARD_64_BIT)\";", 4);
			//p("CLANG_ENABLE_OBJC_ARC = YES;", 4); //default, but incompatible with MacOS < 10.6 and iOS < 4.0
			if (platform === Platform.iOS) this.p("\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\" = \"iPhone Developer\";", 4);
			this.p("COPY_PHASE_STRIP = YES;", 4);
			if (platform === Platform.OSX) this.p("DEBUG_INFORMATION_FORMAT = \"dwarf-with-dsym\";", 4);
			this.p("GCC_C_LANGUAGE_STANDARD = gnu99;", 4);
			if (platform === Platform.OSX) this.p("GCC_ENABLE_OBJC_EXCEPTIONS = YES;", 4);
			this.p("GCC_PREPROCESSOR_DEFINITIONS = (", 4);
			for (var d in project.getDefines()) {
				var define = project.getDefines()[d];
				if (contains(define, '=')) this.p("\"" + define.replaceAll('\"', "\\\\\\\"") + "\",", 5);
				else this.p(define + ",", 5);
			}
			this.p("\"$(inherited)\",", 5);
			this.p(");", 4);
			this.p("GCC_VERSION = com.apple.compilers.llvm.clang.1_0;", 4);
			if (platform === Platform.OSX) this.p("GCC_WARN_64_TO_32_BIT_CONVERSION = YES;", 4);
			this.p("GCC_WARN_ABOUT_RETURN_TYPE = YES;", 4);
			this.p("GCC_WARN_UNINITIALIZED_AUTOS = YES;", 4);
			this.p("GCC_WARN_UNUSED_VARIABLE = YES;", 4);
			if (platform === Platform.iOS) {
				this.p("IPHONEOS_DEPLOYMENT_TARGET = 5.0;", 4);
				this.p("OTHER_CFLAGS = \"-DNS_BLOCK_ASSERTIONS=1\";", 4);
				this.p("SDKROOT = iphoneos;", 4);
				this.p("TARGETED_DEVICE_FAMILY = \"1,2\";", 4);
				this.p("VALIDATE_PRODUCT = YES;", 4);
			}
			else {
				this.p("MACOSX_DEPLOYMENT_TARGET = 10.5;", 4);
				this.p("SDKROOT = macosx;", 4);
			}
		this.p("};", 3);
		this.p("name = Release;", 3);
	this.p("};", 2);
	this.p(nativeDebugId + " /* Debug */ = {", 2);
		this.p("isa = XCBuildConfiguration;", 3);
		this.p("buildSettings = {", 3);
			//p("GCC_PRECOMPILE_PREFIX_HEADER = YES;", 4);
			//p("GCC_PREFIX_HEADER = \"Test/Test-Prefix.pch\";", 4);
			if (platform === Platform.iOS) {
				this.p("\"GCC_PREPROCESSOR_DEFINITIONS[arch=*]\" = (", 4);
					this.p("\"DEBUG=1\",", 5);
					for (var d in project.getDefines()) {
						var define = project.getDefines()[d];
						if (contains(define, '=')) this.p("\"" + define.replaceAll('\"', "\\\"") + "\",", 5);
						else this.p(define + ",", 5);
					}
					this.p("\"$(inherited)\",", 5);
				this.p(");", 4);
				this.p("\"GCC_THUMB_SUPPORT[arch=armv6]\" = \"\";", 4);
			}
			this.p("HEADER_SEARCH_PATHS = (", 4);
				for (var path in project.getIncludeDirs()) this.p(from.resolve(project.getIncludeDirs()[path]).toAbsolutePath().toString() + ",", 5);
			this.p(");", 4);
			this.p("INFOPLIST_FILE = \"" + from.resolve(plistname).toAbsolutePath().toString() + "\";", 4);
			this.p("PRODUCT_NAME = \"$(TARGET_NAME)\";", 4);
			this.p("WRAPPER_EXTENSION = app;", 4);
		this.p("};", 3);
		this.p("name = Debug;", 3);
	this.p("};", 2);
	this.p(nativeReleaseId + " /* Release */ = {", 2);
		this.p("isa = XCBuildConfiguration;", 3);
		this.p("buildSettings = {", 3);
			//p("GCC_PRECOMPILE_PREFIX_HEADER = YES;", 4);
			//p("GCC_PREFIX_HEADER = \"Test/Test-Prefix.pch\";", 4);
			if (platform === Platform.iOS) {
				this.p("\"GCC_PREPROCESSOR_DEFINITIONS[arch=*]\" = (", 4);
					for (var d in project.getDefines()) {
						var define = project.getDefines()[d];
						if (contains(define, '=')) this.p("\"" + replace(define, '\"', "\\\"") + "\",", 5);
						else this.p(define + ",", 5);
					}
					this.p("\"$(inherited)\",", 5);
				this.p(");", 4);
				this.p("\"GCC_THUMB_SUPPORT[arch=armv6]\" = \"\";", 4);
			}
			this.p("HEADER_SEARCH_PATHS = (", 4);
				for (var path in project.getIncludeDirs()) this.p(from.resolve(project.getIncludeDirs()[path]).toAbsolutePath().toString() + ",", 5);
			this.p(");", 4);
			this.p("INFOPLIST_FILE = \"" + from.resolve(plistname).toAbsolutePath().toString() + "\";", 4);
			this.p("PRODUCT_NAME = \"$(TARGET_NAME)\";", 4);
			this.p("WRAPPER_EXTENSION = app;", 4);
		this.p("};", 3);
		this.p("name = Release;", 3);
	this.p("};", 2);
	this.p("/* End XCBuildConfiguration section */");
	this.p();
	this.p("/* Begin XCConfigurationList section */");
	this.p(projectBuildConfigListId + " /* Build configuration list for PBXProject \"" + solution.getName() + "\" */ = {", 2);
		this.p("isa = XCConfigurationList;", 3);
		this.p("buildConfigurations = (", 3);
			this.p(debugId + " /* Debug */,", 4);
			this.p(releaseId + " /* Release */,", 4);
		this.p(");", 3);
		this.p("defaultConfigurationIsVisible = 0;", 3);
		this.p("defaultConfigurationName = Release;", 3);
	this.p("};", 2);
	this.p(nativeBuildConfigListId + " /* Build configuration list for PBXNativeTarget \"" + solution.getName() + "\" */ = {", 2);
		this.p("isa = XCConfigurationList;", 3);
		this.p("buildConfigurations = (", 3);
			this.p(nativeDebugId + " /* Debug */,", 4);
			this.p(nativeReleaseId + " /* Release */,", 4);
		this.p(");", 3);
		this.p("defaultConfigurationIsVisible = 0;", 3);
	this.p("};", 2);
	this.p("/* End XCConfigurationList section */");
this.p("};", 1);
this.p("rootObject = " + projectId + " /* Project object */;", 1);
	this.p("}");
	this.closeFile();
};

module.exports = ExporterXCode;
