"use strict";

const Files = require('./Files.js');
const Paths = require('./Paths.js');
const fs = require('fs');

class Exporter {
	constructor() {

	}

	writeFile(file) {
		this.out = fs.openSync(file.toString(), 'w');
	}

	closeFile() {
		fs.closeSync(this.out);
	}

	p(line, indent) {
		if (line === undefined) line = '';
		if (indent === undefined) indent = 0;
		let tabs = '';
		for (let i = 0; i < indent; ++i) tabs += '\t';
		let data = new Buffer(tabs + line + '\n');
		fs.writeSync(this.out, data, 0, data.length, null);
	}

	copyFile(from, to) {
		Files.copy(from, to, true);
	}

	copyDirectory(from, to) {
		this.createDirectory(to);
		let files = Files.newDirectoryStream(from);
		for (let f in files) {
			let file = Paths.get(from, files[f]);
			if (Files.isDirectory(file)) this.copyDirectory(file, to.resolve(file));
			else this.copyFile(file, to.resolve(files[f]));
		}
	}

	createDirectory(dir) {
		if (!Files.exists(dir)) Files.createDirectories(dir);
	}
}

module.exports = Exporter;
