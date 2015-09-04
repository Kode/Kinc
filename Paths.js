"use strict";

const path = require('path');
const Path = require('./Path.js');

exports.get = function () {
	if (arguments.length == 1 && arguments[0] === '') return new Path('');
	let pathstrings = [];
	for (let i = 0; i < arguments.length; ++i) {
		if (typeof (arguments[i]) === 'string') pathstrings.push(arguments[i]);
		else pathstrings.push(arguments[i].path);
	}
	return new Path(path.join.apply(null, pathstrings));
};

exports.executableDir = function () {
	return new Path(__dirname);
};
