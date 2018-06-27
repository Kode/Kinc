let project = new Project('Empty');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

resolve(project);
