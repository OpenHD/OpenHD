#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lib.h"

void lib_init_packet_buffer(packet_buffer_t *p) {
	assert(p != NULL);

	p->valid = 0;
	p->crc_correct = 0;
	p->len = 0;
	p->data = NULL;
}

void lib_alloc_packet_buffer(packet_buffer_t *p, size_t len) {
	assert(p != NULL);
	assert(len > 0);

	p->len = 0;
	p->data = (uint8_t*)malloc(len);
}

void lib_free_packet_buffer(packet_buffer_t *p) {
	assert(p != NULL);

	free(p->data);
	p->len = 0;
}

packet_buffer_t *lib_alloc_packet_buffer_list(size_t num_packets, size_t packet_length) {
	packet_buffer_t *retval;
	int i;

	assert(num_packets > 0 && packet_length > 0);

	retval = (packet_buffer_t *)malloc(sizeof(packet_buffer_t) * num_packets);
	assert(retval != NULL);

	for(i=0; i<num_packets; ++i) {
		lib_init_packet_buffer(retval + i);
		lib_alloc_packet_buffer(retval + i, packet_length);
	}

	return retval;
}

void lib_free_packet_buffer_list(packet_buffer_t *p, size_t num_packets) {
	int i;

	assert(p != NULL && num_packets > 0);

	for(i=0; i<num_packets; ++i) {
		lib_free_packet_buffer(p+i);
	}

	free(p);
}
