#!/usr/bin/env bash
set -e -o pipefail

working_dir=$(mktemp -d -p /opt underpass.XXXXX)

setup_basics() {
    apt-get -y update && \
    apt-get -y upgrade
    apt-get -y install \
        git     \
        python3 \
        wget    \
        unzip   \
        netcat  \
        postgresql-client
}

setup_dependencies() {
    apt-get -y install \
        autotools-dev         \
        build-essential       \
        gcc                   \
        pkg-config            \
        python3-dev           \
        swig                  \
        openjdk-11-jdk        \
        libboost-all-dev      \
        libopenmpi-dev        \
        libwebkit2gtk-4.0-dev \
        libxml++2.6-dev       \
        libgumbo-dev

    apt-get -y install \
       libgdal-dev     \
       libosmium2-dev  \
       librange-v3-dev
}

setup_libpqxx() {
    cd $${working_dir}
    wget https://github.com/jtv/libpqxx/archive/${libpqxx_version}.zip
    unzip ${libpqxx_version}.zip
    cd libpqxx-${libpqxx_version}

    ./configure --enable-shared && make && sudo make install
}

setup_underpass_from_source() {
    cd $${working_dir}
    git clone https://github.com/hotosm/underpass.git
    export PKG_CONFIG_PATH=$${working_dir}/underpass/m4/
    sudo ln -s /lib/x86_64-linux-gnu/pkgconfig/python-3.8.pc /lib/x86_64-linux-gnu/pkgconfig/python.pc

    cd $${working_dir}/underpass/ && \
        ./autogen.sh && \
        mkdir build && \
        cd build && \
        ../configure && \
        make -j$(($(nproc) * 2))
}

setup_underpass_from_deb() {
    if [[ -f /tmp/underpass_20210614_amd64.deb ]]; then
        dpkg -i /tmp/underpass_20210614_amd64.deb
    fi
}

teardown() {
    rm -fr $${working_dir}
}

install_cloudwatchagent() {
    _pf_arch=$(uname -m)
    if [ $_pf_arch = 'x86_64' ]; then
        _arch='amd64'
    elif [ $_pf_arch = 'aarch64' ]; then
        _arch='arm64'
    fi
    _tempfile=$(mktemp -p /opt deb.XXXXX)
    _deb_src="https://s3.amazonaws.com/amazoncloudwatch-agent/debian/$(_arch)/latest/amazon-cloudwatch-agent.deb"
    wget -O $${_tempfile} $${_deb_src} && \
    dpkg -i $${_tempfile} && \
    rm -f $${_tempfile}
}

setup_basics
#setup_dependencies
#setup_libpqxx
# setup_underpass_from_source
# setup_underpass_from_deb
install_cloudwatchagent
