var fs = require('fs');
var pathSep = require('path').sep;

exports.exists = function (path) {
	return fs.existsSync(path.path);
};

exports.createDirectories = function (path) {
	var dirs = path.path.split(pathSep);
	var root = "";

	while (dirs.length > 0) {
		var dir = dirs.shift();
		if (dir === "") { // If directory starts with a /, the first path will be an empty string.
			root = pathSep;
		}
		if (!fs.existsSync(root + dir)) {
			fs.mkdirSync(root + dir);
		}
		root += dir + pathSep;
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

exports.removeDirectory = function (path) {
	if (fs.existsSync(path.path)) fs.rmdirSync(path.path);
};
