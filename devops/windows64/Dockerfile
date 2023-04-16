# escape=`

# Use the latest Windows Server Core 2019 image.
FROM mcr.microsoft.com/windows/servercore:ltsc2019

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

RUN `
    # Download the Build Tools bootstrapper.
    curl -SL --output vs_buildtools.exe https://aka.ms/vs/17/release/vs_buildtools.exe `
    `
    # Install Build Tools with the Microsoft.VisualStudio.Workload.AzureBuildTools workload, excluding workloads and components with known issues.
    && (start /w vs_buildtools.exe --quiet --wait --norestart --nocache `
    --installPath "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools" `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.VC.ASAN `
    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    --add Microsoft.VisualStudio.Component.Windows10SDK.19041 `
    || IF "%ERRORLEVEL%"=="3010" EXIT 0) `
    `
    # Cleanup
    && del /q vs_buildtools.exe

RUN `
    # Download git
    curl -sL --output git_install.exe https://github.com/git-for-windows/git/releases/download/v2.36.1.windows.1/Git-2.36.1-64-bit.exe `
    `
    # Install silently, see https://github.com/git-for-windows/git/wiki/Silent-or-Unattended-Installation
    && (start /w git_install.exe  /VERYSILENT /NORESTART /NOCANCEL /SP- /CLOSEAPPLICATIONS /RESTARTAPPLICATIONS /COMPONENTS="icons,ext\reg\shellhere,assoc,assoc_sh")

# Enable git symlinks and long paths by default
RUN git config --global core.symlinks true
RUN git config --global core.longpaths true

# Ignore git 'unsafe directory' warnings
RUN git config --global --add safe.directory *

# Download vcpkg
ARG VCPKG_SHA=0
RUN git clone https://github.com/Microsoft/vcpkg.git

RUN git clone https://github.com/CogentEmbedded/vcpkg.git ce-vcpkg

# Install vcpkg
RUN cd .\vcpkg `
    && .\bootstrap-vcpkg.bat -disableMetrics `
    && .\vcpkg integrate install

# Install zstd, getopt
RUN cd .\vcpkg `
    && .\vcpkg install --triplet x64-windows-release zstd getopt

# Install nng
RUN cd .\vcpkg `
    && .\vcpkg install --triplet x64-windows-release --overlay-ports=../ce-vcpkg/ports/nng nng

# Install qt5-base
RUN cd .\vcpkg `
    && .\vcpkg install --triplet x64-windows-release qt5-base

# Install qt5-declarative
RUN cd .\vcpkg `
    && .\vcpkg install --triplet x64-windows-release qt5-declarative

# Install qt5-quickcontrols qt5-quickcontrols2
RUN cd .\vcpkg `
    && .\vcpkg install --triplet x64-windows-release qt5-quickcontrols qt5-quickcontrols2

# Install qt5-tools
RUN cd .\vcpkg `
    && .\vcpkg install --triplet x64-windows-release qt5-tools

ENV VCPKG_TARGET_TRIPLET x64-windows-release
ENV VC_ENV_SETUP="C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat"
RUN setx VCPKG_ROOT "%cd%\vcpkg"
RUN setx path "%path%;%VCPKG_ROOT%;%cd%\vcpkg\installed\x64-windows-release\tools\qt5-tools\bin;%cd%\vcpkg\installed\x64-windows-release\tools\qt5\bin"

# Install NSIS
RUN `
    # Download
    curl -sL --output nsis_install.exe "https://downloads.sourceforge.net/project/nsis/NSIS%203/3.08/nsis-3.08-setup.exe" `
    `
    # Install silently, see https://github.com/actions/virtual-environments/blob/main/images/win/scripts/Installers/Install-NSIS.ps1
    && (start /w nsis_install.exe /S)

ENTRYPOINT ["C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat", "&&", "powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
