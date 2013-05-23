solution = Solution.new("ShaderTest")
project = Project.new("ShaderTest")

project:addFile("Sources/**")
project:setDebugDir("Deployment")

project:addSubProject(Solution.createProject("../.."))

solution:addProject(project)
