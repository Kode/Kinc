let project = new Project('Empty', __dirname);

project.addFile('Sources/**');
project.setDebugDir('Deployment');

Project.createProject('../../', __dirname).then((kore) => {
	project.addSubProject(kore);
	resolve(project);
});
