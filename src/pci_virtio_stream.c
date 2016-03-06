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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <xhyve/support/misc.h>
#include <xhyve/pci_emul.h>
#include <xhyve/virtio.h>
#include <pci_virtio_stream.h>

struct virtio_stream_t {
	struct iovec *read_iovec;
	struct iovec *write_iovec;
	size_t read_position;
	size_t write_position;
};

struct virtio_stream_t*
virtio_stream_new(struct iovec *read, struct iovec *write) {
	struct virtio_stream_t *stream = malloc(sizeof(struct virtio_stream_t));
	stream->read_iovec = read;
	stream->write_iovec = write;
	stream->read_position = 0;
	stream->write_position = 0;
	return stream;
}

void
virtio_stream_free(struct virtio_stream_t* stream) {
	free(stream);
}

void
virtio_stream_read(struct virtio_stream_t *stream, void* data, size_t size) {
	uintptr_t loc = (uintptr_t)stream->read_iovec->iov_base + stream->read_position;
	printf("virtio_stream_read: %d %d %d %d\r\n", (int)stream->read_iovec->iov_base, (int)stream->read_position, (int)loc, (int)size);
	memcpy(data, (void *)loc, size);
	stream->read_position += size;
}

int8_t
virtio_stream_read_int8(struct virtio_stream_t *stream) {
	int8_t retval;
	virtio_stream_read(stream, &retval, sizeof(retval));
	return retval;
}

int16_t
virtio_stream_read_int16(struct virtio_stream_t *stream) {
	int16_t retval;
	virtio_stream_read(stream, &retval, sizeof(retval));
	return retval;
}

int32_t
virtio_stream_read_int32(struct virtio_stream_t *stream) {
	int32_t retval;
	virtio_stream_read(stream, &retval, sizeof(retval));
	return retval;
}

int64_t
virtio_stream_read_int64(struct virtio_stream_t *stream) {
	int64_t retval;
	virtio_stream_read(stream, &retval, sizeof(retval));
	return retval;
}

char*
virtio_stream_read_string(struct virtio_stream_t *stream) {
	size_t length = (size_t)virtio_stream_read_int16(stream);
	printf("virtio_stream_read_string: length=%d\r\n", (int)length);
	char *retval = malloc(length + 1);
	virtio_stream_read(stream, retval, length);
	retval[length] = 0;
	printf("virtio_stream_read_string: retval='%s'\r\n", retval);
	return retval;
}

void
virtio_stream_write(struct virtio_stream_t *stream, void* data, size_t size) {
	uintptr_t loc = (uintptr_t)stream->write_iovec->iov_base + stream->write_position;
	printf("virtio_stream_write: %d %d %d %d\r\n", (int)stream->write_iovec->iov_base, (int)stream->read_position, (int)loc, (int)size);
	memcpy((void *)loc, data, size);
	stream->write_position += size;
}

void
virtio_stream_write_int8(struct virtio_stream_t *stream, int8_t value) {
	virtio_stream_write(stream, &value, sizeof(value));
}

void
virtio_stream_write_int16(struct virtio_stream_t *stream, int16_t value) {
	virtio_stream_write(stream, &value, sizeof(value));
}

void
virtio_stream_write_int32(struct virtio_stream_t *stream, int32_t value) {
	virtio_stream_write(stream, &value, sizeof(value));
}

void
virtio_stream_write_int64(struct virtio_stream_t *stream, int64_t value) {
	virtio_stream_write(stream, &value, sizeof(value));
}

void
virtio_stream_write_string(struct virtio_stream_t *stream, char* value) {
	size_t length = strlen(value);
	virtio_stream_write_int16(stream, (int16_t)length);
	virtio_stream_write(stream, value, length);
}

void
virtio_stream_write_finalize(struct virtio_stream_t *stream) {
	size_t write_position = stream->write_position;
	stream->write_position = 0;
	virtio_stream_write_int32(stream, (int32_t)write_position);
}
