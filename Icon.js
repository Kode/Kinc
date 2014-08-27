var cp = require('child_process');
var fs = require('fs');
var os = require('os');
var path = require('path');

function run(from, to, width, height, format, callback) {
	var exe = "kraffiti-osx";
	if (os.platform() === "linux") {
		exe = "kraffiti-linux";
	}
	else if (os.platform() === "win32") {
		exe = "kraffiti.exe";
	}

	var child = cp.spawn(path.join(__dirname, '..', 'kraffiti', exe), ['from=' + from, 'to=' + to, 'width=' + width, 'height=' + height, 'format=' + format]);
	
	child.stdout.on('data', function (data) {
		//log.info(data);
	});
	
	child.stderr.on('data', function (data) {
		//log.info(data);
	});
	
	child.on('error', function (err) {
		//log.error('kraffiti error');
	});
	
	child.on('close', callback);
}

function findIcon(from) {
	if (fs.existsSync(path.join(from, 'icon.png'))) return path.join(from, 'icon.png');
	else return path.join(__dirname, '..', 'kraffiti', 'ball.png');
}

exports.exportIco = function (to, from) {
	run(findIcon(from.toString()), to.toString(), 0, 0, 'ico', function () { });
};
