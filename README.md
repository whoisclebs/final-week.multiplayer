<a id="readme-top"></a>

<p align="center">
  <b>English</b> · <a href="docs/README.pt-BR.md">Português</a> · <a href="docs/README.es.md">Español</a>
</p>

[![Build][build-shield]][build-url]
[![Release][release-shield]][release-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]

<br />
<div align="center">
  <h3 align="center">Final Week Multiplayer</h3>

  <p align="center">
    Raise the player limit of Finals Week co-op rooms. One click to install, one click to remove.
    <br />
    <a href="https://github.com/whoisclebs/final-week.multiplayer/releases/latest"><strong>Download the latest release »</strong></a>
    <br />
    <br />
    <a href="https://github.com/whoisclebs/final-week.multiplayer/issues/new?labels=bug">Report Bug</a>
    &middot;
    <a href="https://github.com/whoisclebs/final-week.multiplayer/issues/new?labels=enhancement">Request Feature</a>
  </p>
</div>

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#how-it-works">How It Works</a></li>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#development">Development</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>

## About The Project

Finals Week caps multiplayer rooms at **4 players**. The cap is a single constant passed to `CreateAdvancedSession` in the game's `BP_GameInstance` blueprint. This project changes that constant and nothing else.

It ships three things:

* **`fwm-installer`**: a small command-line tool that builds the mod from *your own* game files and installs it. It keeps working after game updates as long as that session call keeps its shape.
* **A prebuilt `_P.pak`** with an 8-player limit, attached to every release, for those who prefer to copy a file by hand.
* **An Unreal Engine 5 C++ plugin** ([`plugin/`](plugin/FinalWeekMultiplayer/README.md)) with a reusable session subsystem for UE projects. The shipped game cannot load C++ plugins, so it is a developer resource, not part of the mod.

The game files are never modified. Unreal Engine loads `_P.pak` files on top of the game's own pak, so deleting the mod file restores the original game.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### How It Works

