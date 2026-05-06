# Browser Chooser

<!-- WISWA-GENERATED-README:START -->

[![C++](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B)](https://isocpp.org)
[![GitHub tag (with filter)](https://img.shields.io/github/v/tag/Tatsh/browserchooser)](https://github.com/Tatsh/browserchooser/tags)
[![License](https://img.shields.io/github/license/Tatsh/browserchooser)](https://github.com/Tatsh/browserchooser/blob/master/LICENSE.txt)
[![GitHub commits since latest release (by SemVer including pre-releases)](https://img.shields.io/github/commits-since/Tatsh/browserchooser/v0.0.2/master)](https://github.com/Tatsh/browserchooser/compare/v0.0.2...master)
[![Dependabot](https://img.shields.io/badge/Dependabot-enabled-blue?logo=dependabot)](https://github.com/dependabot)
[![GitHub Pages](https://github.com/Tatsh/browserchooser/actions/workflows/pages.yml/badge.svg)](https://tatsh.github.io/browserchooser/)
[![Stargazers](https://img.shields.io/github/stars/Tatsh/browserchooser?logo=github&style=flat)](https://github.com/Tatsh/browserchooser/stargazers)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit)](https://github.com/pre-commit/pre-commit)
[![CMake](https://img.shields.io/badge/CMake-6E6E6E?logo=cmake)](https://cmake.org/)
[![Prettier](https://img.shields.io/badge/Prettier-black?logo=prettier)](https://prettier.io/)
[![Tests](https://github.com/Tatsh/browserchooser/actions/workflows/tests.yml/badge.svg)](https://github.com/Tatsh/browserchooser/actions/workflows/tests.yml)
[![Coverage Status](https://coveralls.io/repos/github/Tatsh/browserchooser/badge.svg?branch=master)](https://coveralls.io/github/Tatsh/browserchooser?branch=master)

[![@Tatsh](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor=did%3Aplc%3Auq42idtvuccnmtl57nsucz72&query=%24.followersCount&label=Follow+%40Tatsh&logo=bluesky&style=social)](https://bsky.app/profile/Tatsh.bsky.social)
[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Tatsh-black?logo=buymeacoffee)](https://buymeacoffee.com/Tatsh)
[![Libera.Chat](https://img.shields.io/badge/Libera.Chat-Tatsh-black?logo=liberadotchat)](irc://irc.libera.chat/Tatsh)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/109370961877277568?domain=hostux.social&style=social)](https://hostux.social/@Tatsh)
[![Patreon](https://img.shields.io/badge/Patreon-Tatsh2-F96854?logo=patreon)](https://www.patreon.com/Tatsh2)

<!-- WISWA-GENERATED-README:STOP -->

<!-- [![Flathub link](https://flathub.org/api/badge?locale=en)]
(https://flathub.org/apps/sh.tat.browserchooser) -->

**Pick which browser—and which profile—to use when opening a link.**

![Screenshot](https://raw.githubusercontent.com/Tatsh/browserchooser/master/screenshot.png)

Set Browser Chooser to your default browser and when you click on URLs in non-browsers, it will
display a simple dialogue so you can choose the right browser or profile to use, and optionally
remember your choice per domain.

## Why use this?

- **Work vs personal profiles or browsers** - Open work links in a dedicated profile and personal
  links in another.
- **Security and isolation** - Use a separate browser or profile for banking, social, or untrusted
  sites.

## Features

- **Cross-platform** - Works on Linux (XDG desktop entries), macOS (`.app` bundles), and Windows
  (registry-based discovery).
- **Browser discovery** - Detects Chrome, Firefox, Edge, Brave, Chromium, Opera, Safari (macOS), and
  other common browsers.
- **Profile support** - Lists Chrome/Chromium and Firefox profiles by name, with profile pictures
  for Chromium-based browsers (when available).
- **Guest profiles** - Buttons to open Guest profiles in browsers.
- **Remember per domain** - 'Do not ask again' saves your selection for the domain so the same
  browser/profile opens next time.
- **Configurable filtering** - Hide specific browsers or turn off profile listing for chosen
  browsers via config (e.g. `Advanced/hideBrowsers`, `Advanced/hideProfileBrowsers`). On macOS you
  can use bundle IDs (e.g. `com.apple.Safari`).
- **Pre and post-launch commands** - Run commands before or after launching a given browser or
  profile; keys are per browser (or per browser+profile).

## Configuration

The configuration file is INI format. Location is platform-dependent:

| OS      | Location                                 |
| ------- | ---------------------------------------- |
| Linux   | `~/.config/browserchooserrc`             |
| macOS   | `~/Library/Preferences/browserchooserrc` |
| Windows | `%APPDATA%\browserchooserrc`             |

### Pre-launch and post-launch commands

You can run commands **before** and **after** starting a specific browser or browser/profile.

The value format is a JSON-encoded array of arrays of strings. Each inner array is one command:
first element is the program and the rest are arguments. Example:
`[["notify-send", "Opening browser"], ["/path/to/script.sh"]]`. Unfortunately the JSON must be
inside double quotes and escaped properly.

Linux uses the path to the `.desktop` file (e.g. `/usr/share/applications/firefox.desktop`). macOS
uses the path to the `.app` bundle (e.g. `/Applications/Firefox.app`). Windows uses the path to the
`.exe` (e.g. `C:\Program Files\Mozilla Firefox\firefox.exe`). For a profile, append a pipe (escaped
as `%7C`) and the profile name (e.g. `...\chrome.exe%7CProfile%201` where this is 'Profile 1' but
percent-encoded).

#### Example configuration (Linux)

```ini
[PreLaunchCommands]
; Default
usr\share\applications\google-chrome.desktop = "[[\"notify-send\",\"Chrome\"]]"
; Profile 1
usr\share\applications\google-chrome.desktop%7CProfile%201 = "[[\"notify-send\",\"Profile 1\"]]"

[PostLaunchCommands]
usr\share\applications\firefox.desktop = "[[\"/path/to/firefox-helper.sh\"]]"
```

#### Example configuration (macOS)

```ini
[PreLaunchCommands]
Applications\Firefox.app = "[[\"osascript\",\"-e\",\"display notification \\\"Firefox\\\" with title \\\"Browser\"\"]]"
; Profile 1
Applications\Google%20Chrome.app%7CProfile%201 = "[[\"osascript\",\"-e\",\"display notification \\\"Chrome\\\" with title \\\"Browser\\\"\"]]"
```

## Installation

Not yet written. See [CONTRIBUTING](CONTRIBUTING.md) for build and development setup.
