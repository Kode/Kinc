var args = process.argv;

var from = '.';
var to = 'build';

if (os.platform() === "linux") {
	var platform = Platform.Linux;
}
else if (os.platform() === "win32") {
	var platform = Platform.Windows;
}
else {
	var platform = Platform.OSX;
}

for (var i = 2; i < args.length; ++i) {
	var arg = args[i];
	
	if (arg == "pch") Options.setPrecompiledHeaders(true);
	else if (arg.startsWith("intermediate=")) Options.setIntermediateDrive(arg.substr(13));
	else if (arg.startsWith("gfx=")) Options.setGraphicsApi(arg.substr(4));
	else if (arg.startsWith("vs=")) Options.setVisualStudioVersion(arg.substr(3));

	else if (arg.startsWith("from=")) from = arg.substr(5);
	else if (arg.startsWith("to=")) to = arg.substr(3);

	else {
		for (p in Platform) {
			if (arg === Platform[p]) {
				platform = Platform[p];
			}
		}
	}
}

require('./main.js').run(
{
	from: from,
	to: to,
	platform: platform
},
{
	info: console.log,
	error: console.log
}, function () { });
