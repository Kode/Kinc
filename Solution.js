var fs = require('fs');
var Path = require('./Path.js');
var Paths = require('./Paths.js');
var Platform = require('./Platform.js');
var Project = require('./Project.js');

function getDefines(platform, rotated) {
	var defines = [];
	switch (platform) {
		case Platform.Windows:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_WINDOWS");
			break;
		case Platform.WindowsRT:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_WINDOWSRT");
			break;
		case Platform.PlayStation3:
			defines.push("SYS_PS3");
			break;
		case Platform.iOS:
			if (rotated) defines.push("ROTATE90");
			defines.push("SYS_IOS");
			break;
		case Platform.OSX:
			defines.push("SYS_OSX");
			defines.push("SYS_64BIT");
			break;
		case Platform.Android:
			if (rotated) defines.push("ROTATE90");
			defines.push("SYS_ANDROID");
			break;
		case Platform.Xbox360:
			defines.push("_CRT_SECURE_NO_WARNINGS");
			defines.push("SYS_XBOX360");
		case Platform.HTML5:
			defines.push("SYS_HTML5");
			break;
		case Platform.Linux:
			defines.push("SYS_LINUX");
			break;
		case Platform.Tizen:
			defines.push("SYS_TIZEN");
			break;
	}
	return defines;
}

function Solution(name) {
	this.name = name;
	this.rotated = false;
	this.cmd = false;
	this.projects = [];
}

Solution.scriptdir = new Path('.');

Solution.prototype.getName = function () {
	return this.name;
};
	
Solution.prototype.getProjects = function () {
	return this.projects;
};

Solution.prototype.addProject = function (project) {
	this.projects.push(project);
};

Solution.prototype.searchFiles = function () {
	for (p in this.projects) this.projects[p].searchFiles();
};

Solution.prototype.flatten = function () {
	for (p in this.projects) this.projects[p].flatten();
};

Solution.createProject = function (filename) {
	var file = fs.readFileSync(Solution.scriptdir.resolve(Paths.get(filename, 'korefile.js')).toString(), { encoding: 'utf8' });
	var oldscriptdir = Solution.scriptdir;
	Solution.scriptdir = Solution.scriptdir.resolve(filename);
	var project = new Function(['Project'], file)(Project);
	Solution.scriptdir = oldscriptdir;
	return project;
};

Solution.createSolution = function (filename, platform) {
	var file = fs.readFileSync(Solution.scriptdir.resolve(Paths.get(filename, 'korefile.js')).toString(), {encoding: 'utf8'});
	var oldscriptdir = Solution.scriptdir;
	Solution.scriptdir = Solution.scriptdir.resolve(filename);
	var solution = new Function(['Solution', 'Project'], file)(Solution, Project);
	Solution.scriptdir = oldscriptdir;
	return solution;
};

