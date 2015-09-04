"use strict";

let myInfo = function (text) {
	console.log(text);
};

let myError = function (text) {
	console.log(text);
};

exports.set = function (log) {
	myInfo = log.info;
	myError = log.error;
};

exports.info = function (text) {
	myInfo(text);
};

exports.error = function (text) {
	myError(text);
};
