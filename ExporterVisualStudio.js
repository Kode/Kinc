var Exporter = require('./Exporter.js');
var Files = require('./Files.js');
var GraphicsApi = require('./GraphicsApi.js');
var Icon = require('./Icon.js');
var Paths = require('./Paths.js');
var Platform = require('./Platform.js');
var Project = require('./Project.js');
var Options = require('./Options.js');
var VisualStudioVersion = require('./VisualStudioVersion.js');
var ClCompile = require('./ClCompile.js');
var Configuration = require('./Configuration.js');
var uuid = require('./uuid.js');

var standardconfs = []; // = new String[]{"Debug", "Release"};
var xboxconfs = []; // = new String[]{"CodeAnalysis", "Debug", "Profile_FastCap", "Profile", "Release_LTCG", "Release"};
var windows8systems = []; // = new String[]{"ARM", "Win32", "x64"};
var xboxsystems = []; // = new String[]{"Xbox 360"};
var ps3systems = []; // = new String[]{"PS3"};
var windowssystems = []; // = new String[]{"Win32", "x64"};

function ExporterVisualStudio() {

}

ExporterVisualStudio.prototype = Object.create(Exporter.prototype);
ExporterVisualStudio.constructor = ExporterVisualStudio;

ExporterVisualStudio.prototype.exportUserFile = function (from, to, project, platform) {
	if (project.getDebugDir() === '') return;

	this.writeFile(to.resolve(project.getName() + '.vcxproj.user'));

	this.p("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	this.p("<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");
	this.p("<PropertyGroup>", 1);
	var debugDir = project.getDebugDir();
	if (Paths.get(debugDir).isAbsolute()) {
		debugDir = debugDir.replaceAll('/', '\\');
	}
	else {
		debugDir = from.resolve(debugDir).toAbsolutePath().toString().replaceAll('/', '\\');
	}
	if (platform === Platform.Windows) {
		this.p("<LocalDebuggerWorkingDirectory>" + debugDir + "</LocalDebuggerWorkingDirectory>", 2);
		this.p("<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>", 2);
		//java.io.File baseDir = new File(project.getBasedir());
		//p("<LocalDebuggerCommandArguments>\"SOURCEDIR=" + baseDir.getAbsolutePath() + "\" \"KTSOURCEDIR=" + baseDir.getAbsolutePath() + "\\Kt\"</LocalDebuggerCommandArguments>", 2);
	}
	else if (platform == Platform.PlayStation3) {
		this.p("<LocalDebuggerFileServingDirectory>" + debugDir + "</LocalDebuggerFileServingDirectory>", 2);
		this.p("<DebuggerFlavor>PS3Debugger</DebuggerFlavor>", 2);
	}
	this.p("</PropertyGroup>", 1);
	this.p("</Project>");

	this.closeFile();
};

ExporterVisualStudio.prototype.writeProjectDeclarations = function (project, solutionUuid) {
	this.p("Project(\"{" + solutionUuid.toString().toUpperCase() + "}\") = \"" + project.getName() + "\", \"" + project.getName() + ".vcxproj\", \"{" + project.getUuid().toString().toUpperCase() + "}\"");
	if (project.getSubProjects().length > 0) {
		this.p("ProjectSection(ProjectDependencies) = postProject", 1);
		for (p in project.getSubProjects()) {
			var proj = project.getSubProjects()[p];
			this.p("{" + proj.getUuid().toString().toUpperCase() + "} = {" + proj.getUuid().toString().toUpperCase() + "}", 2);
		}
		this.p("EndProjectSection", 1);
	}
	this.p("EndProject");
	for (proj in project.getSubProjects()) writeProjectDeclarations(project.getSubProjects()[proj], solutionUuid);
};

ExporterVisualStudio.prototype.getConfigs = function (platform) {
	if (platform == Platform.Xbox360) return xboxconfs;
	else return standardconfs;
};

ExporterVisualStudio.prototype.getSystems = function (platform) {
	if (platform == Platform.WindowsApp) return windows8systems;
	if (platform == Platform.PlayStation3) return ps3systems;
	else if (platform == Platform.Xbox360) return xboxsystems;
	else return windowssystems;
};

ExporterVisualStudio.prototype.GetSys = function (platform) {
	return getSystems(platform)[0];
};

ExporterVisualStudio.prototype.writeProjectBuilds = function (project, platform) {
	for (c in this.getConfigs(platform)) {
		var config = this.getConfigs(platform)[c];
		for (s in this.getSystems(platform)) {
			var system = this.getSystems(platform)[s];
			this.p("{" + project.getUuid().toString().toUpperCase() + "}." + config + "|" + system + ".ActiveCfg = " + config + "|" + system, 2);
			this.p("{" + project.getUuid().toString().toUpperCase() + "}." + config + "|" + system + ".Build.0 = " + config + "|" + system, 2);
			if (platform == Platform.WindowsApp) {
				this.p("{" + project.getUuid().toString().toUpperCase() + "}." + config + "|" + system + ".Deploy.0 = " + config + "|" + system, 2);
			}
		}
	}
	for (proj in project.getSubProjects()) writeProjectBuilds(project.getSubProjects()[proj], platform);
};

ExporterVisualStudio.prototype.exportSolution = function (solution, from, to, platform) {
	standardconfs = [];
	standardconfs.push("Debug");
	standardconfs.push("Release");
	xboxconfs = [];
	xboxconfs.push("CodeAnalysis");
	xboxconfs.push("Debug");
	xboxconfs.push("Profile_FastCap");
	xboxconfs.push("Profile");
	xboxconfs.push("Release_LTCG");
	xboxconfs.push("Release");
	windows8systems = [];
	windows8systems.push("ARM");
	windows8systems.push("Win32");
	windows8systems.push("x64");
	xboxsystems = [];
	xboxsystems.push("Xbox 360");
	ps3systems = [];
	ps3systems.push("PS3");
	windowssystems = [];
	windowssystems.push("Win32");
	windowssystems.push("x64");

	this.writeFile(to.resolve(solution.getName() + '.sln'));

	if (platform == Platform.WindowsApp || Options.visualStudioVersion == VisualStudioVersion.VS2015) {
		this.p("Microsoft Visual Studio Solution File, Format Version 12.00");
		this.p("# Visual Studio 2015");
		this.p("VisualStudioVersion = 14.0.22823.1");
		this.p("MinimumVisualStudioVersion = 10.0.40219.1");
	}
	else if (platform == Platform.Windows && Options.visualStudioVersion == VisualStudioVersion.VS2013) {
		this.p("Microsoft Visual Studio Solution File, Format Version 12.00");
		this.p("# Visual Studio 2013");
		this.p("VisualStudioVersion = 12.0.21005.1");
		this.p("MinimumVisualStudioVersion = 10.0.40219.1");
	}
	else if (platform == Platform.Windows && Options.visualStudioVersion == VisualStudioVersion.VS2012) {
		this.p("Microsoft Visual Studio Solution File, Format Version 12.00");
		this.p("# Visual Studio 2012");
	}
	else {
		this.p("Microsoft Visual Studio Solution File, Format Version 11.00");
		this.p("# Visual Studio 2010");
	}
	var solutionUuid = uuid.v4();
	for (project in solution.getProjects()) this.writeProjectDeclarations(solution.getProjects()[project], solutionUuid);
	this.p("Global");
	this.p("GlobalSection(SolutionConfigurationPlatforms) = preSolution", 1);
	for (c in this.getConfigs(platform)) {
		var config = this.getConfigs(platform)[c];
		for (s in this.getSystems(platform)) {
			var system = this.getSystems(platform)[s];
			this.p(config + "|" + system + " = " + config + "|" + system, 2);
		}
	}
	this.p("EndGlobalSection", 1);
	this.p("GlobalSection(ProjectConfigurationPlatforms) = postSolution", 1);
	for (project in solution.getProjects()) this.writeProjectBuilds(solution.getProjects()[project], platform);
	this.p("EndGlobalSection", 1);
	this.p("GlobalSection(SolutionProperties) = preSolution", 1);
	this.p("HideSolutionNode = FALSE", 2);
	this.p("EndGlobalSection", 1);
	this.p("EndGlobal");
	this.closeFile();

	for (p in solution.getProjects()) {
		var project = solution.getProjects()[p];
		this.exportProject(from, to, project, platform, solution.isCmd());
		this.exportFilters(from, to, project, platform);
		this.exportUserFile(from, to, project, platform);
		if (platform == Platform.WindowsApp) {
			this.exportManifest(to, project);
			var white = 0xffffffff;
			Icon.exportPng(to.resolve('Logo.scale-100.png'), 150, 150, white, from);
			Icon.exportPng(to.resolve('SmallLogo.scale-100.png'), 30, 30, white, from);
			Icon.exportPng(to.resolve('StoreLogo.scale-100.png'), 50, 50, white, from);
			Icon.exportPng(to.resolve('SplashScreen.scale-100.png'), 620, 300, white, from);
			Icon.exportPng(to.resolve('WideLogo.scale-100.png'), 310, 150, white, from);
		}
		else if (platform == Platform.Windows) {
			this.exportResourceScript(to);
			Icon.exportIco(to.resolve('icon.ico'), from);
		}
	}
}

ExporterVisualStudio.prototype.exportManifest = function (to, project) {
	this.writeFile(to.resolve("Package.appxmanifest"));

	this.p('<?xml version="1.0" encoding="utf-8"?>');
	this.p('<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10" xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest" xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10" IgnorableNamespaces="uap mp">');
		this.p('<Identity Name="b2714d6a-f52b-4943-b735-9b5777019bc9" Publisher="CN=Robert" Version="1.0.0.0" />', 1);
		this.p('<mp:PhoneIdentity PhoneProductId="b2714d6a-f52b-4943-b735-9b5777019bc9" PhonePublisherId="00000000-0000-0000-0000-000000000000"/>', 1);
		this.p('<Properties>', 1);
			this.p('<DisplayName>' + project.getName() + '</DisplayName>', 2);
			this.p('<PublisherDisplayName>Robert</PublisherDisplayName>', 2);
			this.p('<Logo>StoreLogo.png</Logo>', 2);
		this.p('</Properties>', 1);
		this.p('<Dependencies>', 1);
			this.p('<TargetDeviceFamily Name="Windows.Universal" MinVersion="10.0.10069.0" MaxVersionTested="10.0.10069.0" />', 2);
		this.p('</Dependencies>', 1);
		this.p('<Resources>', 1);
			this.p('<Resource Language="x-generate"/>', 2);
		this.p('</Resources>', 1);
		this.p('<Applications>', 1);
			this.p('<Application Id="App" Executable="$targetnametoken$.exe" EntryPoint="' + project.getName() + '.App">', 2);
				this.p('<uap:VisualElements DisplayName="' + project.getName() + '" Square150x150Logo="Assets\Logo.png" Square44x44Logo="Assets\SmallLogo.png" Description="App2" BackgroundColor="#464646">', 3);
					this.p('<uap:SplashScreen Image="SplashScreen.png" />', 4);
				this.p('</uap:VisualElements>', 3);
			this.p('</Application>', 2);
		this.p('</Applications>', 1);
		this.p('<Capabilities>', 1);
			this.p('<Capability Name="internetClient" />', 2);
		this.p('</Capabilities>', 1);
	this.p('</Package>');

	this.closeFile();
};

ExporterVisualStudio.prototype.exportResourceScript = function (to) {
	this.writeFile(to.resolve("resources.rc"));
	this.p("107       ICON         \"icon.ico\"");
	this.closeFile();
};

ExporterVisualStudio.prototype.exportAssetPathFilter = function (assetPath, dirs, assets) {
	var dir = assetPath.toString();
	if (!contains(dirs, dir)) dirs.push(dir);
	var paths = Files.newDirectoryStream(assetPath);
	for (p in paths) {
		var path = paths[p];
		if (Files.isDirectory(path)) this.exportAssetPathFilter(path, dirs, assets);
		else assets.push(path.toString());
	}
};

function contains(array, element) {
	for (index in array) {
		if (array[index] === element) return true;
	}
	return false;
}

ExporterVisualStudio.prototype.exportFilters = function (from, to, project, platform) {
	for (proj in project.getSubProjects()) this.exportFilters(from, to, project.getSubProjects()[proj], platform);

	this.writeFile(to.resolve(project.getName() + ".vcxproj.filters"));

	this.p("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	this.p("<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");

	//sort(project.getFiles());
	var lastdir = "";
	var dirs = [];
	for (f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (file.contains('/')) {
			var dir = file.substr(0, file.lastIndexOf('/'));
			if (dir != lastdir) {
				var subdir = dir;
				while (subdir.contains("/")) {
					subdir = subdir.substr(0, subdir.lastIndexOf('/'));
					if (!contains(dirs, subdir)) dirs.push(subdir);
				}
				dirs.push(dir);
				lastdir = dir;
			}
		}
	}
	var assets = [];
	if (platform == Platform.WindowsApp) this.exportAssetPathFilter(from.resolve(project.getDebugDir()), dirs, assets);

	this.p("<ItemGroup>", 1);
	for (dir in dirs) {
		this.p("<Filter Include=\"" + dirs[dir].replaceAll('/', '\\') + "\">", 2);
		this.p("<UniqueIdentifier>{" + uuid.v4().toString().toUpperCase() + "}</UniqueIdentifier>", 3);
		this.p("</Filter>", 2);
	}
	if (platform == Platform.WindowsApp) {
		this.p("<Filter Include=\"Package\">", 2);
		this.p("<UniqueIdentifier>{" + uuid.v4().toString().toUpperCase() + "}</UniqueIdentifier>", 3);
		this.p("</Filter>", 2);
	}
	this.p("</ItemGroup>", 1);

	if (platform == Platform.WindowsApp) {
		this.p("<ItemGroup>", 1);
		this.p("<AppxManifest Include=\"Package.appxmanifest\">", 2);
		this.p("<Filter>Package</Filter>", 3);
		this.p("</AppxManifest>", 2);
		this.p("</ItemGroup>", 1);

		var images = ['Logo.scale-100.png', 'SmallLogo.scale-100.png', 'StoreLogo.scale-100.png', 'SplashScreen.scale-100.png', 'WideLogo.scale-100.png'];
		for (var img in images) {
			var image = images[img];
			this.p('<ItemGroup>', 1)
				this.p('<Image Include="' + image + '">', 2);
					this.p('<Filter>Package</Filter>', 3);
				this.p('</Image>', 2);
			this.p('</ItemGroup>', 1);
		}
	}

	lastdir = "";
	this.p("<ItemGroup>", 1);
	for (f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (contains(file, '/')) {
			var dir = file.substr(0, file.lastIndexOf('/'));
			if (dir != lastdir) lastdir = dir;
			if (file.endsWith(".h")) {
				this.p("<ClInclude Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<Filter>" + dir.replaceAll('/', '\\') + "</Filter>", 3);
				this.p("</ClInclude>", 2);
			}
		}
	}
	this.p("</ItemGroup>", 1);

	lastdir = "";
	this.p("<ItemGroup>", 1);
	for (f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (file.contains('/')) {
			var dir = file.substr(0, file.lastIndexOf('/'));
			if (dir != lastdir) lastdir = dir;
			if (file.endsWith(".cpp") || file.endsWith(".c") || file.endsWith("cc")) {
				this.p("<ClCompile Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<Filter>" + dir.replaceAll('/', '\\') + "</Filter>", 3);
				this.p("</ClCompile>", 2);
			}
		}
	}
	this.p("</ItemGroup>", 1);

	lastdir = "";
	this.p("<ItemGroup>", 1);
	for (f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (contains(file, "/")) {
			var dir = file.substr(0, file.lastIndexOf('/'));
			if (dir != lastdir) lastdir = dir;
			if (file.endsWith(".cg") || file.endsWith(".hlsl")) {
				this.p("<CustomBuild Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<Filter>" + dir.replaceAll('/', '\\') + "</Filter>", 3);
				this.p("</CustomBuild>", 2);
			}
		}
	}
	this.p("</ItemGroup>", 1);

	lastdir = "";
	this.p("<ItemGroup>", 1);
	for (f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (file.contains("/")) {
			var dir = file.substr(0, file.lastIndexOf('/'));
			if (dir != lastdir) lastdir = dir;
			if (file.endsWith(".asm")) {
				this.p("<CustomBuild Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<Filter>" + dir.replaceAll('/', '\\') + "</Filter>", 3);
				this.p("</CustomBuild>", 2);
			}
		}
	}
	this.p("</ItemGroup>", 1);

	if (platform == Platform.WindowsApp) {
		lastdir = "";
		this.p("<ItemGroup>", 1);
		for (f in assets) {
			var file = assets[f];
			if (file.contains("/")) {
				var dir = file.substr(0, file.lastIndexOf('/'));
				if (dir != lastdir) lastdir = dir;
				this.p("<None Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<Filter>" + dir.replaceAll('/', '\\') + "</Filter>", 3);
				this.p("</None>", 2);
			}
		}
		this.p("</ItemGroup>", 1);
	}

	if (platform == Platform.Windows) {
		this.p("<ItemGroup>", 1);
			this.p("<None Include=\"icon.ico\">", 2);
				this.p("<Filter>Ressourcendateien</Filter>", 3);
			this.p("</None>", 2);
		this.p("</ItemGroup>", 1);
		this.p("<ItemGroup>", 1);
			this.p("<ResourceCompile Include=\"resources.rc\">", 2);
				this.p("<Filter>Ressourcendateien</Filter>", 3);
			this.p("</ResourceCompile>", 2);
		this.p("</ItemGroup>", 1);
	}

	this.p("</Project>");
	this.closeFile();
};

ExporterVisualStudio.prototype.addPropertyGroup = function (buildType, platform) {
	addPropertyGroup(buildType, false, platform);
};

ExporterVisualStudio.prototype.addPropertyGroup = function (buildType, wholeProgramOptimization, platform) {
	this.p("<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" + buildType + "|" + GetSys(platform) + "'\" Label=\"Configuration\">", 1);
	this.p("<ConfigurationType>Application</ConfigurationType>", 2);
	this.p("<WholeProgramOptimization>" + (wholeProgramOptimization ? "true" : "false") + "</WholeProgramOptimization>", 2);
	this.p("<CharacterSet>MultiByte</CharacterSet>", 2);
	this.p("</PropertyGroup>", 1);
};

ExporterVisualStudio.prototype.addWin8PropertyGroup = function (debug, platform) {
	this.p("<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" + (debug ? "Debug" : "Release") + "|" + platform + "'\" Label=\"Configuration\">", 1);
	this.p("<ConfigurationType>Application</ConfigurationType>", 2);
	this.p("<UseDebugLibraries>" + (debug ? "true" : "false") + "</UseDebugLibraries>", 2);
	if (!debug) this.p("<WholeProgramOptimization>true</WholeProgramOptimization>", 2);
	this.p("<PlatformToolset>v140</PlatformToolset>", 2);
	this.p("</PropertyGroup>", 1);
};

function valueOf(string) {
	if (string == "Debug") return Debug;
	if (string == "CodeAnalysis") return CodeAnalysis;
	if (string == "Profile") return Profile;
	if (string == "Profile_FastCap") return Profile_FastCap;
	if (string == "Release") return Release;
	if (string == "Release_LTCG") return Release_LTCG;
	throw "Unknown configuration";
}

ExporterVisualStudio.prototype.addItemDefinitionGroup = function (incstring, defines, buildType, warningLevel,
		prefast, optimization, functionLevelLinking, stringPooling, favorSize, release,
		profile, nocomdatfolding, ignoreXapilib, optimizeReferences, checksum, fastCap, comdatfolding, ltcg, platform) {
	this.p("<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" + buildType + "|" + this.GetSys(platform) + "'\">", 1);

	var compile = new ClCompile(this.out, 2, Platform.Xbox360, valueOf(buildType), incstring.split(';'), defines.split(';'));
	compile.print();

	this.p("<Link>", 2);
	this.p("<GenerateDebugInformation>true</GenerateDebugInformation>", 3);
	if (nocomdatfolding) this.p("<EnableCOMDATFolding>false</EnableCOMDATFolding>", 3);
	if (comdatfolding) this.p("<EnableCOMDATFolding>true</EnableCOMDATFolding>", 3);
	if (ignoreXapilib) this.p("<IgnoreSpecificDefaultLibraries>xapilib.lib</IgnoreSpecificDefaultLibraries>", 3);
	if (optimizeReferences) this.p("<OptimizeReferences>true</OptimizeReferences>", 3);
	this.p("<ProgramDatabaseFile>$(OutDir)$(ProjectName).pdb</ProgramDatabaseFile>", 3);
	if (checksum) this.p("<SetChecksum>true</SetChecksum>", 3);
	if (profile) this.p("<AdditionalDependencies>xapilibi.lib;d3d9i.lib;d3dx9.lib;xgraphics.lib;xboxkrnl.lib;xnet.lib;xaudio2.lib;xact3i.lib;x3daudioi.lib;xmcorei.lib;xbdm.lib;vcomp.lib</AdditionalDependencies>", 3);
	else if (ltcg) this.p("<AdditionalDependencies>xapilib.lib;d3d9ltcg.lib;d3dx9.lib;xgraphics.lib;xboxkrnl.lib;xnet.lib;xaudio2.lib;xact3ltcg.lib;x3daudioltcg.lib;xmcoreltcg.lib;vcomp.lib</AdditionalDependencies>", 3);
	else if (release) this.p("<AdditionalDependencies>xapilib.lib;d3d9.lib;d3dx9.lib;xgraphics.lib;xboxkrnl.lib;xnet.lib;xaudio2.lib;xact3.lib;x3daudio.lib;xmcore.lib;vcomp.lib</AdditionalDependencies>", 3);
	else this.p("<AdditionalDependencies>xapilibd.lib;d3d9d.lib;d3dx9d.lib;xgraphicsd.lib;xboxkrnl.lib;xnetd.lib;xaudiod2.lib;xactd3.lib;x3daudiod.lib;xmcored.lib;xbdm.lib;vcompd.lib</AdditionalDependencies>", 3);
	this.p("</Link>", 2);

	this.p("</ItemDefinitionGroup>", 1);
};

//private void addWinMD(String name) {
//	p("<Reference Include=\"" + name + ".winmd\">", 2);
//	p("<IsWinMDFile>true</IsWinMDFile>", 3);
//	p("</Reference>", 2);
//}

ExporterVisualStudio.prototype.exportProject = function (from, to, project, platform, cmd) {
	for (proj in project.getSubProjects()) exportProject(from, to, project.getSubProjects()[proj], platform, cmd);

	this.writeFile(to.resolve(project.getName() + ".vcxproj"));

	this.p("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	if (Options.visualStudioVersion == VisualStudioVersion.VS2015) this.p("<Project DefaultTargets=\"Build\" ToolsVersion=\"14.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");
	else if (Options.visualStudioVersion == VisualStudioVersion.VS2013) this.p("<Project DefaultTargets=\"Build\" ToolsVersion=\"12.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");
	else this.p("<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");
	this.p("<ItemGroup Label=\"ProjectConfigurations\">", 1);
	for (sys in this.getSystems(platform)) {
		var system = this.getSystems(platform)[sys];
		for (c in this.getConfigs(platform)) {
			var config = this.getConfigs(platform)[c];
			this.p("<ProjectConfiguration Include=\"" + config + "|" + system + "\">", 2);
			this.p("<Configuration>" + config + "</Configuration>", 3);
			this.p("<Platform>" + system + "</Platform>", 3);
			this.p("</ProjectConfiguration>", 2);
		}
	}
	this.p("</ItemGroup>", 1);
	this.p("<PropertyGroup Label=\"Globals\">", 1);
	this.p("<ProjectGuid>{" + project.getUuid().toString().toUpperCase() + "}</ProjectGuid>", 2);
	//p("<Keyword>Win32Proj</Keyword>", 2);
	//p("<RootNamespace>" + project.Name + "</RootNamespace>", 2);
	if (platform == Platform.WindowsApp) {
		this.p("<DefaultLanguage>en-US</DefaultLanguage>", 2);
		this.p("<MinimumVisualStudioVersion>14.0</MinimumVisualStudioVersion>", 2);
		this.p("<AppContainerApplication>true</AppContainerApplication>", 2);
		this.p('<ApplicationType>Windows Store</ApplicationType>', 2);
		this.p('<ApplicationTypeRevision>8.2</ApplicationTypeRevision>', 2);
	}
	this.p("</PropertyGroup>", 1);
	this.p("<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />", 1);
	if (platform == Platform.Xbox360) {
		this.addPropertyGroup("CodeAnalysis", platform);
		this.addPropertyGroup("Debug", platform);
		this.addPropertyGroup("Profile", platform);
		this.addPropertyGroup("Profile_FastCap", platform);
		this.addPropertyGroup("Release", platform);
		this.addPropertyGroup("Release_LTCG", true, platform);
	}
	else if (platform == Platform.WindowsApp) {
		this.addWin8PropertyGroup(true, "Win32");
		this.addWin8PropertyGroup(true, "ARM");
		this.addWin8PropertyGroup(true, "x64");
		this.addWin8PropertyGroup(false, "Win32");
		this.addWin8PropertyGroup(false, "ARM");
		this.addWin8PropertyGroup(false, "x64");
	}
	else {
		this.p("<PropertyGroup Condition=\"'$(Configuration)'=='Debug'\" Label=\"Configuration\">", 1);
		this.p("<ConfigurationType>Application</ConfigurationType>", 2);
		this.p("<UseDebugLibraries>true</UseDebugLibraries>", 2);
		if (platform == Platform.Windows && Options.visualStudioVersion == VisualStudioVersion.VS2015) {
			this.p("<PlatformToolset>v140</PlatformToolset>", 2);
		}
		else if (platform == Platform.Windows && Options.visualStudioVersion == VisualStudioVersion.VS2013) {
			this.p("<PlatformToolset>v120</PlatformToolset>", 2);
		}
		else if (platform == Platform.Windows && Options.visualStudioVersion == VisualStudioVersion.VS2012) {
			this.p("<PlatformToolset>v110</PlatformToolset>", 2);
		}
		if (platform == Platform.Windows) {
			this.p("<CharacterSet>Unicode</CharacterSet>", 2);
		}
		else if (platform == Platform.PlayStation3) {
			this.p("<PlatformToolset>SNC</PlatformToolset>", 2);
			this.p("<ExceptionsAndRtti>WithExceptsWithRtti</ExceptionsAndRtti>", 2);
		}
		this.p("</PropertyGroup>", 1);
		this.p("<PropertyGroup Condition=\"'$(Configuration)'=='Release'\" Label=\"Configuration\">", 1);
		this.p("<ConfigurationType>Application</ConfigurationType>", 2);
		this.p("<UseDebugLibraries>false</UseDebugLibraries>", 2);
		if (platform == Platform.Windows && Options.visualStudioVersion == VisualStudioVersion.VS2015) {
			this.p("<PlatformToolset>v140</PlatformToolset>", 2);
		}
		else if (platform == Platform.Windows && Options.visualStudioVersion == VisualStudioVersion.VS2013) {
			this.p("<PlatformToolset>v120</PlatformToolset>", 2);
		}
		else if (platform == Platform.Windows && Options.visualStudioVersion == VisualStudioVersion.VS2012) {
			this.p("<PlatformToolset>v110</PlatformToolset>", 2);
		}
		if (platform == Platform.Windows) {
			this.p("<WholeProgramOptimization>true</WholeProgramOptimization>", 2);
			this.p("<CharacterSet>Unicode</CharacterSet>", 2);
		}
		else if (platform == Platform.PlayStation3) {
			this.p("<PlatformToolset>SNC</PlatformToolset>", 2);
			this.p("<ExceptionsAndRtti>WithExceptsWithRtti</ExceptionsAndRtti>", 2);
		}
		this.p("</PropertyGroup>", 1);
	}
	this.p("<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />", 1);
	this.p("<ImportGroup Label=\"ExtensionSettings\">", 1);
	this.p("</ImportGroup>", 1);
	if (platform == Platform.WindowsApp) {
		var configurations = [];
		configurations.push("Debug");
		configurations.push("Release");
		for (configuration in configurations) {
			for (system in this.getSystems(platform)) {
				this.p("<ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='" + configurations[configuration] + "|" + this.getSystems(platform)[system] + "'\">", 1);
				this.p("<Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />", 2);
				this.p("</ImportGroup>", 1);
			}
		}
	}
	else {
		for (system in this.getSystems(platform)) {
			this.p("<ImportGroup Label=\"PropertySheets\" Condition=\"'$(Platform)'=='" + this.getSystems(platform)[system] + "'\">", 1);
			this.p("<Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />", 2);
			this.p("</ImportGroup>", 1);
		}
	}
	this.p("<PropertyGroup Label=\"UserMacros\" />", 1);

	if (platform == Platform.WindowsApp) {
	//<PropertyGroup>
	//<PackageCertificateKeyFile>Direct3DApplication1_TemporaryKey.pfx</PackageCertificateKeyFile>
	//</PropertyGroup>
	}

	if (platform == Platform.Windows || platform == Platform.Xbox360) {
		for (s in this.getSystems(platform)) {
			var system = this.getSystems(platform)[s];
			for (c in this.getConfigs(platform)) {
				var config = this.getConfigs(platform)[c];
				this.p("<PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" + config + "|" + system + "'\">", 1);
					if (system == "Win32") {
						if (Options.intermediateDrive == "") this.p("<IntDir>$(Configuration)\\" + project.getName() + "\\</IntDir>", 2);
						else this.p("<IntDir>" + Options.intermediateDrive + ":\\$(projectname)\\$(Configuration)\\" + project.getName() + "\\</IntDir>", 2);
					}
					else this.p("<IntDir>$(Platform)\\$(Configuration)\\" + project.getName() + "\\</IntDir>", 2);
					this.p("<LinkIncremental>" + ((config == "Debug" || config == "CodeAnalysis") ? "true" : "false") + "</LinkIncremental>", 2);
					this.p("</PropertyGroup>", 1);
			}
		}
	}
	else if (platform == Platform.PlayStation3) {
		this.p("<PropertyGroup />", 1);
	}

	var defines = "";
	for (define in project.getDefines()) defines += project.getDefines()[define] + ";";

	var incstring = "";
	for (inc in project.getIncludeDirs()) {
		var include = project.getIncludeDirs()[inc];
		if (Paths.get(include).isAbsolute()) {
			incstring += include + ';';
		}
		else {
			incstring += from.resolve(include).toAbsolutePath().toString() + ";";
		}
	}
	if (incstring.length > 0) incstring = incstring.substr(0, incstring.length - 1);

	var debuglibs = "";
	for (proj in project.getSubProjects()) debuglibs += "Debug\\" + project.getSubProjects()[proj].getName() + ".lib;";
	for (l in project.getLibs()) {
		var lib = project.getLibs()[l];
		if (Files.exists(from.resolve(lib + ".lib"))) debuglibs += from.resolve(lib).toAbsolutePath().toString() + ".lib;";
		else debuglibs += lib + ".lib;";
	}

	var releaselibs = "";
	for (proj in project.getSubProjects()) releaselibs += "Release\\" + project.getSubProjects()[proj].getName() + ".lib;";
	for (l in project.getLibs()) {
		var lib = project.getLibs()[l];
		if (Files.exists(from.resolve(lib + ".lib"))) releaselibs += from.resolve(lib).toAbsolutePath().toString() + ".lib;";
		else releaselibs += lib + ".lib;";
	}

	if (platform == Platform.Xbox360) {
		this.addItemDefinitionGroup(incstring, defines, "Debug", 3, false, false, false, false, false, false, false, false, false, false, false, false, false, false, platform);
		this.addItemDefinitionGroup(incstring, defines, "CodeAnalysis", 4, true, false, false, false, false, false, false, false, false, false, false, false, false, false, platform);
		this.addItemDefinitionGroup(incstring, defines, "Profile", 3, false, true, true, true, true, true, true, true, true, true, true, false, false, false, platform);
		this.addItemDefinitionGroup(incstring, defines, "Profile_FastCap", 3, false, true, true, true, true, true, true, true, false, true, true, true, false, false, platform);
		this.addItemDefinitionGroup(incstring, defines, "Release", 3, false, true, true, true, true, true, false, false, false, true, true, false, true, false, platform);
		this.addItemDefinitionGroup(incstring, defines, "Release_LTCG", 3, false, true, true, true, true, true, false, false, false, true, true, false, true, true, platform);
	}
	else if (platform == Platform.WindowsApp) {
		/*this.p("<ItemDefinitionGroup>", 1);
			this.p("<Link>", 2);
				this.p("<AdditionalDependencies>MMDevAPI.lib;MFuuid.lib;MFReadWrite.lib;MFplat.lib;d2d1.lib;d3d11.lib;dxgi.lib;ole32.lib;windowscodecs.lib;dwrite.lib;%(AdditionalDependencies)</AdditionalDependencies>", 3);
			this.p("</Link>", 2);
			var compile = new ClCompile(this.out, 2, Platform.WindowsApp, Configuration.Debug, incstring.split(';'), defines.split(';'));
			compile.print();
		this.p("</ItemDefinitionGroup>", 1);*/

		var configs = [
			{ config: 'Debug', system: 'ARM', libdir: '\\arm' }, { config: 'Release', system: 'ARM', libdir: '\\arm' },
			{ config: 'Debug', system: 'Win32', libdir: '' }, { config: 'Release', system: 'Win32', libdir: '' },
			{ config: 'Debug', system: 'x64', libdir: '\\amd64' }, { config: 'Release', system: 'x64', libdir: '\\amd64' }
		];

		for (var c in configs) {
			var config = configs[c];
			
			var libdir = '';
			if (config.system == 'ARM') libdir = '\\arm';
			else if (config.system == 'x64') libdir = '\\amd64';

			var moredefines = config.config === 'Debug' ? '_DEBUG;' : 'NDEBUG;';
			if (config.system === 'x64') moredefines += 'SYS_64;'

			this.p("<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" + config.config + "|" + config.system + "'\">", 1);
				this.p('<Link>', 2);
					this.p('<AdditionalDependencies>d3d11.lib; dxgi.lib; windowscodecs.lib; %(AdditionalDependencies)</AdditionalDependencies>', 3);
					this.p('<AdditionalLibraryDirectories>%(AdditionalLibraryDirectories); $(VCInstallDir)\\lib\\store\\' + libdir + '; $(VCInstallDir)\\lib\\' + libdir + '</AdditionalLibraryDirectories>', 3);
				this.p('</Link>', 2);
				this.p('<ClCompile>', 2);
					this.p('<PrecompiledHeader>NotUsing</PrecompiledHeader>', 3);
					this.p('<AdditionalIncludeDirectories>' + incstring + ';%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>', 3);
					this.p('<AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>', 3);
					this.p('<DisableSpecificWarnings>4453;28204</DisableSpecificWarnings>', 3);
					this.p('<PreprocessorDefinitions>' + defines + moredefines + '%(PreprocessorDefinitions)</PreprocessorDefinitions>', 3);
				this.p('</ClCompile>', 2);
			this.p('</ItemDefinitionGroup>', 1);
		}
	}
	else {
		for (var s in this.getSystems(platform)) {
			var system = this.getSystems(platform)[s];
			this.p("<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|" + system + "'\">", 1);
			this.p("<ClCompile>", 2);

			if (Options.precompiledHeaders) p("<PrecompiledHeader>Use</PrecompiledHeader>", 3);
			this.p("<AdditionalIncludeDirectories>" + incstring + "</AdditionalIncludeDirectories>", 3);

			if (platform == Platform.Windows) {
				this.p("<WarningLevel>Level3</WarningLevel>", 3);
				this.p("<Optimization>Disabled</Optimization>", 3);
				if (system == "x64") this.p("<PreprocessorDefinitions>" + defines + "SYS_64;WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>", 3);
				else this.p("<PreprocessorDefinitions>" + defines + "WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>", 3);
				this.p("<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>", 3);
				this.p("<MultiProcessorCompilation>true</MultiProcessorCompilation>", 3);
				this.p("<MinimalRebuild>false</MinimalRebuild>", 3);
				if (Options.visualStudioVersion == VisualStudioVersion.VS2013) this.p("<SDLCheck>true</SDLCheck>", 3);
			}
			else if (platform == Platform.PlayStation3) {
				this.p("<UserPreprocessorDefinitions>" + defines + "_DEBUG;__CELL_ASSERT__;%(UserPreprocessorDefinitions);</UserPreprocessorDefinitions>", 3);
				this.p("<GenerateDebugInformation>true</GenerateDebugInformation>", 3);
				this.p("<PreprocessorDefinitions>%(UserPreprocessorDefinitions);$(BuiltInDefines);__INTELLISENSE__;%(PreprocessorDefinitions);</PreprocessorDefinitions>", 3);
			}
			this.p("</ClCompile>", 2);
			if (platform == Platform.Windows) {
				this.p("<Link>", 2);
				if (cmd) this.p("<SubSystem>Console</SubSystem>", 3);
				else this.p("<SubSystem>Windows</SubSystem>", 3);
				this.p("<GenerateDebugInformation>true</GenerateDebugInformation>", 3);
				var libs = debuglibs;
				for (l in project.getLibsFor("debug_" + system)) {
					var lib = project.getLibsFor('debug_' + system)[l];
					if (Files.exists(from.resolve(lib + ".lib"))) libs += from.resolve(lib).toAbsolutePath().toString() + ".lib;";
					else libs += lib + ".lib;";
				}
				for (l in project.getLibsFor(system)) {
					var lib = project.getLibsFor(system)[l];
					if (Files.exists(from.resolve(lib + ".lib"))) libs += from.resolve(lib).toAbsolutePath().toString() + ".lib;";
					else libs += lib + ".lib;";
				}
				for (l in project.getLibsFor("debug")) {
					var lib = project.getLibsFor('debug')[l];
					if (Files.exists(from.resolve(lib + ".lib"))) libs += from.resolve(lib).toAbsolutePath().toString() + ".lib;";
					else libs += lib + ".lib;";
				}
				this.p("<AdditionalDependencies>" + libs + "kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)</AdditionalDependencies>", 3);
				this.p("</Link>", 2);
			}
			else if (platform == Platform.PlayStation3) {
				this.p("<Link>", 2);
				this.p("<AdditionalDependencies>libgcm_cmd.a;libgcm_sys_stub.a;libsysmodule_stub.a;libsysutil_stub.a;%(AdditionalDependencies)</AdditionalDependencies>", 3);
				this.p("</Link>", 2);
			}
			this.p("</ItemDefinitionGroup>", 1);
			this.p("<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|" + system + "'\">", 1);
			this.p("<ClCompile>", 2);
			if (Options.precompiledHeaders) this.p("<PrecompiledHeader>Use</PrecompiledHeader>", 3);
			this.p("<AdditionalIncludeDirectories>" + incstring + "</AdditionalIncludeDirectories>", 3);
			if (platform == Platform.Windows) {
				this.p("<WarningLevel>Level3</WarningLevel>", 3);
				this.p("<Optimization>MaxSpeed</Optimization>", 3);
				this.p("<FunctionLevelLinking>true</FunctionLevelLinking>", 3);
				this.p("<IntrinsicFunctions>true</IntrinsicFunctions>", 3);
				if (system == "x64") this.p("<PreprocessorDefinitions>" + defines + "SYS_64;WIN32;NDEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>", 3);
				else this.p("<PreprocessorDefinitions>" + defines + "WIN32;NDEBUG;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>", 3);
				this.p("<RuntimeLibrary>MultiThreaded</RuntimeLibrary>", 3);
				this.p("<MultiProcessorCompilation>true</MultiProcessorCompilation>", 3);
				this.p("<MinimalRebuild>false</MinimalRebuild>", 3);
				if (Options.visualStudioVersion == VisualStudioVersion.VS2013) this.p("<SDLCheck>true</SDLCheck>", 3);
			}
			else if (platform == Platform.PlayStation3) {
				this.p("<UserPreprocessorDefinitions>" + defines + "NDEBUG;%(UserPreprocessorDefinitions);</UserPreprocessorDefinitions>", 3);
				this.p("<OptimizationLevel>Level2</OptimizationLevel>", 3);
				this.p("<PreprocessorDefinitions>%(UserPreprocessorDefinitions);$(BuiltInDefines);__INTELLISENSE__;%(PreprocessorDefinitions);</PreprocessorDefinitions>");
			}

			this.p("</ClCompile>", 2);
			if (platform == Platform.Windows) {
				this.p("<Link>", 2);
				if (cmd) this.p("<SubSystem>Console</SubSystem>", 3);
				else this.p("<SubSystem>Windows</SubSystem>", 3);
				this.p("<GenerateDebugInformation>true</GenerateDebugInformation>", 3);
				this.p("<EnableCOMDATFolding>true</EnableCOMDATFolding>", 3);
				this.p("<OptimizeReferences>true</OptimizeReferences>", 3);
				var libs = releaselibs;
				for (l in project.getLibsFor("release_" + system)) {
					var lib = project.getLibsFor('release_' + system)[l];
					if (Files.exists(from.resolve(lib + ".lib"))) libs += from.resolve(lib).toAbsolutePath().toString() + ".lib;";
					else libs += lib + ".lib;";
				}
				for (l in project.getLibsFor(system)) {
					var lib = project.getLibsFor(system)[l];
					if (Files.exists(from.resolve(lib + ".lib"))) libs += from.resolve(lib).toAbsolutePath().toString() + ".lib;";
					else libs += lib + ".lib;";
				}
				for (l in project.getLibsFor("release")) {
					var lib = project.getLibsFor('release')[l];
					if (Files.exists(from.resolve(lib + ".lib"))) libs += from.resolve(lib).toAbsolutePath().toString() + ".lib;";
					else libs += lib + ".lib;";
				}
				this.p("<AdditionalDependencies>" + libs + "kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)</AdditionalDependencies>", 3);
				this.p("</Link>", 2);
			}
			else if (platform == Platform.PlayStation3) {
				this.p("<Link>", 2);
				this.p("<AdditionalDependencies>libgcm_cmd.a;libgcm_sys_stub.a;libsysmodule_stub.a;libsysutil_stub.a;%(AdditionalDependencies)</AdditionalDependencies>", 3);
				this.p("</Link>", 2);
			}
			this.p("</ItemDefinitionGroup>", 1);
		}
	}

	this.p("<ItemGroup>", 1);
	for (f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (file.endsWith(".h")) this.p("<ClInclude Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\" />", 2);
	}
	this.p("</ItemGroup>", 1);

	if (platform == Platform.WindowsApp) {
		this.p("<ItemDefinitionGroup Condition=\"'$(Configuration)'=='Release'\">", 1);
			this.p("<ClCompile>", 2);
				this.p("<PreprocessorDefinitions>" + defines + "NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>", 3);
			this.p("</ClCompile>", 2);
		this.p("</ItemDefinitionGroup>", 1);

		this.p("<ItemDefinitionGroup Condition=\"'$(Configuration)'=='Debug'\">", 1);
			this.p("<ClCompile>", 2);
				this.p("<PreprocessorDefinitions>" + defines + "_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>", 3);
			this.p("</ClCompile>", 2);
		this.p("</ItemDefinitionGroup>", 1);

		this.p("<ItemGroup>", 1);

		var images = ['Logo.scale-100.png', 'SmallLogo.scale-100.png', 'StoreLogo.scale-100.png', 'SplashScreen.scale-100.png', 'WideLogo.scale-100.png'];
		for (var img in images) {
			var image = images[img];
			this.p('<Image Include="' + image + '" />', 2);
		}

		this.p("</ItemGroup>", 1);
		this.p("<ItemGroup>", 1);
		this.p("<AppxManifest Include=\"Package.appxmanifest\" />", 2);
		this.p("</ItemGroup>", 1);

		this.p("<ItemGroup>", 1);
		this.exportAssetPath(from.resolve(project.getDebugDir()));
		this.p("</ItemGroup>", 1);
	}

	this.p("<ItemGroup>", 1);
	var objects = {};
	var stdafx = '';
	for (var f in project.getFiles()) {
		var file = project.getFiles()[f];
		if (file.endsWith(".cpp") || file.endsWith(".c") || file.endsWith("cc")) {
			if (Options.precompiledHeaders && (file.endsWith("stdafx.cpp") || file.endsWith("pch.cpp"))) {
				stdafx = file;
				continue;
			}
			var name = file.toLowerCase();
			if (name.contains('/')) name = name.substr(name.lastIndexOf('/') + 1);
			name = name.substr(0, name.lastIndexOf('.'));
			if (!objects[name]) {
				if (platform === Platform.WindowsApp && file.endsWith(".c")) {
					this.p("<ClCompile Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
						this.p('<CompileAsWinRT>false</CompileAsWinRT>', 3);
					this.p('</ClCompile>', 2);
				}
				else {
					this.p("<ClCompile Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\" />", 2);
				}
				objects[name] = true;
			}
			else {
				while (objects[name]) {
					name = name + "_";
				}
				this.p("<ClCompile Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<ObjectFileName>$(IntDir)\\" + name + ".obj</ObjectFileName>", 3);
				if (platform === Platform.WindowsApp && file.endsWith(".c")) {
					this.p('<CompileAsWinRT>false</CompileAsWinRT>', 3);
				}
				this.p("</ClCompile>", 2);
				objects[name] = true;
			}
		}
	}
	if (Options.precompiledHeaders) {
		if (stdafx.length == 0) throw "stdafx.cpp not found.";
		this.p("<ClCompile Include=\"../" + stdafx + "\">", 2);
		this.p("<PrecompiledHeader Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">Create</PrecompiledHeader>", 3);
		this.p("<PrecompiledHeader Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">Create</PrecompiledHeader>", 3);
		this.p("</ClCompile>", 2);
	}
	this.p("</ItemGroup>", 1);

	if (platform == Platform.PlayStation3) {
		p("<ItemGroup>", 1);
		for (f in project.getFiles()) {
			var file = project.getFiles()[f];
			if (endsWith(file, ".vp.cg")) {
				this.p("<CustomBuild Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<FileType>Document</FileType>", 2);
				this.p("<Command>$(SCE_PS3_ROOT)\\host-win32\\Cg\\bin\\sce-cgc -quiet -profile sce_vp_rsx -o \"%(Filename).vpo\" \"%(FullPath)\"\n$(SCE_PS3_ROOT)\\host-win32\\ppu\\bin\\ppu-lv2-objcopy  -I binary -O elf64-powerpc-celloslv2 -B powerpc \"%(Filename).vpo\" \"%(Filename).ppu.o\"</Command>", 2);
				this.p("<Outputs>%(Filename).vpo;%(Filename).ppu.o;%(Outputs)</Outputs>", 2);
				this.p("</CustomBuild>", 2);
			}
			else if (endsWith(file, ".fp.cg")) {
				this.p("<CustomBuild Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<FileType>Document</FileType>", 2);
				this.p("<Command>$(SCE_PS3_ROOT)\\host-win32\\Cg\\bin\\sce-cgc -quiet -profile sce_fp_rsx -o \"%(Filename).fpo\" \"%(FullPath)\"\n$(SCE_PS3_ROOT)\\host-win32\\ppu\\bin\\ppu-lv2-objcopy  -I binary -O elf64-powerpc-celloslv2 -B powerpc \"%(Filename).fpo\" \"%(Filename).ppu.o\"</Command>", 2);
				this.p("<Outputs>%(Filename).fpo;%(Filename).ppu.o;%(Outputs)</Outputs>", 2);
				this.p("</CustomBuild>", 2);
			}
		}
		p("</ItemGroup>", 1);
	}

	if (platform == Platform.Windows) {
		this.p("<ItemGroup>", 1);
		for (f in project.getFiles()) {
			var file = project.getFiles()[f];
			if (file.endsWith(".cg")) {
				this.p("<CustomBuild Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<FileType>Document</FileType>", 2);
				this.p("<Command>..\\" + "Kt\\Tools\\ShaderCompiler.exe " + ((Options.graphicsApi == GraphicsApi.OpenGL || Options.graphicsApi == GraphicsApi.OpenGL2) ? "glsl" : "d3d9") + " \"%(FullPath)\" " + from.resolve(project.getDebugDir()).toAbsolutePath().toString().replaceAll('/', '\\') + "\\Shaders\\%(Filename)</Command>", 2);
				this.p("<Outputs>" + from.resolve(project.getDebugDir()).toAbsolutePath().toString().replaceAll('/', '\\') + "\\Shaders\\%(Filename)" + ((Options.graphicsApi === GraphicsApi.OpenGL || Options.graphicsApi === GraphicsApi.OpenGL2) ? ".glsl" : ".d3d9") + ";%(Outputs)</Outputs>", 2);
				this.p("</CustomBuild>", 2);
			}
		}
		this.p("</ItemGroup>", 1);
		this.p("<ItemGroup>", 1);
		for (f in project.getFiles()) {
			var file = project.getFiles()[f];
			if (Project.koreDir.toString() != "" && file.endsWith(".glsl")) {
				this.p("<CustomBuild Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<FileType>Document</FileType>", 2);
				this.p("<Command>\"" + from.resolve(Project.koreDir).toAbsolutePath().toString().replaceAll('/', '\\') + "\\Tools\\kfx\\kfx.exe\" " + ((Options.graphicsApi === GraphicsApi.OpenGL || Options.graphicsApi === GraphicsApi.OpenGL2) ? "glsl" : (Options.graphicsApi === GraphicsApi.Direct3D11 ? "d3d11" : "d3d9")) + " \"%(FullPath)\" ..\\" + project.getDebugDir().replaceAll('/', '\\') + "\\%(Filename) ..\\build</Command>", 2);
				this.p("<Outputs>" + from.resolve(project.getDebugDir()).toAbsolutePath().toString().replaceAll('/', '\\') + "\\%(Filename);%(Outputs)</Outputs>", 2);
				this.p("<Message>Compiling %(FullPath)</Message>", 2);
				this.p("</CustomBuild>", 2);
			}
		}
		this.p("</ItemGroup>", 1);
		this.p("<ItemGroup>", 1);
		for (f in project.getFiles()) {
			var file = project.getFiles()[f];
			if (Project.koreDir.toString() != "" && file.endsWith(".asm")) {
				this.p("<CustomBuild Include=\"" + from.resolve(file).toAbsolutePath().toString() + "\">", 2);
				this.p("<FileType>Document</FileType>", 2);
				this.p("<Command>" + from.resolve(Project.koreDir).toAbsolutePath().toString().replaceAll('/', '\\') + "\\Tools\\yasm-1.2.0-win32.exe -Xvc -f Win32 -g cv8 -o $(OutDir)\\%(Filename).obj -I ..\\Kt\\WebM\\src -I ..\\Kt\\WebM\\build -rnasm -pnasm \"%(FullPath)\"</Command>", 2);
				this.p("<Outputs>$(OutDir)\\%(Filename).obj</Outputs>", 2);
				this.p("<Message>Compiling %(FullPath)</Message>", 2);
				this.p("</CustomBuild>", 2);
			}
		}
		this.p("</ItemGroup>", 1);
		this.p("<ItemGroup>", 1);
			this.p("<None Include=\"icon.ico\" />", 2);
		this.p("</ItemGroup>", 1);
		this.p("<ItemGroup>", 1);
			this.p("<ResourceCompile Include=\"resources.rc\" />", 2);
		this.p("</ItemGroup>", 1);
	}

	if (platform == Platform.PlayStation3) {
		this.p("<Import Condition=\"'$(ConfigurationType)' == 'Makefile' and Exists('$(VCTargetsPath)\\Platforms\\$(Platform)\\SCE.Makefile.$(Platform).targets')\" Project=\"$(VCTargetsPath)\\Platforms\\$(Platform)\\SCE.Makefile.$(Platform).targets\" />", 1);
	}
	this.p("<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />", 1);
	this.p("<ImportGroup Label=\"ExtensionTargets\">", 2);
	this.p("</ImportGroup>", 1);
	this.p("</Project>");
	this.closeFile();
};

ExporterVisualStudio.prototype.exportAssetPath = function (assetPath) {
	var paths = Files.newDirectoryStream(assetPath);
	for (var p in paths) {
		var path = paths[p];
		if (Files.isDirectory(path)) {
			this.exportAssetPath(path);
		}
		else {
			this.p("<None Include=\"../windowsapp/" + path.toString() + "\">", 2);
				this.p("<DeploymentContent>true</DeploymentContent>", 3);
			this.p("</None>", 2);
		}
	}
};

module.exports = ExporterVisualStudio;
