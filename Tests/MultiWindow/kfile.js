const project = new Project('MultiWindow');

await project.addProject('../../');

project.addFile('Sources/**');
project.addDefine("VALIDATE");
project.addDefine("KINC_WAYLAND_FORCE_CSD");
project.setDebugDir('Deployment');

project.flatten();

resolve(project);
