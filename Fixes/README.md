# Affinity on Linux fixes

The fixes the [Affinity on Linux loader](../README.md) applies to Affinity. Each one can be turned off in Affinity's Preferences, on the Affinity on Linux fixes page, or in `aol/config/fixes.toml`.

This plugin began as WineFix by Noah Curoe.

## Fixes

- **Color picker under Wayland**: the picker's zoom preview showed a black image. The magnifier captures through Wine's window capture instead of the screen. Also: the pick ends when Affinity loses focus, the capture is taken again once the overlay is hidden, and a timer that outlived its window is stopped.
- **Empty Leading dropdown** (Affinity 3.3): the text context toolbar's Leading list opened with nothing in it. Its items get the standard dropdown item style.
- **Sub-tool panel columns**: after a studio switch the shape tools came back as one column running the height of the window. The panel's own column rule is put back.
- **Tab into number fields**: Tab and Shift-Tab into a field such as the Transform panel's X or W gave it a focus ring but left the value unselected. The field starts editing, so its value comes up selected, as on a click.
- **Plugin host pin**: Affinity frees its plugin host (`libplugins.dll`) during a session and later calls back into it, crashing with an access violation. A reference to the module keeps the released code mapped.
- **Preferences saved on exit**: on older Affinity 3 releases, a package check threw under Wine and stopped Affinity saving its preferences. The check is answered with "no previous package". Affinity 3.3.3 no longer makes the call, and the fix then changes nothing. Turn it off on Affinity 3.2.3, where it opens a WebView2 page that crashes.

## Removed

WineFix fixes that the Affinity Wine builds make unnecessary. Each was checked on Wine 11.22 with Affinity 3.3.3 before it went:

- **Direct2D geometry** (Bézier rendering, collinear joins, Widen, split recursion): fixed in Wine's d2d1 itself. The in-memory hooks found the wrong function in those builds, and one of them hung Affinity.
- **Command-line and file-manager opening**: Wine provides the WinRT types Affinity needs. Without the fix, a document still opens on a cold start, and a second one is handed to the running Affinity.
- **Synchronous font enumeration**: Wine's dwrite no longer races. Six cold starts with parallel enumeration gave no crash, and startup took the same time.
- **Canva sign-in helper**: the installer registers the `affinity://` handler that sign-in uses, and on Affinity 3.3.3 the helper found nothing to patch.

## Licensing

These fixes are licensed under **GPLv2**. See the [LICENSE](LICENSE) file.

Noah Curoe's original license exempts [Canva](https://github.com/canva) and its subsidiaries, who may (at their option) instead license the source code he wrote for WineFix under the Zero-Clause BSD license.

## Credits

- [WineFix](https://github.com/noahc3/AffinityPluginLoader) by Noah Curoe
- [Arecsu/wine-affinity](https://github.com/Arecsu/wine-affinity): collinear outline join fix, Widen stub fix, bezier split recursion and budget fix
- [Harmony](https://github.com/pardeike/Harmony)
