/* -*- c -*-
 */
// Copyright (C) 2017 Vasily Evseenko <svpcom@p2ptech.org>

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
    unsigned char tx_publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char tx_secretkey[crypto_box_SECRETKEYBYTES];
    unsigned char rx_publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char rx_secretkey[crypto_box_SECRETKEYBYTES];
    FILE *fp;

    crypto_box_keypair(tx_publickey, tx_secretkey);
    crypto_box_keypair(rx_publickey, rx_secretkey);

    if((fp = fopen("tx.key", "w")) == NULL)
    {
        perror("Unable to save tx.key");
        return 1;
    }

    fwrite(tx_secretkey, crypto_box_SECRETKEYBYTES, 1, fp);
    fwrite(rx_publickey, crypto_box_PUBLICKEYBYTES, 1, fp);
    fclose(fp);

    if((fp = fopen("rx.key", "w")) == NULL)
    {
        perror("Unable to save rx.key");
        return 1;
    }

    fwrite(rx_secretkey, crypto_box_SECRETKEYBYTES, 1, fp);
    fwrite(tx_publickey, crypto_box_PUBLICKEYBYTES, 1, fp);
    fclose(fp);

    return 0;
}
