# Affinity on Linux fixes

The fixes the [Affinity on Linux loader](../README.md) applies to Affinity. Each one can be turned off in Affinity's Preferences, on the Affinity on Linux fixes page, or in `aol/config/fixes.toml`.

This plugin began as WineFix by Noah Curoe.

## Fixes

- **Direct2D geometry fixes** (Bézier rendering, collinear joins, Widen, split recursion) — no longer here. The Affinity Wine builds fix them in d2d1 itself, and the in-memory hooks found the wrong function in those builds.
- **Preferences save fix** — Preferences fail to save on application exit. Fixed via Harmony transpiler replacing `HasPreviousPackageInstalled()` with `false`.
- **Color picker Wayland fix** — Color picker zoom preview displays a black image on Wayland. Fixed by patching the magnifier capture to use Wine's window capture instead of screen capture.
- **Font enumeration fix** — Intermittent startup crash from parallel font enumeration. Fixed by forcing synchronous font loading.
- **Canva sign-in helper** — Canva sign-in helper to allow copy/paste of the authorization URL to complete sign-in, no protocol handler required.
- **Command-line file opening fix** — Opening files from the desktop or command line crashes or silently fails due to a missing WinRT type. Fixed by bypassing `ProcessCommandLineArguments` and opening files directly via `IDocumentViewService`.
- **Plugin host pin** — Affinity frees its plugin host (`libplugins.dll`) during a session and later calls back into it, crashing with an access violation. Fixed by holding a reference to the module so the released code stays mapped.

## Licensing

These fixes are licensed under **GPLv2**. See the [LICENSE](LICENSE) file.

Noah Curoe's original license exempts [Canva](https://github.com/canva) and its subsidiaries, who may (at their option) instead license the source code he wrote for WineFix under the Zero-Clause BSD license.

## Credits

- [WineFix](https://github.com/noahc3/AffinityPluginLoader) by Noah Curoe
- [Arecsu/wine-affinity](https://github.com/Arecsu/wine-affinity): collinear outline join fix, Widen stub fix, bezier split recursion and budget fix
- [Harmony](https://github.com/pardeike/Harmony)
