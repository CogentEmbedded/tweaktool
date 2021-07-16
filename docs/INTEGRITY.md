# Build on Green Hills Integrity

## Download NNG 1.4 into source tree

Suppose, you want to install to `$(PROJECT_ROOT)/thirdparty`

NNG from its main repository won't work on integrity.
There are two problems:

 1. No random number generator
 2. ICP/IP stack on Integrity doesn't support AI_ADDRCONFIG flag in `gethostaddr` call.

There's a branch that incorporates fixes for these problems.

It has project Integrity project files for Nng library as well.

```bash
cd ./thirdparty
git clone -b sz-integrity-fixes git@github.com:CogentEmbedded/nng.git
```
## Integrity developer's environment

In this document it is assumed that tweaktool library is being built from
MULTI IDE provided with Integrity developer's toolchain and project is
stored in file `default.gpj` in the project's root directory.
After referencing tweaktool files, user should be able build project
from "Build" menu in MULTI IDE. Build script's snippets below are given
in gpj language.

## Reference NNG directory from project file

```
macro __NNG_ROOT_DIR=%expand_path(./thirdparty/nng)
```
## Reference tweak2 library from project file

```
macro __TWEAK_DIR=%expand_path(./cogent/tweaktool)
```

## Reference libraries from Green Hills project file

```
$(__NNG_ROOT_DIR)/libnng-core.gpj		    [Library]
$(__NNG_ROOT_DIR)/libnng-pair0.gpj		    [Library]
$(__NNG_ROOT_DIR)/libnng-posix.gpj		    [Library]
$(__NNG_ROOT_DIR)/libnng-supplemental.gpj	[Library]
$(__NNG_ROOT_DIR)/libnng-transport-tcp.gpj	[Library]
$(__TWEAK_DIR)/libtweak2.gpj                    [Library]
```

## Reference libraries from include and link section of an Integrity executable

```
    -I$(__TWEAK_DIR)/tweak-wire/include
    -I$(__TWEAK_DIR)/tweak-pickle/include
    -I$(__TWEAK_DIR)/tweak-common/include
    -I$(__TWEAK_DIR)/tweak-app/include
    -I$(__TWEAK_DIR)/tweak2lib/include
```

```
    -lnng-core
    -lnng-pair0
    -lnng-posix
    -lnng-supplemental
    -lnng-transport-tcp
    -ltweak2
```

## Use the library

Include header tweak2.h and call tweak functions. Reference other documents.

```c
#include <tweak2.h>

/* ... */

int main() {
    tweak_initialize_library("nng", "role=server", "tcp://0.0.0.0:7777/");
    const char* meta = "{"
        "\"control\": \"slider\","
        "\"min\": 55.6,"
        "\"max\": 100.4,"
        "\"readonly\": false,"
        "\"decimals\": 3,"
        "}";
    tweak_id input_tweak = tweak_add_scalar_float("/some/input/tweak", "float input tweak", meta, 0.f);
    tweak_id output_tweak = tweak_add_scalar_float("/some/output/tweak", "float output tweak", meta, 0.f);
    tweak_set_scalar_float(input_tweak, 42);
    float value = tweak_get_scalar_float(output_tweak);
    /* ... */
}
```
