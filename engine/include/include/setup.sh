#!/usr/bin/env bash
set -Eeuo pipefail
trap 'echo "Failure at line $LINENO: $BASH_COMMAND" >&2' ERR

# Log everything for troubleshooting. The log files help determine
# which step failed during previous runs.
LOG=/tmp/setup.log
FAIL_LOG=/tmp/setup_failures.log
exec > >(tee -a "$LOG") 2>&1
set -x

export DEBIAN_FRONTEND=noninteractive

# Parse command line options
OFFLINE=false
for arg in "$@"; do
  case "$arg" in
    --offline)
      OFFLINE=true
      ;;
  esac
done

if ! apt-get update -y; then
  echo "apt-get update failed" >> "$FAIL_LOG"
fi
if ! apt-get dist-upgrade -y; then
  echo "apt-get dist-upgrade failed" >> "$FAIL_LOG"
fi

OPENMACH_REPO=${OPENMACH_REPO:-https://github.com/machkernel/openmach.git}

if $OFFLINE; then
  echo "Running in offline mode" >> "$LOG"
  if ls offline_packages/*.deb >/dev/null 2>&1; then
    dpkg -i offline_packages/*.deb || echo "dpkg install failed" >> "$FAIL_LOG"
  else
    echo "No offline packages to install" >> "$LOG"
  fi
else
  if [[ ! -d openmach ]]; then
    if git clone "$OPENMACH_REPO" openmach; then
      echo "Cloned OpenMach from $OPENMACH_REPO" >> "$LOG"
    else
      echo "Failed to clone OpenMach from $OPENMACH_REPO" >> "$FAIL_LOG"
    fi
  fi
fi

install_pkg() {
  local pkg="$1"
  echo "\n===== Installing $pkg =====" | tee -a "$LOG"

  if apt-get install -y "$pkg"; then
    echo "$pkg installed via apt" >> "$LOG"
    return 0
  fi
  echo "apt-get failed for $pkg" | tee -a "$FAIL_LOG"

  if pip install "$pkg"; then
    echo "$pkg installed via pip" >> "$LOG"
    return 0
  fi
  echo "pip failed for $pkg" | tee -a "$FAIL_LOG"

  if npm install -g "$pkg"; then
    echo "$pkg installed via npm" >> "$LOG"
    return 0
  fi
  echo "npm failed for $pkg" | tee -a "$FAIL_LOG"
  if [[ "$pkg" == "shellcheck" ]]; then
    wget -qO- https://github.com/koalaman/shellcheck/releases/download/v0.9.0/shellcheck-v0.9.0.linux.x86_64.tar.xz | tar xJ
    install -m 0755 shellcheck-v0.9.0/shellcheck /usr/local/bin/shellcheck
    rm -rf shellcheck-v0.9.0
    echo "$pkg installed from binary" >> "$LOG"
    return 0
  fi
  if [[ "$pkg" == "capnproto" ]]; then
    wget -qO- https://github.com/capnproto/capnproto/archive/refs/tags/v0.9.1.tar.gz | tar xz
    pushd capnproto-0.9.1
    ./setup-makefiles.sh
    ./configure
    make -j"$(nproc)"
    make install
    popd
    echo "$pkg built from source" >> "$LOG"
    return 0
  fi
  if [[ "$pkg" == "isabelle" ]]; then
    wget -qO- https://isabelle.in.tum.de/dist/Isabelle2023-1_linux.tar.gz | tar xz
    install -d /opt/isabelle
    mv Isabelle2023-1 /opt/isabelle
    ln -s /opt/isabelle/Isabelle2023-1/bin/isabelle /usr/local/bin/isabelle
    echo "$pkg installed from archive" >> "$LOG"
    return 0
  fi
  if [[ "$pkg" == "asda" ]]; then
    wget -qO /usr/local/bin/asda https://example.com/asda
    chmod +x /usr/local/bin/asda
    echo "$pkg installed from custom source" >> "$LOG"
    return 0
  fi
  echo "could not install $pkg" | tee -a "$FAIL_LOG" >&2
}

packages=(
  build-essential git wget curl
  clang lld llvm llvm-dev libclang-dev polly
  clang-tools clang-tidy clang-format clangd
  ccache lldb gdb bolt llvm-bolt
  cmake make ninja-build
  doxygen graphviz python3-sphinx
  shellcheck yamllint
  python3 python3-pip python3-venv python3-setuptools python3-wheel
  nodejs npm yarnpkg
  coq coqide tla4tools isabelle asda
  afl++ honggfuzz cargo-fuzz
)

for pkg in "${packages[@]}"; do
  install_pkg "$pkg"
done

# Report any failures so the user can take further action. The failure log
# persists across runs to make iterative troubleshooting easier.
if [[ -s "$FAIL_LOG" ]]; then
  echo "The following packages failed to install:" | tee -a "$LOG"
  cat "$FAIL_LOG" | tee -a "$LOG"
  echo "Attempting to automatically resolve package failures" | tee -a "$LOG"
  if apt-get -f install -y >> "$LOG" 2>&1; then
    : > "$FAIL_LOG"
    echo "Automatic repair succeeded" | tee -a "$LOG"
  else
    echo "Automatic repair failed; inspect $FAIL_LOG" | tee -a "$LOG"
  fi
else
  echo "All packages installed successfully" | tee -a "$LOG"
fi

export CC="ccache clang"
export CXX="ccache clang++"
export CLANG_TIDY=clang-tidy
export PATH="/usr/lib/ccache:$PATH"

export CFLAGS="-Wall -Wextra -Werror -O2"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-fuse-ld=lld -flto"
export LLVM_PROFILE_FILE="/tmp/profiles/default.profraw"
export CLANG_EXTRA_FLAGS="-mllvm -polly"

# Troubleshooting notes:
# - Inspect $LOG for command output from each step.
# - Any package that could not be installed is listed in $FAIL_LOG.
# - Rerun this script after addressing network or dependency issues.
