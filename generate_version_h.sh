#!/bin/bash

VERSION_MAJOR=0
VERSION_MINOR=$(git rev-list --count HEAD 2>/dev/null)
VERSION_HASH=$(git rev-parse --short HEAD 2>/dev/null)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
OUTPUT_FILE="${SCRIPT_DIR}/version.h"

cat <<EOF > "$OUTPUT_FILE"
#ifndef VERSION_H
#define VERSION_H

#define APP_VERSION_MAJOR ${VERSION_MAJOR}
#define APP_VERSION_MINOR ${VERSION_MINOR}
#define APP_VERSION_HASH "${VERSION_HASH}"
#define APP_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}-${VERSION_HASH}"

#endif // VERSION_H
EOF