var path = require('path');
var Path = require('./Path.js');

exports.get = function (a) {
	var pathstrings = [];
	for (var i = 0; i < arguments.length; ++i) {
		if (typeof (arguments[i]) === 'string') pathstrings.push(arguments[i]);
		else pathstrings.push(arguments[i].path);
	}
	return new Path(path.join.apply(null, pathstrings));
};

exports.executableDir = function () {
	return new Path(__dirname);
};
