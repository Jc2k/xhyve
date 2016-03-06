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
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <xhyve/support/misc.h>
#include <xhyve/support/linker_set.h>
#include <xhyve/xhyve.h>
#include <xhyve/pci_emul.h>
#include <xhyve/virtio.h>
#include <pci_virtio_stream.h>

static int pci_vtp9_debug=1;
#define DPRINTF(params) if (pci_vtp9_debug) printf params

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"

struct virtio_p9_config {
	uint16_t tag_len;
	char tag[32];
};

struct p9_msg {
	uint32_t                     size;
	uint8_t                      cmd;
	uint16_t                     tag;
};


/*
 * Per-device softc
 */
struct pci_vtp9_softc {
	struct virtio_softc vrsc_vs;
	struct vqueue_info vrsc_vq;
	pthread_mutex_t vrsc_mtx;
	uint64_t vrsc_cfg;
	struct virtio_p9_config vsc_config;
};
#pragma clang diagnostic pop

typedef void pci_virtio_p9_handler(struct virtio_stream_t *stream);

static void pci_vtp9_reset(void *);
static void pci_vtp9_notify(void *, struct vqueue_info *);
static int pci_vtp9_cfgwrite(UNUSED void *vsc, int offset, UNUSED int size, UNUSED uint32_t value);
static int pci_vtp9_cfgread(void *vsc, int offset, int size, uint32_t *retval);

static struct virtio_consts vtp9_vi_consts = {
	"virtio-p9", /* our name */
	1, /* we support 1 virtqueue */
	sizeof(struct virtio_p9_config), /* config reg size */
	pci_vtp9_reset, /* reset */
	pci_vtp9_notify, /* device-wide qnotify */
	pci_vtp9_cfgread, /* read virtio config */
	pci_vtp9_cfgwrite, /* write virtio config */
	NULL, /* apply negotiated features */
	1, /* our capabilities */
};


static void pci_vtp9_reset(void *vsc)
{
	struct pci_vtp9_softc *sc = vsc;
	vi_reset_dev(&sc->vrsc_vs);
}

static void pci_virtio_p9_version(struct virtio_stream_t *stream) {
	int32_t size = virtio_stream_read_int32(stream);
	char *version = virtio_stream_read_string(stream);
	DPRINTF(("virtio-p9: VERSION: '%d' '%s'\r\n", size, version));

	virtio_stream_write_int32(stream, 0);
	virtio_stream_write_int8(stream, 101);
	virtio_stream_write_int16(stream, -1);

	virtio_stream_write_int32(stream, size);
	if (!strcmp(version, "9P2000.L"))
		virtio_stream_write_string(stream, version);
	else
		virtio_stream_write_string(stream, "unknown");

  virtio_stream_write_finalize(stream);

	free(version);
}

static void pci_virtio_p9_attach(struct virtio_stream_t *stream) {
	int32_t fid_val = virtio_stream_read_int32(stream);
	int32_t afid = virtio_stream_read_int32(stream);
	char *uname = virtio_stream_read_string(stream);
	char *aname = virtio_stream_read_string(stream);
	int32_t uid = virtio_stream_read_int32(stream);

	DPRINTF(("virtio-p9: ATTACH: fid_val='%d' afid='%d' uname='%s' aname='%s' uid='%d'\r\n", fid_val, afid, uname, aname, uid));

	free(uname);
	free(aname);

	virtio_stream_write_int32(stream, 0);
	virtio_stream_write_int8(stream, 105);
	virtio_stream_write_int16(stream, -1);
  virtio_stream_write_finalize(stream);
}

static void pci_virtio_p9_getattr(struct virtio_stream_t *stream) {
	int32_t fid_val = virtio_stream_read_int32(stream);
	int64_t request_mask = virtio_stream_read_int64(stream);

	DPRINTF(("virtio-p9: GETATTR: fid_val='%d' mask='%lld'\r\n", fid_val, request_mask));

	virtio_stream_write_int32(stream, 0);
	virtio_stream_write_int8(stream, 25);
	virtio_stream_write_int16(stream, -1);
  virtio_stream_write_finalize(stream);
}

