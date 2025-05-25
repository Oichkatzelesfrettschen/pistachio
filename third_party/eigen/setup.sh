#!/usr/bin/env bash
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive

# Log failures but continue
FAIL_LOG=/tmp/setup_failures.log
: > "$FAIL_LOG"

apt_pin_install(){
  pkg="$1"
  ver=$(apt-cache show "$pkg" 2>/dev/null | awk '/^Version:/{print $2; exit}')
  if [ -n "$ver" ]; then
    if ! apt-get install -y "${pkg}=${ver}"; then
      echo "APT install failed: ${pkg}=${ver}" >> "$FAIL_LOG"
      return 1
    fi
  else
    if ! apt-get install -y "$pkg"; then
      echo "APT install failed: $pkg" >> "$FAIL_LOG"
      return 1
    fi
  fi
}

# pip install wrapper that logs failures but continues
pip_install(){
  for pkg in "$@"; do
    if ! pip3 install --no-cache-dir "$pkg"; then
      echo "pip install failed: $pkg" >> "$FAIL_LOG"
    fi
  done
}

install_py_pkg(){
  apt_pkg="$1"
  case "$apt_pkg" in
    python3-torch) pip_pkg="torch";;
    python3-torchvision) pip_pkg="torchvision";;
    python3-torchaudio) pip_pkg="torchaudio";;
    python3-scikit-learn) pip_pkg="scikit-learn";;
    python3-yaml) pip_pkg="PyYAML";;
    *) pip_pkg="${apt_pkg#python3-}";;
  esac
  if ! apt_pin_install "$apt_pkg"; then
    echo "Falling back to pip for $pip_pkg" >> "$FAIL_LOG"
    pip_install "$pip_pkg"
  fi
}

for arch in i386 armel armhf arm64 riscv64 powerpc ppc64el ia64; do
  dpkg --add-architecture "$arch"
done

apt-get update -y || echo "apt-get update failed" >> "$FAIL_LOG"

export CXXFLAGS="-std=c++23"

for pkg in \
  build-essential gcc g++ clang lld llvm \
  clang-17 clang-tools-17 libclang-17-dev \
  clang-format uncrustify astyle editorconfig pre-commit python3-pytest \
  make bmake ninja-build cmake meson \
  autoconf automake libtool m4 gawk flex bison byacc \
  pkg-config file ca-certificates curl git unzip \
  libopenblas-dev liblapack-dev libeigen3-dev \
  strace ltrace linux-perf systemtap systemtap-sdt-dev crash \
  valgrind kcachegrind trace-cmd kernelshark \
  libasan6 libubsan1 likwid hwloc; do
  apt_pin_install "$pkg" || true
done

for pkg in \
  python3 python3-pip python3-dev python3-venv python3-wheel; do
  apt_pin_install "$pkg" || true
done

for pkg in \
  python3-numpy python3-scipy python3-pandas \
  python3-matplotlib python3-scikit-learn \
  python3-torch python3-torchvision python3-torchaudio \
  python3-onnx python3-onnxruntime python3-yaml; do
  install_py_pkg "$pkg"
done

pip_install clang==17.*

# Ensure key Python tooling is installed via pip
pip_install pre-commit configuredb pytest PyYAML pylint pyfuzz black

# Install pre-commit hooks while network is available
if ! pre-commit install --install-hooks; then
  echo "pre-commit install failed" >> "$FAIL_LOG"
fi

pip_install tensorflow-cpu jax jaxlib \
  tensorflow-model-optimization mlflow onnxruntime-tools

for pkg in \
  qemu-user-static \
  qemu-system-x86 qemu-system-arm qemu-system-aarch64 \
  qemu-system-riscv64 qemu-system-ppc qemu-system-ppc64 qemu-system-s390x qemu-utils; do
  apt_pin_install "$pkg" || true
done

# libraries for 32-bit builds
for pkg in \
  gcc-multilib g++-multilib libc6-dev-i386 libstdc++6:i386; do
  apt_pin_install "$pkg" || true
done

