# Offline Package Cache

This directory can hold `.deb` packages for environments without
network access.  When running `setup.sh --offline` the script installs
all packages located here using `dpkg -i`.

To gather the packages on a machine with network access you can use
`apt-get download`::

    $ apt-get download <package-name>

Copy the resulting files into this folder along with any dependency
packages.  The setup script does not resolve dependencies in offline
mode so ensure all required `.deb` files are present.
