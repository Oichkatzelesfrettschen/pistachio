#!/usr/bin/env bash
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive

# Log file for any failures during setup
FAIL_LOG="/tmp/setup_failures.log"
echo "Recording install failures to $FAIL_LOG"
: > "$FAIL_LOG"

#- helper to pin to the repo's exact version if it exists
apt_pin_install(){
  pkg="$1"
  ver=$(apt-cache show "$pkg" 2>/dev/null \
        | awk '/^Version:/{print $2; exit}')
  if [ -n "$ver" ]; then
    if ! apt-get install -y "${pkg}=${ver}"; then
      echo "APT install failed for $pkg" | tee -a "$FAIL_LOG"
      _pip_pkg=${pkg#python3-}
      if [[ "$pkg" == python3-* && "$_pip_pkg" != "$pkg" ]]; then
        pip3 install --no-cache-dir "$_pip_pkg" || echo "pip fallback failed for $_pip_pkg" | tee -a "$FAIL_LOG"
      fi
      return 0
    fi
  else
    if ! apt-get install -y "$pkg"; then
      echo "APT install failed for $pkg" | tee -a "$FAIL_LOG"
      _pip_pkg=${pkg#python3-}
      if [[ "$pkg" == python3-* && "$_pip_pkg" != "$pkg" ]]; then
        pip3 install --no-cache-dir "$_pip_pkg" || echo "pip fallback failed for $_pip_pkg" | tee -a "$FAIL_LOG"
      fi
      return 0
    fi
  fi
  return 0
}

#- enable foreign architectures for cross-compilation
for arch in i386 armel armhf arm64 riscv64 powerpc ppc64el ia64; do
  dpkg --add-architecture "$arch"
done

if curl -fsSL http://deb.debian.org/ >/dev/null; then
  echo "Network detected, performing package installation"
  HAVE_NET=1
else
  echo "WARNING: no network connectivity, skipping package installation" >&2
  HAVE_NET=0
fi

if [ "$HAVE_NET" -eq 1 ]; then
  apt-get update -y || echo "apt-get update failed" | tee -a "$FAIL_LOG"

#- core build tools, formatters, analysis, science libs
for pkg in \
  build-essential gcc g++ clang lld llvm \
  clang-format clang-tidy uncrustify astyle editorconfig pre-commit \
  make bmake ninja-build cmake meson \
  autoconf automake libtool m4 gawk flex bison byacc \
  pkg-config file ca-certificates curl git unzip \
  libopenblas-dev liblapack-dev libeigen3-dev \
  strace ltrace linux-perf systemtap systemtap-sdt-dev crash \
  valgrind kcachegrind trace-cmd kernelshark \
  libasan6 libubsan1 likwid hwloc; do
  apt_pin_install "$pkg"
done

#- Python & deep-learning / MLOps
for pkg in \
  python3 python3-pip python3-dev python3-venv python3-wheel \
  python3-numpy python3-scipy python3-pandas \
  python3-matplotlib python3-scikit-learn \
  python3-torch python3-torchvision python3-torchaudio \
  python3-onnx python3-onnxruntime; do
  apt_pin_install "$pkg"
done

# Install ML-related Python packages individually so failures don't stop the script
for pip_pkg in \
  tensorflow-cpu jax jaxlib \
  tensorflow-model-optimization mlflow onnxruntime-tools; do
  pip3 install --no-cache-dir "$pip_pkg" || echo "pip install $pip_pkg failed" | tee -a "$FAIL_LOG"
done

#- QEMU emulation for foreign binaries
for pkg in \
  qemu-user-static \
  qemu-system-x86 qemu-system-arm qemu-system-aarch64 \
  qemu-system-riscv64 qemu-system-ppc qemu-system-ppc64 qemu-utils; do
  apt_pin_install "$pkg"
done

#- multi-arch cross-compilers
for pkg in \
  bcc bin86 elks-libc \
  gcc-ia64-linux-gnu g++-ia64-linux-gnu \
  gcc-i686-linux-gnu g++-i686-linux-gnu \
  gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
  gcc-arm-linux-gnueabi g++-arm-linux-gnueabi \
  gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
  gcc-riscv64-linux-gnu g++-riscv64-linux-gnu \
  gcc-powerpc-linux-gnu g++-powerpc-linux-gnu \
  gcc-powerpc64-linux-gnu g++-powerpc64-linux-gnu \
  gcc-powerpc64le-linux-gnu g++-powerpc64le-linux-gnu \
  gcc-m68k-linux-gnu g++-m68k-linux-gnu \
  gcc-hppa-linux-gnu g++-hppa-linux-gnu \
  gcc-loongarch64-linux-gnu g++-loongarch64-linux-gnu \
  gcc-mips-linux-gnu g++-mips-linux-gnu \
  gcc-mipsel-linux-gnu g++-mipsel-linux-gnu \
  gcc-mips64-linux-gnuabi64 g++-mips64-linux-gnuabi64 \
  gcc-mips64el-linux-gnuabi64 g++-mips64el-linux-gnuabi64; do
  apt_pin_install "$pkg"
done

#- high-level language runtimes and tools
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
  apt_pin_install "$pkg"
done

#- GUI & desktop-dev frameworks
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
  apt_pin_install "$pkg"
done

#- containers, virtualization, HPC, debug
for pkg in \
  docker.io podman buildah virt-manager libvirt-daemon-system qemu-kvm \
  gdb lldb perf gcovr lcov bcc-tools bpftrace \
  openmpi-bin libopenmpi-dev mpich; do
  apt_pin_install "$pkg"
done

#- IA-16 (8086/286) cross-compiler
IA16_VER=$(curl -fsSL https://api.github.com/repos/tkchia/gcc-ia16/releases/latest \
           | awk -F\" '/tag_name/{print $4; exit}')
if ! curl -fsSL "https://github.com/tkchia/gcc-ia16/releases/download/${IA16_VER}/ia16-elf-gcc-linux64.tar.xz" \
  | tar -Jx -C /opt; then
  echo "gcc-ia16 download failed" | tee -a "$FAIL_LOG"
fi
echo 'export PATH=/opt/ia16-elf-gcc/bin:$PATH' > /etc/profile.d/ia16.sh
export PATH=/opt/ia16-elf-gcc/bin:$PATH

#- protoc installer (pinned)
PROTO_VERSION=25.1
if ! curl -fsSL "https://raw.githubusercontent.com/protocolbuffers/protobuf/v${PROTO_VERSION}/protoc-${PROTO_VERSION}-linux-x86_64.zip" \
  -o /tmp/protoc.zip || ! unzip -d /usr/local /tmp/protoc.zip; then
  echo "protoc install failed" | tee -a "$FAIL_LOG"
fi
  rm -f /tmp/protoc.zip

  # Ensure pre-commit and its hook environments are installed while network is available
  pip3 install --no-cache-dir -U pre-commit || echo "pip install pre-commit failed" | tee -a "$FAIL_LOG"
  if [ -f .pre-commit-config.yaml ]; then
    pre-commit install --install-hooks || echo "pre-commit hook install failed" | tee -a "$FAIL_LOG"
  fi

fi

# Ensure critical Python tooling is present even if package installs were skipped
for pip_pkg in pytest pre-commit; do
  pip3 install --no-cache-dir -U "$pip_pkg" || echo "pip install $pip_pkg failed" | tee -a "$FAIL_LOG"
done

# Create a minimal pre-commit configuration if one does not already exist
if [ ! -f .pre-commit-config.yaml ]; then
cat > .pre-commit-config.yaml <<'EOF'
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.5.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
EOF
fi

#- gmake alias
command -v gmake >/dev/null 2>&1 || ln -s "$(command -v make)" /usr/local/bin/gmake

#- clean up
apt-get clean
rm -rf /var/lib/apt/lists/*

exit 0