for pkg in \
  bcc bin86 elks-libc \
  gcc-ia64-linux-gnu g++-ia64-linux-gnu \
  gcc-x86-64-linux-gnu g++-x86-64-linux-gnu \
  gcc-i686-linux-gnu g++-i686-linux-gnu \
  gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
  gcc-arm-linux-gnueabi g++-arm-linux-gnueabi \
  gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
  gcc-riscv64-linux-gnu g++-riscv64-linux-gnu \
  binutils-powerpc-linux-gnu gcc-powerpc-linux-gnu g++-powerpc-linux-gnu \
  gcc-powerpc64-linux-gnu g++-powerpc64-linux-gnu \
  gcc-powerpc64le-linux-gnu g++-powerpc64le-linux-gnu \
  gcc-m68k-linux-gnu g++-m68k-linux-gnu \
  gcc-hppa-linux-gnu g++-hppa-linux-gnu \
  gcc-loongarch64-linux-gnu g++-loongarch64-linux-gnu \
  gcc-s390x-linux-gnu g++-s390x-linux-gnu \
  gcc-mips-linux-gnu g++-mips-linux-gnu \
  gcc-mipsel-linux-gnu g++-mipsel-linux-gnu \
  gcc-mips64-linux-gnuabi64 g++-mips64-linux-gnuabi64 \
  gcc-mips64el-linux-gnuabi64 g++-mips64el-linux-gnuabi64; do
  apt_pin_install "$pkg" || true
done

for pkg in \
  golang-go nodejs npm typescript \
  rustc cargo clippy rustfmt \
  lua5.4 liblua5.4-dev luarocks \
  ghc cabal-install hlint stylish-haskell \
  sbcl ecl clisp cl-quicklisp slime cl-asdf \
  ldc gdc dmd-compiler dub libphobos-dev \
  chicken-bin libchicken-dev chicken-doc \
  openjdk-17-jdk maven gradle dotnet-sdk-8 mono-complete \
  swift swift-lldb swiftpm kotlin gradle-plugin-kotlin \
  ruby ruby-dev gem bundler php-cli php-dev composer phpunit \
  r-base r-base-dev dart flutter gnat gprbuild gfortran gnucobol \
  fpc lazarus zig nim nimble crystal shards gforth; do
  apt_pin_install "$pkg" || true
done

for pkg in \
  libqt5-dev qtcreator libqt6-dev \
  libgtk1.2-dev libgtk2.0-dev libgtk-3-dev libgtk-4-dev \
  libfltk1.3-dev xorg-dev libx11-dev libxext-dev \
  libmotif-dev openmotif cde \
  xfce4-dev-tools libxfce4ui-2-dev lxde-core lxqt-dev-tools \
  libefl-dev libeina-dev \
  libwxgtk3.0-dev libwxgtk3.0-gtk3-dev \
  libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
  libglfw3-dev libglew-dev; do
  apt_pin_install "$pkg" || true
done

for pkg in \
  docker.io podman buildah virt-manager libvirt-daemon-system qemu-kvm \
  gdb lldb perf gcovr lcov bcc-tools bpftrace \
  afl afl++ honggfuzz \
  openmpi-bin libopenmpi-dev mpich; do
  apt_pin_install "$pkg" || true
done

IA16_VER=$(curl -fsSL https://api.github.com/repos/tkchia/gcc-ia16/releases/latest | awk -F\" '/tag_name/{print $4; exit}')
if [ -n "$IA16_VER" ]; then
  if ! curl -fsSL "https://github.com/tkchia/gcc-ia16/releases/download/${IA16_VER}/ia16-elf-gcc-linux64.tar.xz" | tar -Jx -C /opt; then
    echo "IA16 toolchain install failed" >> "$FAIL_LOG"
  else
    echo 'export PATH=/opt/ia16-elf-gcc/bin:$PATH' > /etc/profile.d/ia16.sh
    export PATH=/opt/ia16-elf-gcc/bin:$PATH
  fi
else
  echo "Failed to fetch IA16 toolchain version" >> "$FAIL_LOG"
fi

PROTO_VERSION=25.1
if curl -fsSL "https://raw.githubusercontent.com/protocolbuffers/protobuf/v${PROTO_VERSION}/protoc-${PROTO_VERSION}-linux-x86_64.zip" -o /tmp/protoc.zip; then
  unzip -d /usr/local /tmp/protoc.zip && rm /tmp/protoc.zip || echo "protoc install failed" >> "$FAIL_LOG"
else
  echo "Failed to download protoc" >> "$FAIL_LOG"
fi

command -v gmake >/dev/null 2>&1 || ln -s "$(command -v make)" /usr/local/bin/gmake
command -v clang-tidy >/dev/null 2>&1 || ln -s "$(command -v clang-tidy-17)" /usr/local/bin/clang-tidy
command -v clang-format >/dev/null 2>&1 || ln -s "$(command -v clang-format-17)" /usr/local/bin/clang-format

# Verify key Python tooling availability
for tool in pre-commit configuredb pytest pyyaml pylint pyfuzz black; do
  if ! "$tool" --version >/dev/null 2>&1; then
    echo "$tool --version failed" >> "$FAIL_LOG"
  fi
done

apt-get clean || echo "apt-get clean failed" >> "$FAIL_LOG"
rm -rf /var/lib/apt/lists/*

exit 0
