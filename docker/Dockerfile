FROM ubuntu:20.04

LABEL "repository"="TBD"
LABEL "homepage"="TBD"
LABEL "maintainer"="rare.breeds"

RUN export DEBIAN_FRONTEND=noninteractive && \
apt-get update -y && \
apt-get upgrade -y && \
apt-get install -y ca-certificates && \
update-ca-certificates && \
apt-get install -y git clang curl cmake unzip jq g++ libgl1-mesa-dev libglu1-mesa-dev zip rsync g++-mingw-w64-x86-64

ENV OSXCROSS_SDK_VERSION 10.11
WORKDIR /opt
RUN export SDK_VERSION=$OSXCROSS_SDK_VERSION && \
    git clone https://github.com/tpoechtrager/osxcross.git && \
    cd osxcross && \
    DEBIAN_FRONTEND=noninteractive ./tools/get_dependencies.sh && \
    curl -L -o ./tarballs/MacOSX${OSXCROSS_SDK_VERSION}.sdk.tar.xz \
        https://github.com/apriorit/osxcross-sdks/blob/master/MacOSX${OSXCROSS_SDK_VERSION}.sdk.tar.xz?raw=true && \
    UNATTENDED=1 TARGET_DIR=/usr/local ./build.sh && \
    cd .. && \
    rm -rf osxcross

# Rack SDK
ENV RACK_SDK_VERSION 1.1.6
RUN curl -L https://vcvrack.com/downloads/Rack-SDK-${RACK_SDK_VERSION}.zip -o rack-sdk.zip && \
    unzip -o rack-sdk.zip && \
    rm rack-sdk.zip

ENV RACK_DIR /opt/Rack-SDK
COPY docker/entrypoint.sh /entrypoint.sh
COPY . /opt/plugin/

ENTRYPOINT ["/entrypoint.sh"]
