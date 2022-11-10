const project = new Project('Shader-G5');

await project.addProject('../../');

project.addFile('Sources/**');
project.addFile('Shaders/**');
project.setDebugDir('Deployment');

project.flatten();

resolve(project);
