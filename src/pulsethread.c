#include "pulsethread.h"

#include <math.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "globals.h"
#include "state.h"
#include "util.h"

void pulse_context_state_cb(pa_context *ctx, void *ud)
{
	struct flashsupport_state *state = ud;
	pa_context_state_t st;

	st = pa_context_get_state(ctx);

	if (config->trace_all)
		trace("   context state=%d", st);

	switch (st)
	{
		case PA_CONTEXT_READY:
		case PA_CONTEXT_FAILED:
		case PA_CONTEXT_TERMINATED:
			pa_threaded_mainloop_signal(state->mainloop, /* wait_for_accept */ 0);
			break;

		case PA_CONTEXT_UNCONNECTED:
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;
	}
}

void pulse_stream_state_cb(pa_stream *stream, void *ud)
{
	struct flashsupport_state *state = ud;
	pa_stream_state_t st;

	st = pa_stream_get_state(stream);

	if (config->trace_all)
		trace("   stream state=%d", st);

	switch (st)
	{
		case PA_STREAM_READY:
		case PA_STREAM_FAILED:
		case PA_STREAM_TERMINATED:
		{
			pa_threaded_mainloop_signal(state->mainloop, /* wait_for_accept */ 0);
			if (st == PA_STREAM_READY)
			{
				// we're very important...
				nice(-5);
			}
			break;
		}

		case PA_STREAM_UNCONNECTED:
		case PA_STREAM_CREATING:
			break;
	}
}

__attribute__((cold))
static void write_failed(pa_stream *stream, size_t numbytes);

void pulse_stream_write_cb(pa_stream *stream, size_t numbytes, void *ud)
{
	static const char randombytes[] = {
		0xf2, 0x89, 0x15, 0x2c, 0xd9, 0x24, 0x29, 0xb9,
		0xcd, 0xfb, 0x2a, 0xf9, 0x5e, 0x13, 0x51, 0xed,
	};
	struct flashsupport_state *state = ud;
	char *buf;

	numbytes = MAX(numbytes, sizeof(randombytes));   // at least this much
	numbytes = MIN(numbytes, config->latency_bytes); // not more than this
	numbytes -= (numbytes % config->frame_size);     // round to frames
	numbytes = MIN(numbytes, 1024*1024);             // limit alloca

	if (config->trace_all)
		trace(">> pulse_stream_write: [%luf/%.2fms] -> [%luf/%.2fms]",
			state->frames_out,
			state->frames_out/44.1,
			(state->frames_out+(numbytes/config->frame_size)),
			(state->frames_out+(numbytes/config->frame_size))/44.1
			);

	buf = alloca(numbytes);

	memcpy(buf, randombytes, sizeof(randombytes));

	fpi_soundoutput_fillbuffer(ud, buf, numbytes);

	if (!bcmp(buf, randombytes, sizeof(randombytes)))
	{
		// oh no!!
		write_failed(stream, numbytes);
		return;
	}

	if (pa_stream_write(stream, buf, numbytes, NULL, 0, PA_SEEK_RELATIVE) != 0)
	{
		warn("pa_stream_write failed");
		return;
	}

	state->frames_out += numbytes/config->frame_size;

	if (config->trace_all)
		trace("<< pulse_stream_write");
}

__attribute__((noinline))
static void write_failed(pa_stream *stream, size_t numbytes)
{
	void *buf;
	size_t avail;

	/*
	 * oh no! fillbuffer didn't fill the buffer
	 * 
	 * this normally happens once at startup: the write callback is called
	 *  before the main thread has returned the plugin instance pointer from
	 *  FPX_SoundOutput_Open(), but since fillbuffer needs the same pointer to
	 *  tell who's calling it, it doesn't yet recognize the value here and
	 *  simply returns without doing anything
	 * 
	 * to handle this, we need to wait a little bit and try the write again.
	 *  below is a hacky way to get pulse to do that
	 * 
	 * notes:
	 * - if we simply return here without writing anything, pulse won't call us a second time (hence the hack)
	 * - retrying the call in a loop won't work, main won't return until we're finished
	 * - starting the stream corked doesn't work, it still calls the write callback
	 * 
	 * bug?: this seems to not work if the normal write is changed to use pa_stream_begin_write() (why?)
	 */

	// write some silence

	avail = numbytes;

	if (pa_stream_begin_write(stream, &buf, &avail) != 0 || !buf)
	{
		warn("pa_stream_begin_write failed");
		return;
	}

	memset(buf, 0, avail);

	if (pa_stream_write(stream, buf, avail, NULL, 0, PA_SEEK_RELATIVE) != 0)
	{
		warn("pa_stream_write failed");
		return;
	}

	// cancel it so that no delay is added and the next call happens immediately

	if (pa_stream_flush(stream, NULL, NULL) == NULL)
	{
		warn("pa_stream_flush failed");
		return;
	}

	if (config->trace_all)
		trace("<< pulse_stream_write (error)");
}
