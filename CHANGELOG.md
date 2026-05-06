<!-- markdownlint-configure-file {"MD024": { "siblings_only": true } } -->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased]

### Fixed

- MSVC Windows installs now bundle their runtime dependencies (Qt6, MSVC redistributable, and other
  required DLLs) into the install tree, matching the behaviour previously available only for MinGW
  builds.

## [0.0.1] - 2026-01-31

First version.

[unreleased]: https://github.com/Tatsh/browserchooser/compare/v0.0.1...HEAD
[0.0.1]: https://github.com/Tatsh/browserchooser/releases/tag/v0.0.1
