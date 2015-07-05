var path = require('path');

var Path = function (path) {
	this.path = path;
};

Path.prototype.startsWith = function (other) {
	var me = this.path;
	var he = other.path;
	if (he == '.') return true;
	if (me[0] == '.' && me[1] == '/') me = me.substr(2);
	if (he[0] == '.' && he[1] == '/') he = he.substr(2);
	for (var i = 0; i < he.length; ++i) {
		if (me[i] != he[i]) return false;
	}
	return true;
};

Path.prototype.relativize = function (other) {
	return new Path(path.relative(this.path, other.path));
};

Path.prototype.resolve = function (subpath) {
	if (typeof (subpath) !== 'string') subpath = subpath.path;
	if (path.isAbsolute(subpath)) return new Path(subpath);
	return new Path(path.join(this.path, subpath));
};

Path.prototype.parent = function () {
	if (this.path == ".") return this.toAbsolutePath().parent();
	else {
		for (var i = this.path.length - 1; i >= 0; --i) {
			if (this.path[i] == '/' || this.path[i] == '\\') {
				return require('./Paths.js').get(this.path.substr(0, i));
			}
		}
	}
	return this;
};

Path.prototype.getFileName = function () {
	return path.basename(this.path);
};

Path.prototype.toString = function () {
	return path.normalize(this.path);
};

Path.prototype.isAbsolute = function () {
	return (this.path.length > 0 && this.path[0] == '/') || (this.path.length > 1 && this.path[1] == ':');
};

Path.prototype.toAbsolutePath = function () {
	if (this.isAbsolute()) return this;
	return new Path(path.resolve(process.cwd(), this.path));
};

module.exports = Path;