/*
namespace {
	Platform platform;

	//Solution
	int Solution_new(lua_State* L) {
		const char* name = lua_tostring(L, 1);
		lua_newtable(L);
		new (lua_newuserdata(L, sizeof(Solution))) Solution(name);
		lua_setfield(L, -2, "this");

		luaL_getmetatable(L, "Solution.meta");
		lua_setmetatable(L, -2);

		return 1;
	}

	int Solution_createProject(lua_State* L) {
		Project* project = Solution::createProject(lua_tostring(L, 1));

		lua_newtable(L);
		new (lua_newuserdata(L, sizeof(Project))) Project(*project);
		lua_setfield(L, -2, "this");
		
		luaL_getmetatable(L, "Project.meta");
		lua_setmetatable(L, -2);

		return 1;
	}

	int Solution_addProject(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Solution* solution = (Solution*)lua_touserdata(L, -1);
		lua_getfield(L, 2, "this");
		Project* project = (Project*)lua_touserdata(L, -1);
		solution->addProject(project);
		return 0;
	}

	int Solution_setRotated(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Solution* solution = (Solution*)lua_touserdata(L, -1);
		solution->setRotated();
		return 0;
	}

	int Solution_setCmd(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Solution* solution = (Solution*)lua_touserdata(L, -1);
		solution->setCmd();
		return 0;
	}

	//Project
	int Project_new(lua_State* L) {
		const char* name = lua_tostring(L, 1);
		lua_newtable(L);
		new (lua_newuserdata(L, sizeof(Project))) Project(name);
		lua_setfield(L, -2, "this");
		
		luaL_getmetatable(L, "Project.meta");
		lua_setmetatable(L, -2);
		
		return 1;
	}

	int Project_addFile(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		project->addFile(lua_tostring(L, 2));
		return 0;
	}

	int Project_addFiles(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		int top = lua_gettop(L);
		for (int i = 2; i < top; ++i) project->addFile(lua_tostring(L, i));
		return 0;
	}

	int Project_addExclude(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		project->addExclude(lua_tostring(L, 2));
		return 0;
	}

	int Project_addExcludes(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		int top = lua_gettop(L);
		for (int i = 2; i < top; ++i) project->addExclude(lua_tostring(L, i));
		return 0;
	}

	int Project_addIncludeDir(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		project->addIncludeDir(lua_tostring(L, 2));
		return 0;
	}

	int Project_addIncludeDirs(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		int top = lua_gettop(L);
		for (int i = 2; i < top; ++i) project->addIncludeDir(lua_tostring(L, i));
		return 0;
	}

	int Project_addLibFor(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		project->addLibFor(lua_tostring(L, 2), lua_tostring(L, 3));
		return 0;
	}

	int Project_addLibsFor(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		int top = lua_gettop(L);
		for (int i = 3; i < top; ++i) project->addLibFor(lua_tostring(L, 2), lua_tostring(L, i));
		return 0;
	}

	int Project_addDefine(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		project->addDefine(lua_tostring(L, 2));
		return 0;
	}

	int Project_addDefines(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		int top = lua_gettop(L);
		for (int i = 2; i < top; ++i) project->addDefine(lua_tostring(L, i));
		return 0;
	}

	int Project_addLib(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		project->addLib(lua_tostring(L, 2));
		return 0;
	}

	int Project_addLibs(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		int top = lua_gettop(L);
		for (int i = 2; i < top; ++i) project->addLib(lua_tostring(L, i));
		return 0;
	}

	int Project_addSubProject(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		lua_getfield(L, 2, "this");
		Project* subproject = (Project*)lua_touserdata(L, -1);
		project->addSubProject(subproject);
		return 0;
	}

	int Project_setDebugDir(lua_State* L) {
		lua_getfield(L, 1, "this");
		if (!lua_touserdata(L, -1)) lua_error(L);
		Project* project = (Project*)lua_touserdata(L, -1);
		project->setDebugDir(lua_tostring(L, 2));
		return 0;
	}

	const luaL_Reg solutionLib[] = {
		{"new", Solution_new},
		{"createProject", Solution_createProject},
		{"addProject", Solution_addProject},
		{"rotate", Solution_setRotated},
		{"cmd", Solution_setCmd},
		{NULL, NULL}
	};

	const struct luaL_Reg projectLib[] = {
		{"new", Project_new},
		{"addFile", Project_addFile},
		{"addFiles", Project_addFiles},
		{"addExclude", Project_addExclude},
		{"addExcludes", Project_addExcludes},
		{"addIncludeDir", Project_addIncludeDir},
		{"addIncludeDirs", Project_addIncludeDirs},
		{"addDefine", Project_addDefine},
		{"addDefines", Project_addDefines},
		{"addLib", Project_addLib},
		{"addLibs", Project_addLibs},
		{"addLibFor", Project_addLibFor},
		{"addLibsFor", Project_addLibsFor},
		{"addSubProject", Project_addSubProject},
		{"setDebugDir", Project_setDebugDir},
		{NULL, NULL}
	};

	void initEnums(lua_State* L, Platform platform) {
		lua_pushinteger(L, (int)platform);
		lua_setglobal(L, "platform");

		lua_newtable(L);
		lua_pushinteger(L, (int)Platform::Android);
		lua_setfield(L, -2, "Android");
		lua_pushinteger(L, (int)Platform::HTML5);
		lua_setfield(L, -2, "HTML5");
		lua_pushinteger(L, (int)Platform::iOS);
		lua_setfield(L, -2, "iOS");
		lua_pushinteger(L, (int)Platform::Linux);
		lua_setfield(L, -2, "Linux");
		lua_pushinteger(L, (int)Platform::OSX);
		lua_setfield(L, -2, "OSX");
		lua_pushinteger(L, (int)Platform::PlayStation3);
		lua_setfield(L, -2, "PlayStation3");
		lua_pushinteger(L, (int)Platform::Windows);
		lua_setfield(L, -2, "Windows");
		lua_pushinteger(L, (int)Platform::WindowsRT);
		lua_setfield(L, -2, "WindowsRT");
		lua_pushinteger(L, (int)Platform::Xbox360);
		lua_setfield(L, -2, "Xbox360");
		lua_pushinteger(L, (int)Platform::Tizen);
		lua_setfield(L, -2, "Tizen");
		lua_setglobal(L, "Platform");

		lua_pushinteger(L, (int)Options::getGraphicsApi());
		lua_setglobal(L, "gfx");

		lua_newtable(L);
		lua_pushinteger(L, (int)GraphicsApi::OpenGL);
		lua_setfield(L, -2, "OpenGL");
		lua_pushinteger(L, (int)GraphicsApi::OpenGL2);
		lua_setfield(L, -2, "OpenGL2");
		lua_pushinteger(L, (int)GraphicsApi::Direct3D9);
		lua_setfield(L, -2, "Direct3D9");
		lua_pushinteger(L, (int)GraphicsApi::Direct3D11);
		lua_setfield(L, -2, "Direct3D11");
		lua_setglobal(L, "Graphics");
	}
}
*/
Solution.evalProjectScript = function (script) {
	/*
	lua_State* L = luaL_newstate();
	luaopen_base(L);
	luaopen_string(L);
	luaopen_math(L);

	luaL_newlib(L, solutionLib);
	lua_setglobal(L, "Solution");
	luaL_newlib(L, projectLib);
	lua_setglobal(L, "Project");

	initEnums(L, platform);
		
	luaL_newmetatable(L, "Solution.meta");
	lua_getglobal(L, "Solution");
	lua_setfield(L, -2, "__index");

	luaL_newmetatable(L, "Project.meta");
	lua_getglobal(L, "Project");
	lua_setfield(L, -2, "__index");

	script.seekg(0, script.end);
	unsigned length = static_cast<unsigned>(script.tellg());
	script.seekg(0, script.beg);
	char* buffer = new char[length + 1];
	script.read(buffer, length);
	buffer[length] = 0;
	
	int error = luaL_loadbuffer(L, buffer, length, "project");
	if (error == 0) error = lua_pcall(L, 0, 0, 0);

	if (error != 0) {
		std::cerr << "\nError: " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1);
	}

	lua_getglobal(L, "project");
	lua_getfield(L, -1, "this");
	if (!lua_touserdata(L, -1)) lua_error(L);
	Project* project = (Project*)lua_touserdata(L, -1);

	return project;
	*/
};

