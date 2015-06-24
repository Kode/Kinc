var cp = require('child_process');
var fs = require('fs');
var os = require('os');
var path = require('path');
var log = require('./log.js');

function run(from, to, width, height, format, background, callback) {
	var exe = "kraffiti-osx";
	if (os.platform() === "linux") {
		exe = "kraffiti-linux";
	}
	else if (os.platform() === "win32") {
		exe = "kraffiti.exe";
	}
	
	var params = ['from=' + from, 'to=' + to, 'width=' + width, 'height=' + height, 'format=' + format, 'keepaspect'];
	if (background !== undefined) params.push('background=' + background.toString(16));
	var child = cp.spawn(path.join(__dirname, '..', 'kraffiti', exe), params);
	
	child.stdout.on('data', function (data) {
		//log.info('kraffiti stdout: ' + data);
	});
	
	child.stderr.on('data', function (data) {
		log.error('kraffiti stderr: ' + data);
	});
	
	child.on('error', function (err) {
		log.error('kraffiti error: ' + err);
	});
	
	child.on('close', function (code) {
		if (code !== 0) log.error('kraffiti exited with code ' + code);
		callback();
	});
}

function findIcon(from) {
	if (fs.existsSync(path.join(from, 'icon.png'))) return path.join(from, 'icon.png');
	else return path.join(__dirname, '..', 'kraffiti', 'ball.png');
}

exports.exportIco = function (to, from) {
	run(findIcon(from.toString()), to.toString(), 0, 0, 'ico', undefined, function () { });
};

exports.exportIcns = function (to, from) {
	run(findIcon(from.toString()), to.toString(), 0, 0, 'icns', undefined, function () { });
};

exports.exportPng = function (to, width, height, background, from) {
	run(findIcon(from.toString()), to.toString(), width, height, 'png', background, function () { });
};
