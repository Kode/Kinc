const fs = require('fs');
const path = require('path');

const headers = {};

function miniPreprocessor(source) {
	const normal = 0;
	const lineStart = 1;
	const pragma = 2;
	let mode = lineStart;
	let currentPragma = null;

	let processed = '';
	
	for (let i = 0; i < source.length; ++i) {
		if (mode === normal) {
			if (source.charAt(i) === '\n' || source.charAt(i) === '\r') {
				mode = lineStart;
			}
			processed += source.charAt(i);
		}
		else if (mode === lineStart) {
			if (source.charAt(i) === '#') {
				currentPragma = '';
				mode = pragma;
			}
			else if (source.charAt(i) === '\n' || source.charAt(i) === '\r') {
				processed += source.charAt(i);
				mode = lineStart;
			}
			else {
				processed += source.charAt(i);
				mode = normal;
			}
		}
		else if (mode === pragma) {
			if (source.charAt(i) === '\n' || source.charAt(i) === '\r') {
				if (currentPragma.startsWith('include')) {
					let start = currentPragma.indexOf('<');
					if (start < 0) {
						start = currentPragma.indexOf('"');
					}
					let end = currentPragma.lastIndexOf('>');
					if (end < 0) {
						end = currentPragma.lastIndexOf('"');
					}

					if (start >= 0 && end >= 0 && currentPragma.length > start + 1) {
						const headerPath = currentPragma.substring(start + 1, end);

						if (currentPragma.substring(start + 1).startsWith('kinc')) {
							let filePath = null;
							if (headerPath.includes('FileReaderImpl') || headerPath.includes('Android')) {
								if (!headers[headerPath]) {
									headers[headerPath] = true;
									processed += '#' + currentPragma + source.charAt(i);
								}
							}
							else {
								if (headerPath.startsWith('kinc/backend')) {
									filePath = path.resolve('Backends', 'System', 'Microsoft', 'Sources', headerPath);
								}
								else {
									filePath = path.resolve('Sources', headerPath);
								}
							
								if (!headers[headerPath]) {
									headers[headerPath] = true;

									let header = fs.readFileSync(filePath, {encoding: 'utf8'});
									console.log('Preprocessing ' + filePath);
									header = miniPreprocessor(header);
									processed += header;
								}
							}
						}
						else {
							if (!headers[headerPath]) {
								headers[headerPath] = true;
								processed += '#' + currentPragma + source.charAt(i);
							}
						}
					}
					else {
						processed += '#' + currentPragma + source.charAt(i);
					}
				}
				else if (currentPragma.startsWith('pragma once')) {
					// ignore
				}
				else {
					processed += '#' + currentPragma + source.charAt(i);
				}
				mode = lineStart;
			}
			else {
				currentPragma += source.charAt(i);
			}
		}
		else {
			throw 'Unknown mode';
		}
	}

	return processed;
}

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
let lib = '#pragma once\n' + miniPreprocessor(audio2_header);

let windows_backend = fs.readFileSync(path.resolve('Backends', 'Audio2', 'WASAPI', 'Sources', 'kinc', 'backend', 'wasapi.c'), {encoding: 'utf8'});
windows_backend = windows_backend.replace('#include <kinc/audio2/audio.h>', '');
windows_backend = miniPreprocessor(windows_backend);

lib = lib.replace('// BACKENDS-PLACEHOLDER', '#ifdef KINC_WINDOWS\n' + windows_backend + '\n#endif');

if (!fs.existsSync('single_header_libs')) {
	fs.mkdirSync('single_header_libs');
}
fs.writeFileSync(path.resolve('single_header_libs', 'kinc_audio2.h'), lib, {encoding: 'utf8'});