Solution.evalSolutionScript = function (script, platform) {
	this.platform = platform;
	/*
	lua_State* L = luaL_newstate();
	luaopen_base(L);
	luaopen_string(L);
	luaopen_math(L);

	luaL_newlib(L, solutionLib);
	lua_setglobal(L, "Solution");
	luaL_newlib(L, projectLib);
	lua_setglobal(L, "Project");

	initEnums(L, platform);
		
	luaL_newmetatable(L, "Solution.meta");
	lua_getglobal(L, "Solution");
	lua_setfield(L, -2, "__index");

	luaL_newmetatable(L, "Project.meta");
	lua_getglobal(L, "Project");
	lua_setfield(L, -2, "__index");
	
	script.seekg(0, script.end);
	unsigned length = static_cast<unsigned>(script.tellg());
	script.seekg(0, script.beg);
	char* buffer = new char[length + 1];
	script.read(buffer, length);
	buffer[length] = 0;
	
	int error = luaL_loadbuffer(L, buffer, length, "solution");
	if (error == 0) error = lua_pcall(L, 0, 0, 0);

	if (error != 0) {
		std::cerr << "\nError: " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1);
	}

	lua_getglobal(L, "solution");
	lua_getfield(L, -1, "this");
	if (!lua_touserdata(L, -1)) lua_error(L);
	Solution* solution = (Solution*)lua_touserdata(L, -1);

	return solution;
	*/
};

Solution.create = function (directory, platform) {
	Solution.scriptdir = directory;
	var solution = Solution.createSolution('.', platform);
	var defines = getDefines(platform, solution.isRotated());
	for (p in solution.projects) {
		for (d in defines) solution.projects[p].addDefine(defines[d]);
	}
	return solution;
};

Solution.prototype.isRotated = function () {
	return this.rotated;
};

Solution.prototype.isCmd = function () {
	return this.cmd;
};

Solution.prototype.setRotated = function () {
	this.rotated = true;
};

Solution.prototype.setCmd = function () {
	this.cmd = true;
};

module.exports = Solution;
