var fs = require('fs');

function Block(out, indentation) {
	this.out = out;
	this.indentation = indentation;
}

Block.prototype.indent = function () {
	++this.indentation;
}

Block.prototype.unindent = function () {
	--this.indentation;
}

Block.prototype.tag = function (name, value) {
	this.p("<" + name + ">" + value + "</" + name + ">");
}

Block.prototype.tagStart = function (name) {
	this.p("<" + name + ">");
	this.indent();
}

Block.prototype.tagEnd = function (name) {
	this.unindent();
	this.p("</" + name + ">");
}

Block.prototype.p = function (line) {
	if (line === undefined) line = '';
	var tabs = '';
	for (var i = 0; i < this.indentation; ++i) tabs += '\t';
	var data = new Buffer(tabs + line + '\n');
	fs.writeSync(this.out, data, 0, data.length, null);
}

module.exports = Block;
