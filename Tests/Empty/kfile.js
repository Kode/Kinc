let project = new Project('Empty');

project.cpp = true;

await project.addProject('../../');
project.flatten();

project.addFile('Sources/**');
project.setDebugDir('Deployment');

resolve(project);
