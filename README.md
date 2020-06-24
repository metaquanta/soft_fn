# soft-fn

**soft-fn** reproduces the behavior of the Chromebook keyboard's **Search** key in linux. It uses an `evdev` `uinput` device, such that it works in Xorg, Wayland, and virtual consoles. 

## Functionality

When certain keys are pressed in combination with **Search**/🔍︎ (located where the caps lock is usually found) different events are produced. This is similar to how the **fn** key behaves on many laptops (no event is produced for the **Search** key itself). However, unlike **fn**, **Search**/🔍︎ functions like an ordinary accelerator key when used in combination with keys _not_ defined below. **Search**/🔍︎ also generates an event when tapped.

Additionally, **caps lock** may be activated by **alt** + 🔍︎.

This emulates the behavior seen in ChromeOS while also allowing the **Search**/🔍︎ key to be used for user-defined shortcuts with the other keys.


## Key Map

**Search**/🔍︎ produces a `KEY_LEFTMETA` code when tapped or held in combination with a key not defined below. This is the code for the key variously known as **super**, **command**/⌘, or the "windows" key.

The codes below are defined in [linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)

The function keys between **esc** and **power** are identified as **F1** to **F10** from left to right below.

| Key  | code                 | 🔍︎+key code |
|------|----------------------|-----------------|
| F1/⇦ | `F1`  | `BACK`      |
| F2/⇨ | `F2`  | `FORWARD`   |
| F3/⟳ | `F3`  | `REFRESH`   |
| F4/⇱︎ | `F4`  | `DASHBOARD`¹|
| F5/⯣ | `F5`  | `SCALE`²    |
| F6/🔅| `F6`  | `BRIGHTNESSDOWN` |
| F7/🔆| `F7`  | `BRIGHTNESSUP`   |
| F8/🔇| `F8`  | `MUTE`           |
| F9/🔉| `F9`  | `VOLUMEDOWN`     |
|F10/🔊| `F10` | `VOLUMEUP`       |
| ⏻/🔒︎ | `F11`³ | `POWER`         |
|  ⌫   | `BACKSPACE` | `DELETE` |
|  ←   | `LEFT`  | `HOME`       |
|  →   | `RIGHT` | `END`        |
|  ↑   | `UP`    | `PAGEUP`     |
|  ↓   | `DOWN`  | `PAGEDOWN`   |
 
¹ The `KEY_FULL_SCREEN` code is inconveniently located beyond the lower 7-bits and not well known to software.

² `KEY_SCALE` is the code Apple keyboards use for the "expose" key, which performs a similar function.

³ This is different from the ChromeOS behavior.

The **power**/⏻ (or **logout**/🔒︎) key usually generates a `KEY_POWER` (or `KEY_F13`) event. With **soft-fn** it generates `KEY_F11` alone and `KEY_POWER` with **Search**. 

Note: The power key on traditional form-factor Chromebooks generates a redundant `KEY_POWER` event from a second dedicated device that will trigger a suspend regardless of **soft-fn**. To disable this device copy the included udev rules file to `/etc/udev/rules.d/`. 
I need more information on the behavior of the power key and other input devices on convertible Chromebooks.

## Installation

For AMD64 Debian/Ubuntu/Gallium/etc, get a `.deb` [here](https://github.com/metaquanta/soft_fn/releases/tag/v0.2).

To build a `.deb`, clone then `dpkg-buildpackage -us -uc`.

If you don't want a `.deb`, clone then `make`.

## Compatibility

Tested in bullseye on CHELL and CAVE, but should work on any laptop that emulates an i8042-controlled keyboard.
