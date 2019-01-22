/* -*- c -*-
 */
// Copyright (C) 2017, 2018 Vasily Evseenko <svpcom@p2ptech.org>

/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 3.
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


#include <stdio.h>
#include <sodium.h>

int main(void)
{
    unsigned char drone_publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char drone_secretkey[crypto_box_SECRETKEYBYTES];
    unsigned char gs_publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char gs_secretkey[crypto_box_SECRETKEYBYTES];
    FILE *fp;

    crypto_box_keypair(drone_publickey, drone_secretkey);
    crypto_box_keypair(gs_publickey, gs_secretkey);

    if((fp = fopen("tx.key", "w")) == NULL)
    {
        perror("Unable to save tx.key");
        return 1;
    }

    fwrite(drone_secretkey, crypto_box_SECRETKEYBYTES, 1, fp);
    fwrite(gs_publickey, crypto_box_PUBLICKEYBYTES, 1, fp);
    fclose(fp);

    fprintf(stderr, "Drone keypair (drone sec + gs pub) saved to tx.key\n");

    if((fp = fopen("rx.key", "w")) == NULL)
    {
        perror("Unable to save rx.key");
        return 1;
    }

    fwrite(gs_secretkey, crypto_box_SECRETKEYBYTES, 1, fp);
    fwrite(drone_publickey, crypto_box_PUBLICKEYBYTES, 1, fp);
    fclose(fp);

    fprintf(stderr, "GS keypair (gs sec + drone pub) saved to rx.key\n");
    return 0;
}
