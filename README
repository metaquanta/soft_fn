# soft-fn

*soft-fn* reproduces the behavior of the Chromebook keyboard's *Search* key in linux. It uses an `evdev` `uinput` device, such that it works in Xorg, Wayland, and virtual consoles. 

## Functionality

When certain keys are pressed in combination with *Search*/🔍︎ (located where the caps lock is usually found) different events are produced. This is similar to how the *fn* key behaves on many laptops (no event is produced for the *Search* key itself). However, unlike *fn*, *Search*/🔍︎ functions like an ordinary accelerator key when used in combination with keys _not_ defined below. *Search*/🔍︎ also generates an event when tapped.

Additionally, *caps lock* may be activated by *alt* + 🔍︎.

This emulates the behavior seen in ChromeOS while also allowing the *Search*/🔍︎ key to be used for user-defined shortcuts with the other keys.


## Key Map

*Search*/🔍︎ produces a `KEY_LEFTMETA` code when tapped or held in combination with a key not defined below. This is the code for the key variously known as *super*, *command*/⌘, or the "windows" key.

The function keys between *esc* and *power* are identified as *F1* to *F10* from left to right below.

| Key  | code                 | *Search* code |
| F1/⇦ | F1  | BACK      |
| F2/⇨ | F2  | FORWARD   |
| F3/⟳ | F3  | REFRESH   |
| F4/⇱︎ | F4  | DASHBOARD¹|
| F5/⯣ | F5  | SCALE²    |
| F6/🔅| F6  | BRIGHTNESSDOWN |
| F7/🔆| F7  | BRIGHTNESSUP   |
| F8/🔇| F8  | MUTE           |
| F9/🔉| F9  | VOLUMEDOWN     |
|F10/🔊| F10 | VOLUMEUP       |
| ⏻/🔒︎ | F11 | POWER          |
|  ⌫   | BACKSPACE | DELETE |
|  ←   | LEFT  | HOME       |
|  →   | RIGHT | END        |
|  ↑   | UP    | PAGEUP     |
|  ↓   | DOWN  | PAGEDOWN   |


The *power*/⏻ (or *logout*/🔒︎) key usually generates a `KEY_POWER` (or `KEY_F13`) event. With *soft-fn* it generates `KEY_F11` alone and `KEY_POWER` with *Search*. 
(Note: The power key on traditional form-factor Chromebooks generates a redundant *KEY_POWER* event from a second dedicated device called `power button`. The `power button` device must be disabled elsewhere to prevent some systems from suspending regardless of *soft-fn*. This isn't a problem with convertibles.)

¹ The `KEY_FULL_SCREEN` is inconveniently beyond beyond 7-bits and not well known/often interpreted by software.
² `KEY_SCALE` is the code Apple keyboards use for the "expose" key, which performs a similar function.
