let project = new Project('MultiWindow');

project.addFile('Sources/**');
project.setDebugDir('Deployment');
project.addDefine("VALIDATE");
project.addDefine("KINC_WAYLAND_FORCE_CSD");

resolve(project);