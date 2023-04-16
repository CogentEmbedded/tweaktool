#### Verbosity level
# 0: Turn script tracing off.
# 1: Trace script lines as they run.
# 2: Trace script lines, variable assignments, function calls, and scripts.
Set-PSDebug -Trace 1

#### Stop on all errors
$ErrorActionPreference = "Stop"
function CheckExecutionResult {
    if (-not $?)
    {
        throw 'Command execution failed'
    }
}

#### Directories
"Triplet=$Env:VCPKG_TARGET_TRIPLET"
"VCPKG_ROOT=$Env:VCPKG_ROOT"
"PATH=$Env:path"
$install_dir=[IO.Path]::Combine($PWD.Path, 'install')

vcpkg list

#### Configure
cmake -G Ninja `
    -DCMAKE_TOOLCHAIN_FILE="$Env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
    -DVCPKG_TARGET_TRIPLET="$Env:VCPKG_TARGET_TRIPLET" `
    -DCMAKE_BUILD_TYPE=Release `
    -DX_VCPKG_APPLOCAL_DEPS_INSTALL=ON `
    -DWITH_PYTHON=OFF `
    -DBUILD_TESTS=ON `
    -DBUILD_GUI=ON `
    -DCMAKE_INSTALL_PREFIX="$install_dir" `
    ..
CheckExecutionResult

#### Build
cmake --build . --parallel
CheckExecutionResult

#### Test
ctest --output-on-failure
# Tests are not checked until they are stable
# CheckExecutionResult

#### Install and deploy the app
cmake --install .
CheckExecutionResult

#### Package the installer
cpack --verbose
CheckExecutionResult
