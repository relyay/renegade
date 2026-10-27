<img alt="Renegade firmware screens: desktop with clock, About screen and first start slideshow" src="/.github/assets/banner.png">

# Renegade

Custom firmware for Flipper Zero, based on the [official firmware](https://github.com/flipperdevices/flipperzero-firmware).

What is different from upstream:

- No dolphin: no pet, levels, mood, passport, idle animations or games. The desktop shows a static wallpaper with a clock, date and SD card / battery hints.
- Own branding and redrawn assets.
- The SDK API stays compatible with the official firmware: `dolphin_*` functions are kept as no-op stubs, so apps from the official catalog keep working.
- Every push is built by GitHub Actions and published as a `dev-xxxxxx` pre-release with the update package and the list of commits.

## Installing

Download `renegade-f7-update-dev-xxxxxx.tgz` from [Releases](https://github.com/relyay/renegade/releases) and either:

- install it with qFlipper: "Install from file";
- or unpack it to `/ext/update/` on the SD card and launch `update.fuf` from the Archive.

## Building

Clone the source code with submodules:

```shell
git clone --recursive https://github.com/relyay/renegade.git
```

Build the full update package (firmware, radio stack, all SD card databases and apps) with [just](https://github.com/casey/just):

```shell
just update
```

The package is written to `dist/f7-C/renegade-f7-update-local.tgz`. Other recipes:

| Recipe              | What it does                                              |
|---------------------|-----------------------------------------------------------|
| `just update`       | Full update package, release build                        |
| `just update-debug` | Full update package with debug firmware                   |
| `just build`        | Debug firmware build                                      |
| `just apps`         | All `.fap` applications, including debug ones, in `dist/` |
| `just flash`        | Install the update package over USB                       |
| `just deploy-apps`  | Copy built `.fap` applications to the Flipper over USB    |
| `just format`       | Format sources, scripts and images                        |
| `just lint`         | Check formatting                                          |
| `just clean`        | Remove `build/` and `dist/`                               |

Set `TARGET_HW=18` to build for the f18 target and `DIST_SUFFIX` to change the version suffix:

```shell
DIST_SUFFIX=my-build just update
```

The Flipper Build Tool (`./fbt`) downloads its own toolchain on the first run and can still be used directly, see [fbt documentation](/documentation/fbt.md).

## Documentation

- [Flipper Build Tool](/documentation/fbt.md) - building, flashing, and debugging Flipper software
- [Applications](/documentation/AppsOnSDCard.md), [Application Manifest](/documentation/AppManifests.md) - developing, building, deploying, and debugging Flipper applications
- [Hardware combos and Un-bricking](/documentation/KeyCombo.md) - recovering your Flipper from the most nasty situations
- [Flipper File Formats](/documentation/file_formats) - everything about how Flipper stores your data and how you can work with it
- [Upstream developer documentation](https://developer.flipper.net/flipperzero/doxygen)

## Project structure

- `applications`        - Applications and services used in firmware
- `applications_user`   - Place for your additional applications and services
- `assets`              - Assets used by applications and services
- `documentation`       - Documentation generation system configs and input files
- `furi`                - Furi Core: OS-level primitives and helpers
- `lib`                 - Libraries, drivers, tools and etc...
- `site_scons`          - Build system configuration and modules
- `scripts`             - Supplementary scripts and various python libraries
- `targets`             - Firmware targets: platform specific code

## License

[GPL-3.0](/LICENSE), same as the upstream firmware.
