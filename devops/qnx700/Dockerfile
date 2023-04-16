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

FROM cerroy/qnx700:1.0

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

RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ bionic main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null

RUN apt-get update && apt-get -y install \
    bash \
    build-essential \
    cmake \
    doxygen \
    expect \
    git \
    graphviz \
    protobuf-compiler

# Clean apt cache
RUN apt-get clean && apt-get autoclean

# Hack to give all users access to QNX SDP
RUN find /opt/qnx700 -type d | xargs chmod 775

# Initialize QNX SDP in every shell
ENV QNX_BASE="/opt/qnx700"
ENV PATH="${QNX_BASE}/host/linux/x86_64/usr/bin:/root/.qnx/bin:${QNX_BASE}/jre/bin:${PATH}"
ENV QNX_CONFIGURATION="/root/.qnx"
ENV QNX_HOST="${QNX_BASE}/host/linux/x86_64"
ENV QNX_TARGET="${QNX_BASE}/target/qnx7"
ENV MAKEFLAGS="-I${QNX_BASE}/target/qnx7/usr/include"

CMD ["/bin/bash"]
