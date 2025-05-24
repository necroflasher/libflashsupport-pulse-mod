# libflashsupport-pulse-mod

## about

this is an improved sound output library for flash player on linux

it's implemented using the optional "libflashsupport" platform abstraction
library used by flash player on linux (more on that [here][h1] and [here][h2])

the code was originally based on [flashsupport.c](https://git.0pointer.net/libflashsupport.git/tree/flashsupport.c) by Lennart Poettering (the systemd guy)

[h1]: https://web.archive.org/web/20061106172854/http://www.kaourantin.net/2006/10/extending-reach-of-flash-player-on.html
[h2]: https://web.archive.org/web/20090130103906/http://labs.adobe.com/wiki/index.php/Flash_Player:Additional_Interface_Support_for_Linux

## what's so good about it

- lower latency
  - more satisfying to click things
  - sound and visuals might be less out of sync
  - some flashes might look better because `Sound.position` will update more
    frequently

- more consistent latency
  - flashes are no longer desynced by different amounts when reopened - this
    could be caused by pulseaudio randomly assigning a latency value on startup

- bug fixes
  - stroke.swf now plays without pausing
  - the very specific version of flash player 27.0.0.130 had a bug where it
    could bypass your volume control, using this library avoids the issue

## compile and install

requires gcc and libpulse development files for both 64-bit and 32-bit

```sh
make
sudo make install
```

on some distros (including arch but not debian), the directories installed to
might have to be added to the runtime linker's search path. if you use such a
highly advanced distro you probably know how to do this already

## compatibility

tested with pulseaudio. no idea if it'll work with pipewire, i'm specifically
worried about the hack in [`pulsethread.c:write_failed`](https://github.com/necroflasher/libflashsupport-pulse-mod/blob/e762250f69c7d02bc2b454cbcdb8f51cc6e2782b/src/pulsethread.c#L119)

## how to tell it's working

if the library is used, the audio source's name in pulseaudio should be
`Flash Animation`. check it using pavucontrol, pulsemixer or
`pactl list sink-inputs`

you can also try these flashes:

- [stroke.swf] - freezes for a while without this library
- [tamagunchi.swf] - should look smoother

[stroke.swf]: https://archive.4plebs.org/f/search/image/Wgh0IxMQJ8naKedINOwb1A/
[tamagunchi.swf]: https://archive.4plebs.org/f/search/image/wsgE9Wl_oCfuQuW0_Vu3iQ/

## environment variables

these can be set to customize the behavior of the library

- `TRACE_ALL`: print some debug messages to stderr
- `TRACE_TIMESTAMPS`: include timestamps in trace output
- `LATENCY_FRAMES` or `LATENCY_MSEC`: set the desired latency in audio frames or
  milliseconds (only one of these can be used at a time)
