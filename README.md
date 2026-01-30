# Browser Chooser

[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/browserchooser)](https://github.com/Tatsh/browserchooser/tags)
[![License](https://img.shields.io/github/license/Tatsh/browserchooser)](https://github.com/Tatsh/browserchooser/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/browserchooser/v0.0.1/master)](https://github.com/Tatsh/browserchooser/compare/v0.0.1...master)
[![CodeQL](https://github.com/Tatsh/browserchooser/actions/workflows/codeql.yml/badge.svg)](https://github.com/Tatsh/browserchooser/actions/workflows/codeql.yml)
[![QA](https://github.com/Tatsh/browserchooser/actions/workflows/qa.yml/badge.svg)](https://github.com/Tatsh/browserchooser/actions/workflows/qa.yml)
[![GitHub Pages](https://github.com/Tatsh/browserchooser/actions/workflows/pages.yml/badge.svg)](https://tatsh.github.io/browserchooser/)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/browserchooser?logo=github&style=flat)](https://github.com/Tatsh/browserchooser/stargazers)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor%3Ddid%3Aplc%3Auq42idtvuccnmtl57nsucz72%26query%3D%24.followersCount%26style%3Dsocial%26logo%3Dbluesky%26label%3DFollow%2520%40Tatsh&query=%24.followersCount&style=social&logo=bluesky&label=Follow%20%40Tatsh)](https://bsky.app/profile/Tatsh.bsky.social)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)

**Pick which browser—and which profile—to use when opening a link.**

Set Browser Chooser to your default browser and when you click on URLs in non-browsers, it will
display a simple dialogue so you can choose the right browser or profile to use, and optionally
remember your choice per domain.

## Why use this?

- **Work vs personal** - Open work links in a dedicated profile and personal links in another.
- **Security and isolation** - Use a separate browser or profile for banking, social, or
  untrusted sites.
- **Multiple accounts** - Switch between Chrome/Firefox profiles (e.g. Work, Personal, Dev) from one
  place when a link is opened.

---

## Features

- **Cross-platform** - Linux (XDG desktop entries), macOS (`.app` bundles), and Windows
  (registry-based discovery).
- **Browser discovery** - Detects Chrome, Firefox, Edge, Brave, Chromium, Opera, Safari (macOS), and
  other common browsers.
- **Profile support** - Lists Chrome/Chromium and Firefox profiles by name, with profile pictures
  for Chromium-based browsers (when available).
- **Guest profiles** - Buttons to open Guest profiles in browsers; these can be hidden.
- **Remember per domain** - “Do not ask again” saves your selection for that domain so the same
  browser/profile opens next time.
- **Configurable filtering** - Hide specific browsers or turn off profile listing for chosen
  browsers via config (e.g. `Advanced/hideBrowsers`, `Advanced/hideProfileBrowsers`). On macOS you
  can use bundle IDs (e.g. `com.apple.Safari`).

---

## Installation

Not yet written. See [CONTRIBUTING](CONTRIBUTING.md) for build and development setup.

---

## Links

- [Changelog](CHANGELOG.md)
- [Contributing](CONTRIBUTING.md)
- [Security policy](SECURITY.md)
