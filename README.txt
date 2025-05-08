
implementation of "libflashsupport" sound output library for flash player

--- links

original source this is based on:
https://git.0pointer.net/libflashsupport.git/tree/flashsupport.c

other references:
https://freedesktop.org/software/pulseaudio/doxygen/
https://github.com/mpv-player/mpv/blob/master/audio/out/ao_pulse.c

historical:
https://web.archive.org/web/20061106172854/http://www.kaourantin.net/2006/10/extending-reach-of-flash-player-on.html
https://web.archive.org/web/20090130103906/http://labs.adobe.com/wiki/index.php/Flash_Player:Additional_Interface_Support_for_Linux

--- test flashes

https://archive.4plebs.org/f/search/image/wsgE9Wl_oCfuQuW0_Vu3iQ/
movement of the characters depends on Sound.position which updates more
 frequently with a low buffer size

https://archive.4plebs.org/f/search/image/Wgh0IxMQJ8naKedINOwb1A/
doesn't play correctly with alsa output, it gets stuck for a while

https://archive.4plebs.org/f/search/image/xNyjZ9XF6BRNJQo_-dIC8w/
flash with synced visuals and sound

fixet_boobs.swf
heartbeat.swf

https://archive.4plebs.org/f/search/image/qKVjBtuikHUpfO-H8pUiSQ/
not properly synced but it's a cool song

some flashes with a transition from intro -> loop don't work seamlessly:
https://archive.4plebs.org/f/search/image/NHQRHHxySSrnO_6X3nv2GA/
https://archive.4plebs.org/f/search/image/WWjqtUN4AftrKRBiuebqAQ/
wonder if there's something to improve or if it's just a problem with the flash itself
^ huh, slightly better with flash 11.2 than 32
11.0 to 11.2 seem to work best
