import * as cp from 'child_process';
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as log from './log';
import * as exec from './exec';

function run(from: string, to: string, width: number, height: number, format: string, background, callback) {
	const exe = 'kraffiti' + exec.sys();
	let params = ['from=' + from, 'to=' + to, 'format=' + format, 'keepaspect'];
	if (width > 0) params.push('width=' + width);
	if (height > 0) params.push('height=' + height);
	if (background !== undefined) params.push('background=' + background.toString(16));
	let child = cp.spawn(path.join(__dirname, '..', '..', 'kraffiti', exe), params);
	
	child.stdout.on('data', (data) => {
		//log.info('kraffiti stdout: ' + data);
	});
	
	child.stderr.on('data', (data) => {
		log.error('kraffiti stderr: ' + data);
	});
	
	child.on('error', (err) => {
		log.error('kraffiti error: ' + err);
	});
	
	child.on('close', (code) => {
		if (code !== 0) log.error('kraffiti exited with code ' + code);
		callback();
	});
}

function findIcon(from: string) {
	if (fs.existsSync(path.join(from, 'icon.png'))) return path.join(from, 'icon.png');
	else return path.join(__dirname, '..', '..', 'kraffiti', 'ball.png');
}

export function exportIco(to: string, from: string) {
	run(findIcon(from.toString()), to.toString(), 0, 0, 'ico', undefined, function () { });
}

export function exportIcns(to: string, from: string) {
	run(findIcon(from.toString()), to.toString(), 0, 0, 'icns', undefined, function () { });
}

export function exportPng(to: string, width: number, height: number, background, from: string) {
	run(findIcon(from.toString()), to.toString(), width, height, 'png', background, function () { });
}
