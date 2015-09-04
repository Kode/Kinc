"use strict";

const GraphicsApi = require('./GraphicsApi.js');
const VisualStudioVersion = require('./VisualStudioVersion.js');

module.exports = {
	precompiledHeaders: false,
	intermediateDrive: '',
	graphicsApi: GraphicsApi.Direct3D9,
	visualStudioVersion: VisualStudioVersion.VS2013,
	compile: false,
	run: false
};
