"use strict";

const pathlib = require('path');

class Path {
	constructor(path) {
		this.path = path;
	}

	startsWith(other) {
		let me = this.path;
		let he = other.path;
		if (he == '.') return true;
		if (me[0] == '.' && me[1] == '/') me = me.substr(2);
		if (he[0] == '.' && he[1] == '/') he = he.substr(2);
		for (let i = 0; i < he.length; ++i) {
			if (me[i] != he[i]) return false;
		}
		return true;
	}

	relativize(other) {
		return new Path(pathlib.relative(this.path, other.path));
	}

	resolve(subpath) {
		if (typeof (subpath) !== 'string') subpath = subpath.path;
		if (pathlib.isAbsolute(subpath)) return new Path(subpath);
		return new Path(pathlib.join(this.path, subpath));
	}

	parent() {
		if (this.path == ".") return this.toAbsolutePath().parent();
		else {
			for (var i = this.path.length - 1; i >= 0; --i) {
				if (this.path[i] == '/' || this.path[i] == '\\') {
					return require('./Paths.js').get(this.path.substr(0, i));
				}
			}
		}
		return this;
	}

	getFileName() {
		return pathlib.basename(this.path);
	}

	toString() {
		return pathlib.normalize(this.path);
	}

	toEscapedString() {
		return this.toString().replace(/ /g,"\\\\ ");
	}

	isAbsolute() {
		return (this.path.length > 0 && this.path[0] == '/') || (this.path.length > 1 && this.path[1] == ':');
	}

	toAbsolutePath() {
		if (this.isAbsolute()) return this;
		return new Path(pathlib.resolve(process.cwd(), this.path));
	}
}

module.exports = Path;
