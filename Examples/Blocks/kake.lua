solution = Solution.new("BlocksFromHeaven")
project = Project.new("BlocksFromHeaven")

project:addFile("Sources/**")
project:setDebugDir("Deployment")

project:addSubProject(Solution.createProject("../.."))

solution:addProject(project)
