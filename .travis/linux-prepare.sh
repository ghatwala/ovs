#!/bin/bash

set -ev

# Build and install sparse.
#
# Explicitly disable sparse support for llvm because some travis
# environments claim to have LLVM (llvm-config exists and works) but
# linking against it fails.
git clone git://git.kernel.org/pub/scm/devel/sparse/sparse.git
cd sparse
make -j4 HAVE_LLVM= install
cd ..

if [[ "$TRAVIS_ARCH" == "amd64" ]] || [[ -z "$TRAVIS_ARCH" ]]; then
    # Install dependent packages only for x86-64 architecture
    sudo apt-get install \
    -y --no-install-suggests --no-install-recommends \
    gcc-multilib libunbound-dev:i386
elif [ "$TRAVIS_ARCH" == "aarch64" ]; then
    sudo apt-get install \
    -y --no-install-suggests --no-install-recommends \
    python-pip python3-pip \
    python-setuptools python3-setuptools \
    python-dev python3-dev \
    libtool
fi

pip install --disable-pip-version-check --user six flake8 hacking
pip install --user --upgrade docutils

if [ "$TRAVIS_ARCH" == "aarch64" ]; then
    pip install --disable-pip-version-check --user pyOpenSSL
    pip3 install --disable-pip-version-check --user pyOpenSSL
fi

if [ "$M32" ]; then
    # 32-bit and 64-bit libunwind can not be installed at the same time.
    # This will remove the 64-bit libunwind and install 32-bit version.
    sudo apt-get install -y libunwind-dev:i386
fi