1. The installer finds the game through your Steam libraries.
2. It reads `BP_GameInstance` from `CopyGirls-Windows.pak`. That pak is Oodle-compressed; the installer decompresses it with [oozextract](https://github.com/lvlvllvlvllvlvl/oozextract), an open-source decompressor, so it never connects to the internet.
3. It searches the cooked bytecode for the `CreateAdvancedSession(PublicConnections, PrivateConnections = 0, …)` call and replaces `PublicConnections`. The call must be found **exactly once**; otherwise it stops without writing anything, so an unknown game build is never patched blindly.
4. It writes `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` to `CopyGirls/Content/Paks`. The file is written to a temporary path first and renamed only when complete.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

* [![Rust][rust-shield]][rust-url]
* [![Unreal Engine][unreal-shield]][unreal-url]
* [repak](https://github.com/trumank/repak) for reading and writing Unreal pak files
* [oozextract](https://github.com/lvlvllvlvllvlvl/oozextract) for Oodle decompression
* [clap](https://github.com/clap-rs/clap) for the command line

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Getting Started

### Prerequisites

* Finals Week installed through Steam, on Windows or on Linux / Steam Deck (Proton).
* Only the **host** needs the mod. Everyone having it is recommended.

### Installation

#### Option A: installer (recommended)

1. Download `fwm-installer.exe` (Windows) or `fwm-installer-linux` from the [latest release][release-url].
2. Close the game.
3. Double-click `fwm-installer.exe`. It installs the mod with an 8-player limit. If Steam is in a custom folder, put the installer in the game folder (the one with `CopyGirls.exe`) first.

For another limit, run it from a terminal:

```sh
fwm-installer install --players 12
```

#### Option B: prebuilt pak

1. In Steam, right-click **Finals Week** → **Manage** → **Browse local files**.
2. Open `CopyGirls\Content\Paks`.
3. Copy `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` from the release into that folder. Do not replace or delete anything.
4. Check that the file was not tampered with by comparing its hash with `SHA256SUMS.txt` from the release:

   ```powershell
   Get-FileHash .\CopyGirls-Windows_FinalWeekMultiplayer_P.pak
   ```

> [!IMPORTANT]
> The prebuilt pak contains the game asset of the build it was made from. After a game update, use the installer or wait for a new release.

#### Uninstall

Run `fwm-installer uninstall`, or delete `CopyGirls-Windows_FinalWeekMultiplayer_P.pak` from the `Paks` folder.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Usage

```text
fwm-installer [COMMAND]
```

| Command | Description |
|---|---|
| *(none)* | Install with the default settings and wait for Enter (double-click) |
| `install [--players N] [--game-dir DIR]` | Install the mod; run again to change the limit |
| `uninstall [--game-dir DIR]` | Remove the mod |
| `pack [--from PAK] [--output FILE] [--players N]` | Build the mod pak without installing it |

* `--players` accepts **2 to 64** (default **8**).
* `--game-dir` is the folder that contains `CopyGirls.exe`. Without it, the installer looks in its own folder first, then in the Steam libraries.

### Known limitations

* Tested by the community with up to 4 players. With more, exam rooms may not have enough seats, and names on boards and lists may be cut off.
* Another `_P.pak` mod that also replaces `BP_GameInstance` (for example an older "more players" mod) conflicts with this one. The installer warns when it finds other `_P.pak` files.
* If a game update changes the session call, the installer refuses to install. Remove the mod and wait for an update.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Development

```text
.
├── .github/workflows/   CI: lint, test, cross-build and release (Linux runners)
├── assets/              Original BP_GameInstance from the supported game build
├── docs/                Translated READMEs
├── installer/           Rust installer and packer
│   ├── oodle-shim/      Offline Oodle decompressor plugged into repak
│   └── src/
│       ├── main.rs      Command line
│       ├── game.rs      Steam and game folder discovery
│       ├── pak.rs       Pak reading and writing
│       └── patch.rs     Bytecode patch
└── plugin/              Unreal Engine 5 C++ plugin
```

Build and test the installer:

```sh
cd installer
cargo test
cargo build --release
```

Every push and pull request runs the [Build workflow][build-url] on Ubuntu: `cargo fmt`, `clippy`, tests, Linux and Windows builds (cross-compiled with MinGW) and the 8-player pak packed from `assets/`. Pushing a `v*` tag publishes a GitHub release with the installers, the pak, the plugin source and `SHA256SUMS.txt`.

After a game update, refresh the committed asset from your installed game. Use `--players 4`, the game's own value, so the asset stays unmodified:

```sh
fwm-installer pack --players 4 --output assets/BP_GameInstance.pak
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Roadmap

- [x] Installer with a configurable player limit
- [x] Prebuilt pak built by CI
- [x] Unreal Engine 5 session plugin
- [x] Offline installer, no downloads
- [ ] Confirm seats and UI with more than 4 players

See the [open issues][issues-url] for the full list of proposed features and known issues.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Contributing

Contributions are welcome.

1. Fork the project
2. Create your feature branch (`git checkout -b feat/amazing-feature`)
3. Commit using [Conventional Commits](https://www.conventionalcommits.org/) (`git commit -m 'feat: add amazing feature'`)
4. Push the branch (`git push origin feat/amazing-feature`)
5. Open a pull request

Code, comments and commit messages are written in English.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## License

Distributed under the MIT License. See [`LICENSE`](LICENSE) for more information.

This is an unofficial fan project, not made or supported by the game's developers; please do not report bugs to them while using it. `assets/` contains an unmodified game asset (Steam build 26650291), kept only so CI can build the pak; all rights to it belong to its owners.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Contact

whoisclebs: [github.com/whoisclebs](https://github.com/whoisclebs)

Project link: [github.com/whoisclebs/final-week.multiplayer](https://github.com/whoisclebs/final-week.multiplayer)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Acknowledgments

* [rafaalmeida1/Final-Week-Game---More-Players](https://github.com/rafaalmeida1/Final-Week-Game---More-Players), the original mod that found the player limit in `BP_GameInstance`
* [trumank/repak](https://github.com/trumank/repak)
* [Best-README-Template](https://github.com/othneildrew/Best-README-Template)
* [Shields.io](https://shields.io)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

[build-shield]: https://img.shields.io/github/actions/workflow/status/whoisclebs/final-week.multiplayer/build.yml?branch=main&style=for-the-badge
[build-url]: https://github.com/whoisclebs/final-week.multiplayer/actions/workflows/build.yml
[release-shield]: https://img.shields.io/github/v/release/whoisclebs/final-week.multiplayer?style=for-the-badge
[release-url]: https://github.com/whoisclebs/final-week.multiplayer/releases/latest
[issues-shield]: https://img.shields.io/github/issues/whoisclebs/final-week.multiplayer.svg?style=for-the-badge
[issues-url]: https://github.com/whoisclebs/final-week.multiplayer/issues
[license-shield]: https://img.shields.io/github/license/whoisclebs/final-week.multiplayer.svg?style=for-the-badge
[license-url]: https://github.com/whoisclebs/final-week.multiplayer/blob/main/LICENSE
[rust-shield]: https://img.shields.io/badge/Rust-000000?style=for-the-badge&logo=rust&logoColor=white
[rust-url]: https://www.rust-lang.org/
[unreal-shield]: https://img.shields.io/badge/Unreal%20Engine%205-0E1128?style=for-the-badge&logo=unrealengine&logoColor=white
[unreal-url]: https://www.unrealengine.com/
