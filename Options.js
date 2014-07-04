var GraphicsApi = require('./GraphicsApi.js');
var VisualStudioVersion = require('./VisualStudioVersion.js');

module.exports = {
	precompiledHeaders: false,
	intermediateDrive: '',
	graphicsApi: GraphicsApi.Direct3D9,
	visualStudioVersion: VisualStudioVersion.VS2013,
};
