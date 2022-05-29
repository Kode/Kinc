const project = new Project('MultiWindow');

await project.addProject('../../');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

project.flatten();

resolve(project);
