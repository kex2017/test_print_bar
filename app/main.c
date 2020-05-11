/*
 * Copyright (C) 2014 Philipp Rosenkranz
 * Copyright (C) 2014 Nico von Geyso
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "tests-crypto.h"

int test(void)
{
    TESTS_START();
    TESTS_RUN(tests_crypto_helper_tests());
    TESTS_RUN(tests_crypto_chacha_tests());
    TESTS_RUN(tests_crypto_poly1305_tests());
    TESTS_RUN(tests_crypto_chacha20poly1305_tests());
    TESTS_RUN(tests_crypto_aes_tests());
    TESTS_RUN(tests_crypto_cipher_tests());
    TESTS_RUN(tests_crypto_modes_ccm_tests());
    TESTS_RUN(tests_crypto_modes_ocb_tests());
    TESTS_RUN(tests_crypto_modes_ecb_tests());
    TESTS_RUN(tests_crypto_modes_cbc_tests());
    TESTS_RUN(tests_crypto_modes_ctr_tests());
    TESTS_END();
    return 0;
}

#include "crypto/aes.h"

static uint8_t TEST_0_KEY[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

static uint8_t TEST_0_INP[] = {
    0x8,
    0x9,
    0xA,
    0xB,
    0xC,
    0xD,
    0xE,
    0xF,
    0x0,
    0x1,
    0x2,
    0x3,
    0x4,
    0x5,
    0x6,
    0x7,
};

#include "xtimer.h"

#include "progress_bar.h"

int main(void)
{
    cipher_context_t ctx;

    int err;
    uint8_t data_encrypted[AES_BLOCK_SIZE] = {0};
    uint8_t data_decrypted[AES_BLOCK_SIZE] = {0};

    err = aes_init(&ctx, TEST_0_KEY, sizeof(TEST_0_KEY));

    err = aes_encrypt(&ctx, TEST_0_INP, data_encrypted);

    printf("after encrypt err is %d data is :\r\n", err);
    for (int i = 0; i < AES_BLOCK_SIZE; i++)
    {
        printf("%02x ", data_encrypted[i]);
    }
    printf("\r\n");

    err = aes_decrypt(&ctx, data_encrypted, data_decrypted);

    printf("after decrypt err is %d data is :\r\n", err);
    for (int i = 0; i < AES_BLOCK_SIZE; i++)
    {
        printf("%02x ", data_decrypted[i]);
    }
    printf("\r\n");

    char *name = "xk";
    printf("|id\t|name\t|age\t|\r\n");
    for (int i = 1; i < 100; i += 1)
    {
        printf("\r|%d\t|%s\t|%d\t|\n", i,name, 10 + i);
        if (i % 5 == 0)
        {
            printf("\033[5A");
        }

        xtimer_sleep(1);
    }

    // char *prefix = "1";

    // char *suffix = "l";
    // progress_bar_print(prefix, suffix, 3);

    return 0;
}
