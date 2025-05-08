#include "flashexports.h"

#include <stdbool.h>
#include "config.h"
#include "globals.h"
#include "pulsethread.h"
#include "state.h"
#include "util.h"

static void *FPX_SoundOutput_Open(void);
static int   FPX_SoundOutput_Close(struct flashsupport_state *state);
static int   FPX_SoundOutput_Latency(struct flashsupport_state *state);

static const struct FPX_Functions fpx_functions = {
	.count = 12,
	.funcs = {
		[6] = FPX_SoundOutput_Open,
		[7] = FPX_SoundOutput_Close,
		[8] = FPX_SoundOutput_Latency,
	},
};

__attribute__((visibility("default")))
const void *FPX_Init(void *api)
{
	if (config->trace_all)
		trace("FPX_Init");

	fpi_soundoutput_fillbuffer = ((struct FPX_Functions *)api)->funcs[2];

	return &fpx_functions;
}

static bool create_mainloop(struct flashsupport_state *state);
static bool create_context(struct flashsupport_state *state);
static bool start_mainloop(struct flashsupport_state *state);
static bool wait_context(struct flashsupport_state *state);
static bool create_start_wait_stream(struct flashsupport_state *state);

static void *FPX_SoundOutput_Open(void)
{
	struct flashsupport_state *state;

	if (config->trace_all)
		trace(">> FPX_SoundOutput_Open");

	state = calloc(1, sizeof(struct flashsupport_state));
	if (!state)
	{
		warn("out of memory");
		goto fail;
	}

	if (!create_mainloop(state))
		goto fail;

	if (!create_context(state))
		goto fail;

	if (!start_mainloop(state))
		goto fail;

	pa_threaded_mainloop_lock(state->mainloop);

	if (!wait_context(state))
		goto unlock_and_fail;

	if (!create_start_wait_stream(state))
		goto unlock_and_fail;

	pa_threaded_mainloop_unlock(state->mainloop);

	if (config->trace_all)
		trace("<< FPX_SoundOutput_Open");

	return state;

unlock_and_fail:

	pa_threaded_mainloop_unlock(state->mainloop);

fail:

	if (state)
		FPX_SoundOutput_Close(state);

	if (config->trace_all)
		trace("<< FPX_SoundOutput_Open (error)");

	return NULL;
}

static int FPX_SoundOutput_Close(struct flashsupport_state *state)
{
	if (config->trace_all)
		trace(">> FPX_SoundOutput_Close");

	if (state->mainloop)
		pa_threaded_mainloop_stop(state->mainloop);

	if (state->stream)
	{
		pa_stream_disconnect(state->stream);
		pa_stream_unref(state->stream);
	}

	if (state->context)
	{
		pa_context_disconnect(state->context);
		pa_context_unref(state->context);
	}

	if (state->mainloop)
		pa_threaded_mainloop_free(state->mainloop);

	free(state);

	if (config->trace_all)
		trace("<< FPX_SoundOutput_Close");

	return 0;
}

// create the pulse main loop
static bool create_mainloop(struct flashsupport_state *state)
{
	state->mainloop = pa_threaded_mainloop_new();
	if (!state->mainloop)
	{
		warn("pa_threaded_mainloop_new failed");
		return false;
	}

	pa_threaded_mainloop_set_name(state->mainloop, "libflashsupport");

	return true;
}

// create and connect the pulse context
static bool create_context(struct flashsupport_state *state)
{
	state->context = pa_context_new(pa_threaded_mainloop_get_api(state->mainloop), "Adobe Flash");
	if (!state->context)
	{
		warn("pa_context_new failed");
		return false;
	}

	pa_context_set_state_callback(state->context, pulse_context_state_cb, state);

	if (pa_context_connect(state->context, NULL, 0, NULL) < 0)
	{
		warn("pa_context_connect failed");
		return false;
	}

	return true;
}

