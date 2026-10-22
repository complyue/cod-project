#!/usr/bin/env bash

set -e

# Xcode or Command Line Tools provide AppleClang and the macOS SDK.
xcrun --sdk macosx --show-sdk-path

# SWIG and Python development files enable LLDB's Python bindings.
brew install cmake ninja zlib swig python pkgconf
