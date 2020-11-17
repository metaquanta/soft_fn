# soft-fn

**soft-fn** reproduces the behavior of the Chromebook keyboard's **Search** key in linux. It uses an `evdev` `uinput` device, such that it works in Xorg, Wayland, and virtual consoles. 

## Functionality

When certain keys are pressed in combination with **Search**/🔍︎ (located where the caps lock is usually found) different events are produced. This is similar to how the **fn** key behaves on many laptops (no event is produced for the **Search** key itself). However, unlike **fn**, **Search**/🔍︎ functions like an ordinary accelerator key when used in combination with keys _not_ defined below. **Search**/🔍︎ also generates an event when tapped.

Additionally, **caps lock** may be activated by **alt** + 🔍︎.

This emulates the behavior seen in ChromeOS while also allowing the **Search**/🔍︎ key to be used for user-defined shortcuts with the other keys.


## Key Map

**Search**/🔍︎ produces a `KEY_LEFTMETA` code when tapped or held in combination with a key not defined below. This is the code for the key variously known as **super**, **command**/⌘, or the "windows" key.

The codes below are defined in [linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)


| Key  | code             | 🔍︎+key code |
|------|------------------|-------------|
| F1/⇦ | `BACK`           | `F1`        |
| F2/⇨ | `FORWARD`        | `F2`        |
| F3/⟳ | `REFRESH`        | `F3`        |
| F4/⇱︎ | `DASHBOARD`¹     | `F4`        |
| F5/🍱 | `SCALE`²         | `F5`        |
| F6/🔅| `BRIGHTNESSDOWN` | `F6`        |
| F7/🔆| `BRIGHTNESSUP`   | `F7`        |
| F8/🔇| `MUTE`           | `F8`        |
| F9/🔉| `VOLUMEDOWN`     | `F9`        |
|F10/🔊| `VOLUMEUP`       | `F10`       |
|  ⏻   | nothing³         | `POWER`     |
|  ⌫   | `BACKSPACE`      | `DELETE`    |
|  ←   | `LEFT`           | `HOME`      |
|  →   | `RIGHT`          | `END`       |
|  ↑   | `UP`             | `PAGEUP`    |
|  ↓   | `DOWN`           | `PAGEDOWN`  |
 
¹ The `KEY_FULL_SCREEN` code is inconveniently located beyond the lower 7-bits and not well known to software.

² `KEY_SCALE` is the code Apple keyboards use for the "expose" key, which performs a similar function.

³ This is different from the ChromeOS behavior.

Note: The power key on traditional form-factor Chromebooks generates a redundant `KEY_POWER` event from a second dedicated device that will trigger a suspend regardless of **soft-fn**. To disable this device copy the included udev rules file to `/etc/udev/rules.d/`. Do not disable this device to retain the functionality of
the ⏻/**power** key next to the volume rocker on convertibles. 

## Installation

For AMD64 Debian/Ubuntu/Gallium/etc, get a `.deb` [here](https://github.com/metaquanta/soft_fn/releases/tag/v0.2).

To build a `.deb`, clone then `dpkg-buildpackage -us -uc`.

If you don't want a `.deb`, clone then `make`.

## Compatibility

Tested in bullseye on CHELL, but should work on any intel Chromebook.

## Acknowledgments

Thank you, @Francesco149 for [stupidlayers](https://github.com/Francesco149/stupidlayers), even though I've since refactored it away.
