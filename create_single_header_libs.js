const fs = require('fs');
const path = require('path');

const headers = {};

function findHeader(headerPath) {
	const cachedHeader = headers[headerPath];
	if (cachedHeader) {
		return cachedHeader;
	}

	const filePath = path.resolve('Sources', headerPath);
	let header = fs.readFileSync(filePath, {encoding: 'utf8'});
	header = header.replace('#pragma once', '');
	for (const headerPath in headers) {
		header = header.replace('#include <' + headerPath + '>', '');
	}
	header = header.trim();
	headers[headerPath] = header;
	return header;
}

function insertHeaders(file, headerPaths) {
	for (const headerPath of headerPaths) {
		file = file.replace('#include <' + headerPath + '>', findHeader(headerPath));
	}
	return file;
}

const audio2_header = fs.readFileSync(path.resolve('Sources', 'kinc', 'audio2', 'audio.h'), {encoding: 'utf8'});
let lib = insertHeaders(audio2_header, ['kinc/global.h']);

let windows_backend = fs.readFileSync(path.resolve('Backends', 'Audio2', 'WASAPI', 'Sources', 'kinc', 'backend', 'wasapi.c'), {encoding: 'utf8'});
windows_backend = windows_backend.replace('#include <kinc/audio2/audio.h>', '');
windows_backend = insertHeaders(windows_backend, ['kinc/error.h', 'kinc/log.h']);
windows_backend = windows_backend.trim();

lib = lib.replace('// BACKENDS-PLACEHOLDER', '#ifdef KORE_WINDOWS\n' + windows_backend + '\n#endif');

if (!fs.existsSync('single_header_libs')) {
	fs.mkdirSync('single_header_libs');
}
fs.writeFileSync(path.resolve('single_header_libs', 'audio2.h'), lib, {encoding: 'utf8'});
