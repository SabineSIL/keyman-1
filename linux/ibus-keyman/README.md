# Keyman engine for IBus

## Requirements

You need autoconf, autopoint, gettext, automake and libtool to generate the build system.

Run `./autogen.sh` to run them.

## Building

```bash
./autogen.sh
./configure
make
sudo make install
```

For a debug build:

```bash
./configure CPPFLAGS=-DG_MESSAGES_DEBUG CFLAGS="-g -O0" CXXFLAGS="-g -O0"
```

To use the header files from the source repo, you need to specify paths to the include files in core:

```bash
./configure CPPFLAGS="-DG_MESSAGES_DEBUG -I../../common/core/desktop/build/arch/debug/include/ -I../../common/core/desktop/include/" \
  CFLAGS="-g -O0" CXXFLAGS="-g -O0"
```
