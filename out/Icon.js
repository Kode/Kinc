"use strict";
const cp = require('child_process');
const fs = require('fs');
const path = require('path');
const log = require('./log');
const exec = require('./exec');
function run(from, to, width, height, format, background, callback) {
    const exe = 'kraffiti' + exec.sys();
    let params = ['from=' + from, 'to=' + to, 'format=' + format, 'keepaspect'];
    if (width > 0)
        params.push('width=' + width);
    if (height > 0)
        params.push('height=' + height);
    if (background !== undefined)
        params.push('background=' + background.toString(16));
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
        if (code !== 0)
            log.error('kraffiti exited with code ' + code);
        callback();
    });
}
function findIcon(from) {
    if (fs.existsSync(path.join(from, 'icon.png')))
        return path.join(from, 'icon.png');
    else
        return path.join(__dirname, '..', '..', 'kraffiti', 'ball.png');
}
function exportIco(to, from) {
    run(findIcon(from.toString()), to.toString(), 0, 0, 'ico', undefined, function () { });
}
exports.exportIco = exportIco;
function exportIcns(to, from) {
    run(findIcon(from.toString()), to.toString(), 0, 0, 'icns', undefined, function () { });
}
exports.exportIcns = exportIcns;
function exportPng(to, width, height, background, from) {
    run(findIcon(from.toString()), to.toString(), width, height, 'png', background, function () { });
}
exports.exportPng = exportPng;
//# sourceMappingURL=Icon.js.map