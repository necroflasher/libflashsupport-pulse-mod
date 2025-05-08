#pragma once

#include <pulse/pulseaudio.h>

void pulse_context_state_cb(pa_context *ctx, void *ud);
void pulse_stream_state_cb(pa_stream *stream, void *ud);
void pulse_stream_write_cb(pa_stream *stream, size_t numbytes, void *ud);
