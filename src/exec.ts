import * as os from 'os';

export function sys() {
	if (os.platform() === 'linux') {
		if (os.arch() === 'arm') return '-linuxarm';
		else if (os.arch() === 'x64') return '-linux64';
		else return '-linux32';
	}
	else if (os.platform() === 'win32') {
		return '.exe';
	}
	else {
		return '-osx';
	}
}
