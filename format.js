const child_process = require('child_process');
const fs = require('fs');
const path = require('path');

const excludes = JSON.parse(fs.readFileSync('format-excludes.json', 'utf8'));

function isExcluded(filepath) {
	filepath = filepath.replace(/\\/g, '/');
	for (let exclude of excludes) {
		if (filepath === exclude) return true;
	}
	return false;
}

function format(dir) {
	const files = fs.readdirSync(dir);
	for (let file of files) {
		if (file.startsWith('.')) continue;
		let filepath = path.join(dir, file);
		let info = fs.statSync(filepath);
		if (info.isDirectory()) {
			format(filepath);
		}
		else {
			if (isExcluded(filepath)) continue;
			if (file.endsWith('.cpp') || file.endsWith('.h')) {
				console.log('Format ' + filepath);
				child_process.execFileSync('clang-format', ['-style=file', '-i', filepath]);
			}
		}
	}
}

format('.');
