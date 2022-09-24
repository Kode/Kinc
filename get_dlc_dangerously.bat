@pushd "%~dp0"
@if exist Tools\windows_x64\icon.png (
@git submodule update --remote --merge Tools/windows_x64
) else (
@git submodule update --init --remote Tools/windows_x64
@git -C Tools/windows_x64 checkout main
)
@popd