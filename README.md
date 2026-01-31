# Browser Chooser

[![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black)](https://www.gentoo.org/)
[![macOS](https://img.shields.io/badge/macOS-000000?logo=apple&logoColor=F0F0F0)](https://www.apple.com/macos)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white)](https://www.microsoft.com/en-us/windows)
[![CMake](https://img.shields.io/badge/CMake-6E6E6E?logo=cmake)](https://cmake.org/)
[![Qt 6.7+ supported](https://img.shields.io/badge/qt-6.7+-black.svg?logo=qt&logoColor=00fa6f)](https://doc.qt.io/)
[![C++](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B)](https://isocpp.org)
[![Prettier](https://img.shields.io/badge/Prettier-enabled-black?logo=prettier)](https://prettier.io/)
[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/browserchooser)](https://github.com/Tatsh/browserchooser/tags)
[![License](https://img.shields.io/github/license/Tatsh/browserchooser)](https://github.com/Tatsh/browserchooser/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/browserchooser/v0.0.1/master)](https://github.com/Tatsh/browserchooser/compare/v0.0.1...master)
[![CodeQL](https://github.com/Tatsh/browserchooser/actions/workflows/codeql.yml/badge.svg)](https://github.com/Tatsh/browserchooser/actions/workflows/codeql.yml)
[![QA](https://github.com/Tatsh/browserchooser/actions/workflows/qa.yml/badge.svg)](https://github.com/Tatsh/browserchooser/actions/workflows/qa.yml)
[![Dependabot](https://img.shields.io/badge/Dependabot-enabled-blue?logo=dependabot)](https://github.com/dependabot)
[![GitHub Pages](https://github.com/Tatsh/browserchooser/badge/pages)](https://Tatsh.github.io/browserchooser/)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)](https://github.com/pre-commit/pre-commit)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/browserchooser?logo=github&style=flat)](https://github.com/Tatsh/browserchooser/stargazers)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor=did%3Aplc%3Auq42idtvuccnmtl57nsucz72&query=%24.followersCount&style=social&logo=bluesky&label=Follow+%40Tatsh)](https://bsky.app/profile/Tatsh.bsky.social)
[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Tatsh-black?logo=buymeacoffee)](https://buymeacoffee.com/Tatsh)
[![Libera.Chat](https://img.shields.io/badge/Libera.Chat-Tatsh-black?logo=liberadotchat)](irc://irc.libera.chat/Tatsh)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)
[![Patreon](https://img.shields.io/badge/Patreon-Tatsh2-F96854?logo=patreon)](https://www.patreon.com/Tatsh2)

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
- **Pre- and post-launch commands** - Run commands before or after launching a given browser or
  profile; keys are per browser (or per browser+profile). See [Configuration](#configuration).

## Configuration

The config file is INI format. Location is platform-dependent (e.g. `~/.config/browserchooserrc` on
Linux, `~/Library/Preferences/browserchooserrc` on macOS, `%APPDATA%\browserchooserrc` on Windows).

### Pre-launch and post-launch commands

You can run commands **before** (pre-launch) and **after** (post-launch) starting a specific browser
or browser+profile. Pre-launch commands run one after another, synchronously; post-launch commands
run detached.

**INI sections:** `[PreLaunchCommands]` and `[PostLaunchCommands]`.

**Key format:** The key is the same as used when remembering a browser for a domain:

- **Browser only:** the path that identifies the browser (desktop path).
- **Browser + profile:** that path, then `|`, then the profile name (e.g. `Default`, `Profile 1`).

If there is no entry for browser+profile, the entry for the browser only (same path, no `|`) is
used.

**Value format:** A JSON array of arrays of strings. Each inner array is one command: first element
is the program, the rest are arguments. Example: `[["notify-send", "Opening browser"], ["/path/to/script.sh"]]`.

**Key examples by platform:** Linux uses the path to the `.desktop` file (e.g.
`/usr/share/applications/firefox.desktop`). macOS uses the path to the `.app` bundle (e.g.
`/Applications/Firefox.app`). Windows uses the path to the `.exe` (e.g.
`C:/Program Files/Mozilla Firefox/firefox.exe`). For a profile, append a pipe and the profile name
(e.g. `.../chrome.exe|Default`).

**Example config (Linux):**

```ini
[PreLaunchCommands]
/usr/share/applications/firefox.desktop=[["notify-send", "Firefox"]]
/usr/share/applications/google-chrome.desktop=[["notify-send", "Chrome"]]
/usr/share/applications/google-chrome.desktop|Default=[["notify-send", "Chrome Default"]]

[PostLaunchCommands]
/usr/share/applications/firefox.desktop=[["/path/to/firefox-helper.sh"]]
```

**Example config (macOS):**

```ini
[PreLaunchCommands]
/Applications/Firefox.app=[["osascript", "-e", "display notification \"Firefox\" with title \"Browser\""]]
/Applications/Google Chrome.app=[["osascript", "-e", "display notification \"Chrome\" with title \"Browser\""]]
```

**Example config (Windows):**

```ini
[PreLaunchCommands]
C:/Program Files/Mozilla Firefox/firefox.exe=[["notify-send", "Firefox"]]
C:/Program Files/Google/Chrome/Application/chrome.exe=[["notify-send", "Chrome"]]
```

## Installation

Not yet written. See [CONTRIBUTING](CONTRIBUTING.md) for build and development setup.

## Links

- [Changelog](CHANGELOG.md)
- [Contributing](CONTRIBUTING.md)
- [Security policy](SECURITY.md)
