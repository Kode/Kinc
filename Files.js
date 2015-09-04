"use strict";

const fs = require('fs');
const pa = require('path');
const execSync = require('child_process').execSync;

function filesDiffer(file1, file2) {
	// Treat them as different if one of them does not exist.
	if (!fs.existsSync(file1)) return true;
	if (!fs.existsSync(file2)) return true;

	let isDifferent = true;
	let output;
	try {
		output = execSync("fc " + file1 + " " + file2, { encoding: 'utf8' });
	}
	catch (error) {
		output = "";
	}
	if(output.indexOf("no differences encountered") > -1) {
		isDifferent = false;
	}
	return isDifferent;
}

exports.exists = function (path) {
	return fs.existsSync(path.path);
};

exports.createDirectories = function (path) {
	let dirs = path.path.split(pa.sep);
	let root = '';

	while (dirs.length > 0) {
		let dir = dirs.shift();
		if (dir === '') { // If directory starts with a /, the first path will be an empty string.
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
	if (replace || !fs.existsSync(to.path)) {
        fs.writeFileSync(to.path, fs.readFileSync(from.path));
    }
};

exports.copyIfDifferent = function (from, to, replace) {
	exports.createDirectories(to.parent());
	if (replace || !fs.existsSync(to.path)) {
		if (filesDiffer(to.path, from.path)) {
			fs.writeFileSync(to.path, fs.readFileSync(from.path));
			//console.log("Copying differing file: " + from.path);
		}
		else {
			//console.log("Skipped file: " + from.path);
		}
	}
};

exports.newDirectoryStream = function (path) {
	return fs.readdirSync(path.path);
};

var rmdir = function (dir) {
	let list = fs.readdirSync(dir);
	for (let i = 0; i < list.length; ++i) {
		let filename = pa.join(dir, list[i]);
		let stat = fs.statSync(filename);
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
