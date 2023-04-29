#!/bin/bash

# Generate Ornatrix C4D SDK Example files

# Configure
export TOOL_PATH="../../../../../External/Maxon/Cinema4D/ProjectTool/cinema4d_r21_project_tool_20190903"
if [ "$TOOL_PATH" == "" ]; then
	echo "TOOL_PATH not set"
	exit 1
fi
if [ ! -d "$TOOL_PATH" ]; then
	echo "TOOL_PATH not found"
	exit 1
fi

export TOOL_BIN="$TOOL_PATH/kernel_app.app/Contents/MacOS/kernel_app"
if [ ! -f "$TOOL_BIN" ]; then
	echo "TOOL_BIN not found"
	exit 1
fi

# Generate project
export FRAMEWORKS_PATH="../../../frameworks"
export GENERATED_DIR="$PWD"

echo "Generating project..."
"$TOOL_BIN" g_updateproject="$GENERATED_DIR"

echo "Generating code..."
python "$FRAMEWORKS_PATH/settings/sourceprocessor/sourceprocessor.py" "$GENERATED_DIR"

echo "Done!"
