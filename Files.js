var fs = require('fs');
var pa = require('path');

exports.exists = function (path) {
	return fs.existsSync(path.path);
};

exports.createDirectories = function (path) {
	var dirs = path.path.split(pa.sep);
	var root = "";

	while (dirs.length > 0) {
		var dir = dirs.shift();
		if (dir === "") { // If directory starts with a /, the first path will be an empty string.
			root = pa.sep;
		}
		if (!fs.existsSync(root + dir)) {
			fs.mkdirSync(root + dir);
		}
		root += dir + pa.sep;
	}
};

exports.isDirectory = function (path) {
	if (!this.exists(path)) return false;
	return fs.statSync(path.path).isDirectory();
};

exports.copy = function (from, to, replace) {
	exports.createDirectories(to.parent());
	if (replace || !fs.existsSync(to.path)) fs.writeFileSync(to.path, fs.readFileSync(from.path));
};

exports.newDirectoryStream = function (path) {
	return fs.readdirSync(path.path);
};

var rmdir = function (dir) {
	var list = fs.readdirSync(dir);
	for (var i = 0; i < list.length; ++i) {
		var filename = pa.join(dir, list[i]);
		var stat = fs.statSync(filename);
		if (filename == "." || filename == "..") {

		}
		else if (stat.isDirectory()) {
			rmdir(filename);
		}
		else {
			fs.unlinkSync(filename);
		}
	}
	fs.rmdirSync(dir);
};

exports.removeDirectory = function (path) {
	if (fs.existsSync(path.path)) rmdir(path.path);
};
