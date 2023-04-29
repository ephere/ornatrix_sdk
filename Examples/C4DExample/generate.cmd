:: Generate Ornatrix C4D SDK Example files

:: Configure
@echo off

setlocal

set TOOL_PATH=..\..\..\..\..\External\Maxon\Cinema4D\ProjectTool\cinema4d_r21_project_tool_20190903
if %TOOL_PATH% == "" (
	echo TOOL_PATH not set
	exit /B 1
)
if not exist %TOOL_PATH% (
	echo TOOL_PATH not found %TOOL_PATH%
	exit /B 1
)

set TOOL_BIN=%TOOL_PATH%\kernel_app_64bit.exe
if not exist %TOOL_BIN% (
	echo TOOL_BIN not found %TOOL_BIN%
	exit /B 1
)

set FRAMEWORKS_PATH=..\..\..\frameworks
if not exist %FRAMEWORKS_PATH% (
	echo FRAMEWORKS_PATH not found %FRAMEWORKS_PATH%
	exit /B 1
)

:: Generate project
echo "Generating project..."
set GENERATED_DIR="%cd%"
%TOOL_BIN% g_updateproject=%GENERATED_DIR%

echo "Generating code..."
python.exe %FRAMEWORKS_PATH%\settings\sourceprocessor\sourceprocessor.py %GENERATED_DIR%

echo "Done!"
