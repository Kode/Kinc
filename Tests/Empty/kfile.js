let project = new Project('Empty');

project.cpp = true;

await project.addProject('../../');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

project.flatten();

resolve(project);
