def platforms = ['ubuntu16.04']

def buildArgs = ['ubuntu16.04': [CMAKE_BUILD_TYPE:"Release", CMAKE_INSTALL_PREFIX:"/usr", BUILD_GUI:"ON", WITH_QTCHARTS:"OFF"]]

def buildDeps = []

def environment = ['ubuntu16.04': []]

buildWrapper.cmake platforms: platforms, buildDependencies: buildDeps, buildArguments: buildArgs, environment: environment, doTests: false
