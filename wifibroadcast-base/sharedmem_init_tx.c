// sharedmem_init_rx by Rodizio. Licensed under GPL2
// Creates and initiliazes wifibroadcast shared memory to be used by different
// programs like the osd, tx, rx, etc.
/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "lib.h"
#include <time.h>
#include <sys/resource.h>

wifibroadcast_tx_status_t *tx_status = NULL;
wifibroadcast_rx_status_t *rx_status = NULL;
//wifibroadcast_rx_status_t_osd *rx_status = NULL;
//wifibroadcast_rx_status_t_uplink *rx_status = NULL;

wifibroadcast_rx_status_t_rc *rx_status_rc = NULL;
wifibroadcast_rx_status_t_sysair *rx_status_sysair = NULL;

void status_memory_init(wifibroadcast_rx_status_t *s) {
	s->received_block_cnt = 0;
	s->damaged_block_cnt = 0;
	s->received_packet_cnt = 0;
	s->lost_packet_cnt = 0;
	s->tx_restart_cnt = 0;
	s->wifi_adapter_cnt = 0;

	int i;
	for(i=0; i<8; ++i) {
		s->adapter[i].received_packet_cnt = 0;
		s->adapter[i].wrong_crc_cnt = 0;
		s->adapter[i].current_signal_dbm = -1;
	}
}

void status_memory_init_tx(wifibroadcast_tx_status_t *s) {
    s->last_update = 0;
    s->injected_block_cnt = 0;
    s->skipped_fec_cnt = 0;
    s->injection_fail_cnt = 0;
    s->injection_time_block = 0;
}

void status_memory_init_osd(wifibroadcast_rx_status_t *s) {
	s->received_block_cnt = 0;
	s->damaged_block_cnt = 0;
	s->received_packet_cnt = 0;
	s->lost_packet_cnt = 0;
	s->tx_restart_cnt = 0;
	s->wifi_adapter_cnt = 0;

	int i;
	for(i=0; i<8; ++i) {
		s->adapter[i].received_packet_cnt = 0;
		s->adapter[i].wrong_crc_cnt = 0;
		s->adapter[i].current_signal_dbm = -1;
	}
}

void status_memory_init_rc(wifibroadcast_rx_status_t_rc *s) {
	s->received_block_cnt = 0;
	s->damaged_block_cnt = 0;
	s->received_packet_cnt = 0;
	s->lost_packet_cnt = 0;
	s->tx_restart_cnt = 0;
	s->wifi_adapter_cnt = 0;

	int i;
	for(i=0; i<8; ++i) {
		s->adapter[i].received_packet_cnt = 0;
		s->adapter[i].wrong_crc_cnt = 0;
		s->adapter[i].current_signal_dbm = -1;
	}
}

void status_memory_init_sysair(wifibroadcast_rx_status_t_sysair *s) {
	s->cpuload = 0;
	s->temp = 0;
	s->skipped_fec_cnt = 0;
	s->injected_block_cnt = 0;
	s->injection_fail_cnt = 0;
	s->injection_time_block = 0;
	s->bitrate_kbit = 0;
	s->bitrate_measured_kbit = 0;
	s->cts = 0;
	s->undervolt = 0;
}


wifibroadcast_rx_status_t *status_memory_open(void) {
	char buf[128];
	int fd;
	sprintf(buf, "/wifibroadcast_rx_status_0");
	fd = shm_open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { perror("shm_open"); exit(1); }
	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	wifibroadcast_rx_status_t *tretval = (wifibroadcast_rx_status_t*)retval;
	status_memory_init(tretval);
	return tretval;
}

wifibroadcast_rx_status_t *status_memory_open_osd(void) {
	char buf[128];
	int fd;
	sprintf(buf, "/wifibroadcast_rx_status_3");
	fd = shm_open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { perror("shm_open"); exit(1); }
	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	wifibroadcast_rx_status_t *tretval = (wifibroadcast_rx_status_t*)retval;
	status_memory_init(tretval);
	return tretval;
}

wifibroadcast_rx_status_t_rc *status_memory_open_rc(void) {
	char buf[128];
	int fd;
	sprintf(buf, "/wifibroadcast_rx_status_rc");
	fd = shm_open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { perror("shm_open"); exit(1); }
	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t_rc)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t_rc), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	wifibroadcast_rx_status_t_rc *tretval = (wifibroadcast_rx_status_t_rc*)retval;
	status_memory_init_rc(tretval);
	return tretval;
}

wifibroadcast_rx_status_t *status_memory_open_uplink(void) {
	char buf[128];
	int fd;
	sprintf(buf, "/wifibroadcast_rx_status_uplink");
	fd = shm_open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { perror("shm_open"); exit(1); }
	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	wifibroadcast_rx_status_t *tretval = (wifibroadcast_rx_status_t*)retval;
	status_memory_init(tretval);
	return tretval;
}

wifibroadcast_tx_status_t *status_memory_open_tx(void) {
	char buf[128];
	int fd;
	sprintf(buf, "/wifibroadcast_tx_status_0");
	fd = shm_open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { perror("shm_open"); exit(1); }
	if (ftruncate(fd, sizeof(wifibroadcast_tx_status_t)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_tx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	wifibroadcast_tx_status_t *tretval = (wifibroadcast_tx_status_t*)retval;
	status_memory_init_tx(tretval);
	return tretval;
}

wifibroadcast_rx_status_t_sysair *status_memory_open_sysair(void) {
	char buf[128];
	int fd;
	sprintf(buf, "/wifibroadcast_rx_status_sysair");
	fd = shm_open(buf, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { perror("shm_open"); exit(1); }
	if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t_sysair)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t_sysair), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	wifibroadcast_rx_status_t_sysair *tretval = (wifibroadcast_rx_status_t_sysair*)retval;
	status_memory_init_sysair(tretval);
	return tretval;
}


int main(int argc, char *argv[]) {
	setpriority(PRIO_PROCESS, 0, 10);

//	printf("sharedmem_init_tx started\n");

	tx_status = status_memory_open_tx();
	rx_status = status_memory_open();
	rx_status = status_memory_open_osd();
	rx_status_rc = status_memory_open_rc();
//	rx_status = status_memory_open_uplink();
//	rx_status_sysair = status_memory_open_sysair();

	return (0);
}
