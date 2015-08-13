var os = require('os');
var Options = require('./Options.js');
var Platform = require('./Platform.js');
var GraphicsApi = require('./GraphicsApi.js');
var VisualStudioVersion = require('./VisualStudioVersion.js');

if (!String.prototype.startsWith) {
	Object.defineProperty(String.prototype, 'startsWith', {
		enumerable: false,
		configurable: false,
		writable: false,
		value: function (searchString, position) {
			position = position || 0;
			return this.indexOf(searchString, position) === position;
		}
	});
}

if (!String.prototype.endsWith) {
	Object.defineProperty(String.prototype, 'endsWith', {
		enumerable: false,
		configurable: false,
		writable: false,
		value: function (searchString, position) {
			position = position || this.length;
			position = position - searchString.length;
			var lastIndex = this.lastIndexOf(searchString);
			return lastIndex !== -1 && lastIndex === position;
		}
	});
}


var defaultTarget;
if (os.platform() === "linux") {
	defaultTarget = Platform.Linux;
}
else if (os.platform() === "win32") {
	defaultTarget = Platform.Windows;
}
else {
	defaultTarget = Platform.OSX;
}

var options = [
	{
		full: 'from',
		value: true,
		description: 'Location of your project',
		default: '.'

	},
	{
		full: 'to',
		value: true,
		description: 'Build location',
		default: 'build'
	},
	{
		full: 'emcc',
		value: true,
		description: 'Location of emscripten shader compiler',
	},
	{
		full: 'pch',
		description: 'Use precompiled headers for C++ targets',
		value: false
	},
	{
		full: 'intermediate',
		description: 'Intermediate location for object files.',
		value: true,
		default: '',
		hidden: true
	},
	{
		full: 'visualstudio',
		short: 'vs',
		description: 'Version of Visual Studio to use',
		value: true,
		default: VisualStudioVersion.VS2015
	},
	{
		full: 'nokrafix',
		description: 'Switch off the new shader compiler',
		value: false
	},
	{
		full: 'compile',
		description: 'Compile executable',
		value: false
	},
	{
		full: 'run',
		description: 'Run executable',
		value: false
	},
	{
		full: 'update',
		description: 'Update Kore and it\'s submodules',
		value: false
	},
	{
		full: 'platform',
		short: 'p',
		value: true,
		description: 'Target platform',
		default: defaultTarget,
		values: Platform
	},
	{
		full: 'graphicsApi',
		short: 'gfx',
		description: 'Graphics api to use',
		value: true,
		default: GraphicsApi.Direct3D9,
		values: GraphicsApi
	},
];

var parsedOptions = {

};

function printHelp() {
	console.log("Usage: ");
	console.log("node Kore/make.js <platform> [args]");
	console.log("")
	console.log("The possible options are:")

	for (var o in options) {
		var option = options[o];
		if (option.hidden) {
			continue;
		}

		var str = '';
		if (option.short) {
			str += '-' + option.short + ' '
		}
		str += '--' + option.full;

		// Add tabs to allow show in a fancy way
		if(str.length <= 6) {
			str += '\t\t\t';
		} else
		if(str.length <= 15) {
			str += '\t\t';
		} else {
			str += '\t';
		}

		str += option.description

		// Add the possible values
		if(option.default) {
			str += ", default '";
			str += option.default;
			str += "'";
		}

		// Add the possible values
		if(option.values) {
			str += ", options: \n";
			for (v in option.values) {
				str += " * " + option.values[v] + "\n";
			}
		}

		console.log(str);
	}
}

for (var o in options) {
	var option = options[o];
	if (option.value) {
		parsedOptions[option.full] = option.default;
	}
	else {
		parsedOptions[option.full] = false;
	}
}


var args = process.argv;

for (var i = 2; i < args.length; ++i) {
	var arg = args[i];

	if (arg[0] == '-') {
		if (arg[1] == '-') {
			if (arg.substr(2) === 'help') {
				printHelp();
				process.exit(0);
			}
			for (var o in options) {
				var option = options[o];
				if (arg.substr(2) === option.full) {
					if (option.value) {
						++i;
						parsedOptions[option.full] = args[i];
					}
					else {
						parsedOptions[option.full] = true;
					}
				} else
				if(arg.substr(2).startsWith(option.full + '=')) {
					if (option.value) {
						parsedOptions[option.full] = arg.substr(option.full.length + 3); // + 3 because we have two -- and 1 =
					} else {
						parsedOptions[option.full] = true;
					}
				}
			}
		}
		else {
			if (arg[1] === 'h') {
				printHelp();
				process.exit(0);
			}
			for (var o in options) {
				var option = options[o];

				if (option.short && arg.substr(1) === option.short) {
					if (option.value) {
						++i;
						parsedOptions[option.full] = args[i];
					} else {
						parsedOptions[option.full] = true;
					}
				} else
				if(option.short && arg.substr(1).startsWith(option.short + '=')) {
					if (option.value) {
						parsedOptions[option.full] = arg.substr(option.short.length + 2); // + 2 because we have one - and 1 =
					} else {
						parsedOptions[option.full] = true;
					}
				}
			}
		}
	}
	else {
		parsedOptions.platform = arg;
	}
}

if (parsedOptions.graphicsApi === GraphicsApi.OpenGL) {
	parsedOptions.graphicsApi = GraphicsApi.OpenGL2;
}

if (parsedOptions.run) {
	parsedOptions.compile = true;
}

if(!parsedOptions.emcc && parsedOptions.platform == Platform.HTML5) {
	console.error("For HTML5 platforms you need to set the emcc path");
	process.exit(0);
}

if(parsedOptions.update) {
	console.log("Updating everything...");
	require('child_process').spawnSync('git', ['submodule', 'foreach', '--recursive', 'git', 'pull', 'origin', 'master'], { stdio: 'inherit', stderr: 'inherit' });	
	process.exit(0);
}

require('./main.js').run(
	parsedOptions,
{
	info: console.log,
	error: console.log
}, function () { });
