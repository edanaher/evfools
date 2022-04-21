EvFools
=======
A memorable portmanteau of EvDev, foot, and tools, this is a few tools to help make
foot pedals a useful input device when combined with
[KMonad](https://github.com/kmonad/kmonad).

I use this with three three-key foot pedals to allow me to do common web browsing
hand-free; tasks like scrolling, changing tabs, and closing tabs are easy, and
morse code provides the ability to follow links by pressing single letters using
[link hints](https://github.com/philc/vimium/wiki/Using-Link-Hints).

## evmerge

This simple tools combines multiple evdev devices into a single one.  There are
other tools that do this, but I couldn't get any of them to reliably work, and
there's not a whole lot to it.

It's complicated a bit by the ability to remap keys for some devices - e.g., if
two foot pedals send the same keys, one of them can be re-mapped here so that
kmonad sees each key producing a different value.

## dobuttons

This converts various high F-keys to mouse clicks; if this is run after kmonad,
it can be used to effectively allow kmonad to press mouse buttons as well.

I'm not currently using it; it probably mostly works.

## evmorse

This converts F17 (as produced by kmonad) into a morse-key that can generate the
alphabet and numbers via morse code.  (E.g., a short press, quick release, long
press, and long release will product an "A").

It also supports modifiers; any key pressed between F18's (e.g., F18, ctrl, F18)
will wrap the next pressed key - so the sequence (F18, ctrl, F18, short F17,
long F17, pause) will product ctrl-A.  This is easy to in kmonad with its
"around" functionality: (around F18 lctrl) will cause the next morse code key to
be wrapped in control.

This isn't intended for full typing, but as a sort of "escape hatch" for rare
keys that don't fit in the normal mapping.  For example, with [link
hints](https://github.com/philc/vimium/wiki/Using-Link-Hints), I can follow a
link on a web page by pressing the hint key, then the letter of the link; it's
a bit more work than just clicking the link, but I haven't yet come up with a
better way to do this.

I may in the future try to do a faster four-key typing mechanism (two keys per
foot seems like the most that can be used ergonomically at a reasonable speed).
But for the moment, morse code is simple and fast enough.
