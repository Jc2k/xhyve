/*-
 * Copyright (c) 2015 xhyve developers
 * Copyright (c) 2016 John Carr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

struct virtio_stream_t;

struct virtio_stream_t* virtio_stream_new(struct iovec *read, struct iovec *write);
void virtio_stream_free(struct virtio_stream_t* stream);

void virtio_stream_read(struct virtio_stream_t *stream, void* data, size_t size);
int8_t virtio_stream_read_int8(struct virtio_stream_t *stream);
int16_t virtio_stream_read_int16(struct virtio_stream_t *stream);
int32_t virtio_stream_read_int32(struct virtio_stream_t *stream);
int64_t virtio_stream_read_int64(struct virtio_stream_t *stream);
char* virtio_stream_read_string(struct virtio_stream_t *stream);

void virtio_stream_write(struct virtio_stream_t *stream, void* data, size_t size);
void virtio_stream_write_int8(struct virtio_stream_t *stream, int8_t value);
void virtio_stream_write_int16(struct virtio_stream_t *stream, int16_t value);
void virtio_stream_write_int32(struct virtio_stream_t *stream, int32_t value);
void virtio_stream_write_int64(struct virtio_stream_t *stream, int64_t value);
void virtio_stream_write_string(struct virtio_stream_t *stream, char* value);
void virtio_stream_write_finalize(struct virtio_stream_t *stream);
