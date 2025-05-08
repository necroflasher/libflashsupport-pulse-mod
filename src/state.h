#pragma once

#include <pulse/pulseaudio.h>

struct flashsupport_state
{
	pa_threaded_mainloop *mainloop;
	pa_context           *context;
	pa_stream            *stream;
	unsigned long         frames_out; // just for debug prints
};
