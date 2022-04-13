let project = new Project('SIMD');

await project.addProject('../../');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

project.flatten();

resolve(project);
