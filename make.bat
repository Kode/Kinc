@if exist "%~dp0Tools\windows_x64\kmake.exe" (
	@call "%~dp0Tools\windows_x64\kmake.exe" %*
) else (
	echo kmake was not found, please run the get_dlc script.
)
