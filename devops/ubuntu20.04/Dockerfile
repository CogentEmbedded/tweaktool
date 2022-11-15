#
#  Copyright (c) 2018-2022, Cogent Embedded, Inc
#
# THIS FILE IS SUBJECT TO THE LICENSE TERMS PROVIDED IN THE 'LICENSE' FILE FOUND
# IN THE TOP-LEVEL DIRECTORY OF THIS SOFTWARE PACKAGE OR BY REQUEST VIA
# http://cogentembedded.com, source@cogentembedded.com
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install --no-install-recommends -y \
    apt-utils \
    ca-certificates \
    gpg \
    software-properties-common \
    sudo \
    wget \
    unzip

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null

RUN apt-get update && apt-get -y install \
    bash \
    build-essential \
    cmake \
    debhelper \
    debhelper-compat \
    devscripts \
    dh-python \
    doxygen \
    equivs \
    expect \
    fakeroot \
    git \
    graphviz \
    libmbedtls-dev \
    libreadline-dev \
    ninja-build \
    protobuf-compiler \
    pybind11-dev \
    python3-dev \
    python3-numpy \
    python3-protobuf \
    qml-module-qt-labs-settings \
    qml-module-qtqml-models2 \
    qml-module-qtquick-* \
    qml-module-qtquick-controls2 \
    qt5-default \
    qtdeclarative5-dev \
    qtquickcontrols2-5-dev \
    scons \
    ubuntu-dev-tools

# Build and install nng
ADD build-nng-debs.sh ./
RUN chmod +x ./build-nng-debs.sh && \
    ./build-nng-debs.sh

# Clean apt cache
RUN apt-get clean && apt-get autoclean

CMD ["/bin/bash"]
