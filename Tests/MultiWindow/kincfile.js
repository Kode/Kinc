let project = new Project('MultiWindow');

project.addFile('Sources/**');
project.setDebugDir('Deployment');
project.addDefine("VALIDATE");

resolve(project);