static void pci_virtio_p9_walk(struct virtio_stream_t *stream) {
	int32_t fid_val = virtio_stream_read_int32(stream);
	int32_t newfid_val = virtio_stream_read_int32(stream);
	int16_t nwname = virtio_stream_read_int16(stream);

	DPRINTF(("virtio-p9: WALK: fid_val='%d' newfid_val='%d' nwname='%d'\r\n", fid_val, newfid_val, nwname));

	virtio_stream_write_int32(stream, 0);
	virtio_stream_write_int8(stream, 111);
	virtio_stream_write_int16(stream, -1);
  virtio_stream_write_finalize(stream);
}

static pci_virtio_p9_handler *virtio_p9_handlers [] = {
	[24]  = pci_virtio_p9_getattr,
	[100] = pci_virtio_p9_version,
	[104] = pci_virtio_p9_attach,
	[110] = pci_virtio_p9_walk,
};

static void pci_vtp9_notify(void *vsc, struct vqueue_info *vq)
{
	struct iovec iov[2];
	struct pci_vtp9_softc *sc;
	struct virtio_stream_t *stream;

	uint16_t idx;
	int32_t size;
	int8_t cmd;
	int16_t tag;

	sc = vsc;

	while (vq_has_descs(vq)) {
		vq_getchain(vq, &idx, iov, 2, NULL);
		stream = virtio_stream_new(&iov[0], &iov[1]);

		size = virtio_stream_read_int32(stream);
		cmd = virtio_stream_read_int8(stream);
		tag = virtio_stream_read_int16(stream);

		DPRINTF(("virtio-p9: NOTIFY: size=%d cmd=%d tag=%d\r\n", size, cmd, tag));

		virtio_p9_handlers[cmd](stream);
		virtio_stream_free(stream);

		vq_relchain(vq, idx, ((uint32_t) iov[0].iov_len));
	}
	vq_endchains(vq, 1);	/* Generate interrupt if appropriate. */
}

static int pci_vtp9_cfgwrite(UNUSED void *vsc, UNUSED int offset, UNUSED int size, UNUSED uint32_t value)
{
	return (1);
}

static int pci_vtp9_cfgread(void *vsc, int offset, int size, uint32_t *retval)
{
	struct pci_vtp9_softc *sc = vsc;
	void *ptr;
	ptr = (uint8_t *)&sc->vsc_config + offset;
	memcpy(retval, ptr, size);
	return (0);
}

static int pci_vtp9_init(struct pci_devinst *pi, UNUSED char *opts)
{
	struct pci_vtp9_softc *sc;
	sc = calloc(1, sizeof(struct pci_vtp9_softc));

	vi_softc_linkup(&sc->vrsc_vs, &vtp9_vi_consts, sc, pi, &sc->vrsc_vq);
	sc->vrsc_vs.vs_mtx = &sc->vrsc_mtx;

	sc->vrsc_vq.vq_qsize = 64;

	sc->vsc_config.tag_len = strlen("kvm_9p");
	memcpy(&sc->vsc_config.tag, "kvm_9p", sc->vsc_config.tag_len);

	/* initialize config space */
	pci_set_cfgdata16(pi, PCIR_DEVICE, 0x1009);
	pci_set_cfgdata16(pi, PCIR_VENDOR, VIRTIO_VENDOR);
	pci_set_cfgdata8(pi, PCIR_CLASS, 0xff);
	pci_set_cfgdata16(pi, PCIR_SUBDEV_0, 9);
	pci_set_cfgdata16(pi, PCIR_SUBVEND_0, VIRTIO_VENDOR);

	if (vi_intr_init(&sc->vrsc_vs, 1, fbsdrun_virtio_msix()))
		return (1);
	vi_set_io_bar(&sc->vrsc_vs, 0);

	return (0);
}


static struct pci_devemu pci_de_vp9 = {
	.pe_emu =	"virtio-p9",
	.pe_init =	pci_vtp9_init,
	.pe_barwrite =	vi_pci_write,
	.pe_barread =	vi_pci_read
};
PCI_EMUL_SET(pci_de_vp9);
