#!/usr/bin/awk -f

@include "check-copyright-base.awk"

BEGIN {
    HAS_AT_FILE = 0
    HAS_AT_COPYRIGHT = 0
    HAS_MIT = 0
}

/@file|\\file/ { HAS_AT_FILE = 1 }

/@copyright|\\copyright/ { HAS_AT_COPYRIGHT = 1 }

/@file/ {
    if (index(SHORT_FILENAME, $NF) == 0) { printf("%s: error: wrong file Doxygen command: %d %s\n", RELATIVE_FILENAME, NR, $0) }
}

END {
    if (!HAS_AT_FILE) { printf("%s: error: file does not contain @file Doxygen command\n", RELATIVE_FILENAME) }

    if (!HAS_AT_COPYRIGHT) { printf("%s: error: file does not contain @copyright Doxygen command\n", RELATIVE_FILENAME) }
}
