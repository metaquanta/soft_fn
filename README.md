# soft-fn

**soft-fn** reproduces the behavior of the Chromebook keyboard's **Search** key in linux. It uses an `evdev` `uinput` device, such that it works in Xorg, Wayland, and virtual consoles.

## Functionality

When certain keys are pressed in combination with **Search**/🔍︎ (located where the caps lock is usually found) different events are produced. This is similar to how the **fn** key behaves on many laptops (no event is produced for the **Search** key itself). However, unlike **fn**, **Search**/🔍︎ functions like an ordinary accelerator key when used in combination with keys _not_ defined below. **Search**/🔍︎ also generates an event when tapped.

Additionally, **caps lock** may be activated by **alt** + 🔍︎.

This emulates the behavior seen in ChromeOS while also allowing the **Search**/🔍︎ key to be used for user-defined shortcuts with the other keys.


## Key Map

**Search**/🔍︎ produces a `KEY_LEFTMETA` code when tapped or held in combination with a key not defined below. This is the code for the key variously known as **super**, **command**/⌘, or the "windows" key.

The codes below are defined in [linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)


| Key | code        | 🔍︎+key code |
|-----|-------------|-------------|
| ⌫   | `BACKSPACE` | `DELETE`    |
| ←   | `LEFT`      | `HOME`      |
| →   | `RIGHT`     | `END`       |
| ↑   | `UP`        | `PAGEUP`    |
| ↓   | `DOWN`      | `PAGEDOWN`  |
 
Function/media keys and power are passed through unchanged.

For chromebooks with the top row keys defined in /sys/devices/platform/i8042/serio0/function_row_physmap
replace those with F-keys by passing the hex keycodes with `--function_row='xx xx xx xx xx xx xx xx xx xx xx xx xx'`
Holding **Search**/🔍︎ while pressing
one of those keys suppresses `KEY_LEFTMETA` and passes the original media-key
code through. 

The scan codes in function_row_physmap are translated by `toprow.c`. (See `launch.sh`)

## Installation

For AMD64 Debian/Ubuntu/Gallium/etc, get a `.deb` [here](https://github.com/metaquanta/soft_fn/releases/tag/v0.2).

To build a `.deb`, clone then `dpkg-buildpackage -us -uc`.

If you don't want a `.deb`, clone then `make`.

## Verification

My public key is at [metaquanta.com/deb_signing_key.gpg](https://metaquanta.com/deb_signing_key.gpg).

## Compatibility

Tested in trixie on Redrix, but should work on any intel Chromebook.

## Acknowledgments

Thank you, @Francesco149 for [stupidlayers](https://github.com/Francesco149/stupidlayers), even though I've since refactored it away.
