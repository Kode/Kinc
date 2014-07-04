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

module.exports = Exporter;
