var Files = require('./Files.js');
var Paths = require('./Paths.js');
var fs = require('fs');

function Exporter() {

}

Exporter.prototype.writeFile = function (file) {
	this.out = fs.openSync(file.toString(), 'w');
};

Exporter.prototype.closeFile = function () {
	fs.closeSync(this.out);
};

Exporter.prototype.p = function (line, indent) {
	if (line === undefined) line = '';
	if (indent === undefined) indent = 0;
	var tabs = '';
	for (var i = 0; i < indent; ++i) tabs += '\t';
	var data = new Buffer(tabs + line + '\n');
	fs.writeSync(this.out, data, 0, data.length, null);
};

Exporter.prototype.copyFile = function (from, to) {
	Files.copy(from, to, true);
};

Exporter.prototype.copyDirectory = function (from, to) {
	this.createDirectory(to);
	var files = Files.newDirectoryStream(from);
	for (var f in files) {
		var file = Paths.get(from, files[f]);
		if (Files.isDirectory(file)) this.copyDirectory(file, to.resolve(file));
		else this.copyFile(file, to.resolve(files[f]));
	}
};

Exporter.prototype.createDirectory = function (dir) {
	if (!Files.exists(dir)) Files.createDirectories(dir);
};

module.exports = Exporter;
