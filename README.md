# CrankItUp

A Nuke NDK plugin that cranks up colour channels to help flatten lighting inconsistencies — especially useful for pulling difficult green/blue screen keys.

**Original author:** Gerard Benjamin Pierre
([IMDb](http://www.imdb.com/name/nm0682633/) )

**Original Nukepedia page:** [CrankItUp on Nukepedia](https://www.nukepedia.com/tools/plugins/colour/crank-it-up/)

## Description

CrankItUp provides several equation modes that manipulate RGB channels in unique ways, helping to even out uneven screen lighting before keying. It can also be used as a despill tool. Some modes produce effects similar to gamma/contrast adjustments but with a distinct character — and a few will give you some psychedelic results along the way.

## Building from Source

### Requirements

- CMake 3.16+
- Nuke NDK (matching your target Nuke version)
- C++17 compatible compiler

### Build

```bash
mkdir build && cd build
cmake .. -DNUKE_INSTALL_PATH=/path/to/Nuke17.0
cmake --build . --config Release
```

### Install

Copy the resulting `CrankItUp.so` (Linux), `CrankItUp.dylib` (macOS), or `CrankItUp.dll` (Windows) to your `~/.nuke` directory or any path on `NUKE_PATH`.

## Usage

1. Add a **CrankItUp** node after your plate (before keying).
2. Select a **mode** — each equation processes the RGB channels differently.
3. Adjust to taste — the goal is to flatten uneven screen colour so your keyer has an easier time.

## License

This project is licensed under the **BSD 3-Clause License** — see the [LICENSE](LICENSE) file for details.

Copyright (c) 2011, Gerard Benjamin Pierre. All rights reserved.

## Credits

- **Gerard Benjamin Pierre** — original author and all plugin logic.
- **Peter Mercell** — Nuke 17 compilation and distribution.
