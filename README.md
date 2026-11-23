# Affinity on Linux loader

Fixes for Affinity by Canva under Wine that cannot be made in Wine itself,
because they correct Affinity's own code. They are applied in memory with
[Harmony](https://github.com/pardeike/Harmony) while Affinity starts; nothing
on disk is patched.

The [Affinity on Linux installer](https://github.com/jfacemyer/Linux-Affinity-Manager)
installs it, and its launcher, `affinity-on-linux.exe`, loads it into Affinity.
Bugs in Wine itself are fixed in the
[Affinity Wine builds](https://github.com/jfacemyer/Affinity-Wine-Builder) instead.

## What is in it

| File | What it does | License |
|---|---|---|
| `aol-bootstrap.dll` | Native DLL the launcher loads into Affinity. Starts the loader on Affinity's .NET runtime. | MIT |
| `aol-loader.dll` | Loads the plugins in `aol/plugins`, applies their patches at the right moment, and adds their settings to Affinity's Preferences. | MIT |
| `aol/plugins/aol-fixes.dll` | The fixes. See [Fixes/README.md](Fixes/README.md). | GPLv2 |
| `0Harmony.dll` | Harmony 2.4.2. | MIT |

Settings live in `aol/config/*.toml` in Affinity's folder and in Affinity's
Preferences. Any setting can be overridden for one run with an environment
variable, `AOL__<PLUGIN>__<KEY>`, for example `AOL__FIXES__PIN_PLUGIN_HOST=false`.
The log is `aol/logs/aol.latest.log`, when logging to a file is turned on.

## Building

```
./build.sh
```

Needs the .NET SDK and MinGW-w64. It writes `dist/aol-loader-<version>.tar.xz`
and prints its SHA-256. Builds are reproducible, so a release can be rebuilt
and checked against the installer's pin.

## Origin

This is [AffinityPluginLoader](https://github.com/noahc3/AffinityPluginLoader)
and its WineFix plugin by Noah Curoe, renamed, and changed for the Affinity on
Linux project:

- AffinityHook.exe is gone. Affinity on Linux's own launcher starts Affinity
  and loads the bootstrap, so there is one launcher, not two.
- The bootstrap builds with a plain MinGW-w64 and needs no import libraries.
- WineFix's Direct2D hooks are gone: the Affinity Wine builds fix those bugs in
  d2d1 itself, and the hooks found the wrong function in those builds.
- Fixes for the color picker, the Leading dropdown, the sub-tool panel and the
  plugin host, first sent upstream as pull requests.

Its git history is kept, so every upstream change keeps its author.

## Licensing

The loader and the bootstrap are under the MIT license, the fixes under GPLv2;
see the LICENSE file in each folder. Noah Curoe's original licenses also exempt
Canva and its subsidiaries, who may instead take the code he wrote under the
Zero-Clause BSD license; that exemption still applies to his code.