// start the pulse mainloop
static bool start_mainloop(struct flashsupport_state *state)
{
	if (pa_threaded_mainloop_start(state->mainloop) < 0)
	{
		warn("pa_threaded_mainloop_start failed");
		return false;
	}

	return true;
}

// wait for the context to become ready
static bool wait_context(struct flashsupport_state *state)
{
	for (;;)
	{
		int st = pa_context_get_state(state->context);
		if (st == PA_CONTEXT_READY)
			break;
		if (!PA_CONTEXT_IS_GOOD(st))
		{
			warn("pa_context_get_state != PA_CONTEXT_READY");
			return false;
		}
		pa_threaded_mainloop_wait(state->mainloop);
	}

	return true;
}

// create and connect the stream, wait for it to become ready
static bool create_start_wait_stream(struct flashsupport_state *state)
{
	pa_sample_spec ss = {
		.format   = PA_SAMPLE_S16LE,
		.rate     = 44100,
		.channels = 2,
	};
	pa_buffer_attr attr = {
		.maxlength = -1,
		.tlength   = config->latency_bytes,
		.prebuf    = -1,
		.minreq    = -1,
	};
	int streamflags = 0;

	// new

	state->stream = pa_stream_new(state->context, "Flash Animation", &ss, NULL);
	if (!state->stream)
	{
		warn("pa_stream_new failed");
		return false;
	}

	// set callbacks

	pa_stream_set_state_callback(state->stream, pulse_stream_state_cb, state);
	pa_stream_set_write_callback(state->stream, pulse_stream_write_cb, state);

	// connect playback

	if (!config->test_return_constant_latency_value)
	{
		streamflags |= PA_STREAM_INTERPOLATE_TIMING|PA_STREAM_AUTO_TIMING_UPDATE;
	}

	if (config->test_use_adjust_latency_flag)
	{
		streamflags |= PA_STREAM_ADJUST_LATENCY;
	}

	if (pa_stream_connect_playback(state->stream, NULL, &attr, streamflags, NULL, NULL) < 0)
	{
		warn("pa_stream_connect_playback failed");
		return false;
	}

	// wait for it to become ready

	for (;;)
	{
		int st = pa_stream_get_state(state->stream);
		if (st == PA_STREAM_READY)
			break;
		if (!PA_STREAM_IS_GOOD(st))
		{
			warn("pa_stream_get_state != PA_STREAM_READY");
			return false;
		}
		pa_threaded_mainloop_wait(state->mainloop);
	}

	return true;
}

static int latency_proper(struct flashsupport_state *state);

// fixme: what exactly does this do? can't see/hear any difference with 0 or a huge value
static int FPX_SoundOutput_Latency(struct flashsupport_state *state)
{
	int rv;

	if (config->test_return_constant_latency_value)
		rv = config->latency_frames;
	else
		rv = latency_proper(state);

	if (config->trace_all)
		trace("   FPX_SoundOutput_Latency frames=%d", rv);

	return rv;
}

static int latency_proper(struct flashsupport_state *state)
{
	pa_usec_t usec;
	int negative;
	int rv;

	/* We lock here only if we are not called from our event loop thread */
	if (!pa_threaded_mainloop_in_thread(state->mainloop))
		pa_threaded_mainloop_lock(state->mainloop);

	usec = 0;
	if (pa_stream_get_latency(state->stream, &usec, &negative) < 0)
	{
		rv = 0;
		warn("pa_stream_get_latency failed");
	}
	else if (negative)
	{
		rv = 0;
		// does this happen?
		trace("test: negative latency");
	}
	else
	{
		size_t bytes;
		bytes = pa_usec_to_bytes(usec, pa_stream_get_sample_spec(state->stream));
		rv = (int)(bytes / config->frame_size);
	}

	if (!pa_threaded_mainloop_in_thread(state->mainloop))
		pa_threaded_mainloop_unlock(state->mainloop);

	return rv;
}
