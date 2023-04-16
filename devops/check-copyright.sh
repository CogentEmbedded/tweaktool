#!/bin/bash -e

test "${BASH_SOURCE[0]}" = "" && echo "This script only can be run from bash" && return
SCRIPT_SOURCE=$(readlink -f "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(dirname "${SCRIPT_SOURCE}")

function check_file() {
    SUFFIX=$1
    FULL_FILENAME=$2

    FILENAME=$(realpath --relative-to="$PWD" "$FULL_FILENAME")
    (
        # Subshell for cd command
        cd "${SCRIPT_DIR}" || exit
        awk -f "./check-copyright-${SUFFIX}.awk" \
            -v RELATIVE_FILENAME="$FILENAME" \
            -v SHORT_FILENAME="$(basename "$FULL_FILENAME")" \
            -v YEAR="$(date +%Y)" \
            "$FULL_FILENAME"
    )
}

folders=(
    cmake
    devops
    examples
    packaging
    tweak1lib
    tweak2lib
    tweak-app
    tweak-app-cl
    tweak-common
    tweak-gui-qml
    tweak-gw
    tweak-json
    tweak-metadata
    tweak-mock-server
    tweak-pickle
    tweak-py
    tweak-wire
)

for folder in "${folders[@]}"; do
    find "$PWD/$folder" \
        \( -name '*.c' -or -name '*.h' -or -name '*.hpp' -or -name '*.cpp' -or -name '*.qml' \) \
        \! -type l \
        -print |
        grep -v 'tweak-pickle/src/autogen/' |
        while IFS= read -r filename; do
            check_file c "$filename"
        done
done

for folder in "${folders[@]}"; do
    find "$PWD/$folder" \
        \( -name 'CMakeLists.txt' -or -name '*.cmake' -or -name '*.exp' \) \
        \! -type l \
        -print |
        grep -v 'tweak-pickle/src/autogen/' |
        while IFS= read -r filename; do
            check_file cmake "$filename"
        done
done

for folder in "${folders[@]}"; do
    find "$PWD/$folder" \
        \( -name '*.py' \) \
        \! -type l \
        -print |
        grep -v 'tweak-pickle/src/autogen/' |
        while IFS= read -r filename; do
            check_file py "$filename"
        done
done
