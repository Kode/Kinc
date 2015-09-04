"use strict";

const Block = require('./Block.js');
const Platform = require('./Platform.js');
const Configuration = require('./Configuration.js');

function toLine(options) {
	let line = '';
	for (let option of options) {
		line += option + ";";
	}
	return line;
}

class ClCompile extends Block {
	constructor(out, indentation, platform, configuration, includes, defines) {
		super(out, indentation);

		this.platform = platform;
		this.includes = includes;
		this.defines = defines;

		this.minimalRebuild = false;

		//PS3
		this.userPreprocessorDefinitions = '';

		//Xbox360
		this.functionLevelLinking = false;
		this.stringPooling = false;
		this.prefast = false;
		this.favorSize = false;
		this.fastCap = false;

		this.warningLevel = 3;
		this.optimization = false;
		this.runtimeLibrary = "MultiThreadedDebug";
		this.multiProcessorCompilation = true;
		this.objectFileName = "$(IntDir)\\build\\%(RelativeDir)";
		this.generateDebugInformation = false;
		//this.configuration = configuration;
		switch (platform) {
			case Platform.WindowsApp:
				this.includes.push("$(ProjectDir)");
				this.includes.push("$(IntermediateOutputPath)");
				this.includes.push("%(AdditionalIncludeDirectories)");
				this.defines.push("_UNICODE");
				this.defines.push("UNICODE");
				this.defines.push("%(PreprocessorDefinitions)");
				break;
			case Platform.Xbox360:
				this.defines.push("_XBOX");
				switch (configuration) {
					case Configuration.Debug:
						this.defines.push("_DEBUG");
						this.warningLevel = 3;
						this.prefast = false;
						this.optimization = false;
						this.functionLevelLinking = false;
						this.stringPooling = false;
						this.favorSize = false;
						this.fastCap = false;
						break;
					case Configuration.CodeAnalysis:
						this.defines.push("_DEBUG");
						this.warningLevel = 4;
						this.prefast = true;
						this.optimization = false;
						this.functionLevelLinking = false;
						this.stringPooling = false;
						this.favorSize = false;
						this.fastCap = false;
						break;
					case Configuration.Profile:
						this.defines.push("NDEBUG");
						this.defines.push("PROFILE");
						this.warningLevel = 3;
						this.prefast = false;
						this.optimization = true;
						this.functionLevelLinking = true;
						this.stringPooling = true;
						this.favorSize = true;
						this.fastCap = false;
						break;
					case Configuration.Profile_FastCap:
						this.defines.push("NDEBUG");
						this.defines.push("FASTCAP");
						this.warningLevel = 3;
						this.prefast = false;
						this.optimization = true;
						this.functionLevelLinking = true;
						this.stringPooling = true;
						this.favorSize = true;
						this.fastCap = true;
						break;
					case Configuration.Release:
						this.defines.push("NDEBUG");
						this.warningLevel = 3;
						this.prefast = false;
						this.optimization = true;
						this.functionLevelLinking = true;
						this.stringPooling = true;
						this.favorSize = true;
						this.fastCap = false;
						break;
					case Configuration.Release_LTCG:
						this.defines.push("NDEBUG");
						this.defines.push("LTCG");
						this.warningLevel = 3;
						this.prefast = false;
						this.optimization = true;
						this.functionLevelLinking = true;
						this.stringPooling = true;
						this.favorSize = true;
						this.fastCap = false;
				}
				break;
			default:
				break;
		}
	}

	print() {
		let defineLine = toLine(this.defines);
		let includeLine = toLine(this.includes);

		this.tagStart("ClCompile");
		this.tag("AdditionalIncludeDirectories", includeLine);
		if (this.platform == Platform.Windows) {
			this.tag("WarningLevel", "Level" + this.warningLevel);
			this.tag("Optimization", this.optimization ? "Enabled" : "Disabled");
			this.tag("PreprocessorDefinitions", defineLine);
			this.tag("RuntimeLibrary", this.runtimeLibrary);
			this.tag("MultiProcessorCompilation", this.multiProcessorCompilation ? "true" : "false");
			this.tag("MinimalRebuild", this.minimalRebuild ? "true" : "false");
			this.tag("ObjectFileName", this.objectFileName);
			this.tag("SDLCheck", "true");
		}
		else if (this.platform == Platform.WindowsApp) {
			//tag("PreprocessorDefinitions", defineLine);
			this.tag("PrecompiledHeader", "NotUsing");
			this.tag("ObjectFileName", this.objectFileName);
			this.tag("DisableSpecificWarnings", "4453");
			this.tag("SDLCheck", "true");
		}
		else if (this.platform == Platform.PlayStation3) {
			this.tag("UserPreprocessorDefinitions", this.userPreprocessorDefinitions);
			this.tag("GenerateDebugInformation", this.generateDebugInformation ? "true" : "false");
			this.tag("PreprocessorDefinitions", defineLine);
			this.tag("ObjectFileName", this.objectFileName);
		}
		else if (this.platform == Platform.Xbox360) {
			this.tag("PrecompiledHeader", "Use");
			this.tag("WarningLevel", "Level" + this.warningLevel);
			this.tag("DebugInformationFormat", "ProgramDatabase");
			this.tag("Optimization", this.optimization ? "Full" : "Disabled");
			if (this.functionLevelLinking) this.tag("FunctionLevelLinking", "true");
			this.tag("ExceptionHandling", "false");
			if (this.stringPooling) this.tag("StringPooling", "true");
			this.tag("MinimalRebuild", "true");
			if (this.prefast) this.tag("PREfast", "AnalyzeOnly");
			if (this.favorSize) this.tag("FavorSizeOrSpeed", "Size");
			this.tag("BufferSecurityCheck", "false");
			this.tag("PrecompiledHeaderOutputFile", "$(OutDir)$(ProjectName).pch");
			this.tag("RuntimeLibrary", this.runtimeLibrary);
			this.tag("PreprocessorDefinitions", defineLine);
			this.tag("CallAttributedProfiling", this.fastCap ? "Fastcap" : "Callcap");
		}
		this.tagEnd("ClCompile");
	}
}

module.exports = ClCompile;
