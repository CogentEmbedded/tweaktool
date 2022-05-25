#!/usr/bin/awk -f
BEGIN {
    HAS_AT_FILE = 0
    HAS_AT_COPYRIGHT = 0
    HAS_MIT = 0
}

/Permission is hereby granted, free of charge/ { HAS_MIT = HAS_MIT + 1 }
/all copies or substantial portions of the Software/ { HAS_MIT = HAS_MIT + 1 }
/HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER/ { HAS_MIT = HAS_MIT + 1 }

tolower($0) ~ /copyright.*20/ {
    if (!match($0, YEAR)) { printf("%s: error: Invalid copyright year %d %s\n", RELATIVE_FILENAME, NR, $0) }
}

END {
    if (HAS_MIT < 3) { printf("%s: error: does not contain proper MIT license\n", RELATIVE_FILENAME) }
}
