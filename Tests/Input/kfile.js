let project = new Project('Input');

await project.addProject('../../');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

project.flatten();

resolve(project);
