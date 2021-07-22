# Setup your Keyman build environment on Windows

*** THIS FILE IS IN PROGRESS; see /windows/src/README.md for clearer notes.
7
## Target Projects

On Windows, you can build the following projects:

* Keyman for Android
* Keyman for Windows
* Keyman Developer (together with Keyman for Windows)
* KeymanWeb

The following libraries can also be built:

* Keyman Core (Windows, wasm targets) (aka common/core/desktop)
* Common/Web (aka common/core/web)

The following projects **cannot** be built on Windows:

* Keyman for Linux
* Keyman for macOS
* Keyman for iOS

## System Requirements

* Minimum Windows version: Windows 10 x64

## Prerequisites

Many dependencies are only required for specific projects.

Core development tools required:
* Delphi 10.3 Community
* Visual Studio 2019 Community

<!--
The remaining dependencies can be installed via script:
  `resources/devbox/windows/windows.sh`

This script will also update your environment to the values in:
  `resources/devbox/windows/keyman.windows.env.sh`

It will also add these environment settings to your local profile?
-->

These dependencies are also listed below if you'd prefer to install manually.

We prefer Chocolatey at present for installation of dependencies:

```ps1
...TODO...
```

While we use PowerShell for dependency installation, we use a mixture of bash
scripts and Borland makefiles

## Shared Dependencies

* Shared: git for Windows, jq, Python 3, Meson, Ninja, Rust, Pandoc

  ```ps1
  choco ...
  ```

* Web: node.js, emscripten, wasm-pack, openjdk 8

  ```shell
  choco ...
  rustup target add wasm32-unknown-unknown
  ```

## Android-only

* Android: Android SDK, Android Studio, Ant, Gradle, Maven

  ```shell
  choco ... install android-sdk android-studio ant gradle maven
  # update path
  #source ../resources/devbox/windows/keyman.windows.env.sh
  # optionally install sdk images
  sdkmanager "system-images;android-30;google_apis;armeabi-v7a"
  sdkmanager --update
  sdkmanager --licenses
  ```

* Run Android Studio once after installation to install additional components
  such as emulator images and SDK updates.

## Optional Dependencies

* sentry-cli (optional)
  - Uploading symbols for Sentry-based error reporting

  ```
  ...
  ```
