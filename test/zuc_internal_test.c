/*
 * Copyright 2022 The Tongsuo Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/Tongsuo-Project/Tongsuo/blob/master/LICENSE.txt
 */

/*
 * Tests for ZUC keystream generation.
 *
 * All tests come from 3GPP specification - Specification of the 3GPP
 * Confidentiality and Integrity Algorithms 128-EEA3 & 128-EIA3 Document 3:
 * Implementor’s Test Data
 */

#include <string.h>
#include <openssl/opensslconf.h>
#include "testutil.h"

#ifndef OPENSSL_NO_ZUC
# include "internal/nelem.h"
# include "crypto/zuc.h"
# include "../crypto/eia3/eia3_local.h"

typedef struct zuc_test_st {
    uint8_t key[16];
    uint8_t iv[16];
    uint32_t L;
    uint32_t output[3]; /* store the 2000th output in output[2] */
} ZUC_TV;

typedef struct eia3_test_st {
    uint8_t key[16];
    uint8_t iv[16];
    size_t input_len;
    unsigned char input[1024];
    uint32_t expected;
} EIA3_TV;

/*
 * All test vectors listed here are defined in:
 * "Specification of the 3GPP Confidentiality and Integrity Algorithms
 * 128-EEA3 & 128-EIA3
 * Document 3: Implementor’s Test Data" Section 3.
 *
 * Note: we only include test vectors of ZUC, the keystream generator only.
 * The 128-EEA3 cipher's test vectors are not included here. That part is
 * placed in EVP's test case.
 */
static ZUC_TV ztv[4] = {

    /* Test 1 */
    {
        /* Key */
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        /* IV */
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        /* L */
        2,

        /* output */
        { 0x27bede74, 0x018082da, 0x0 }
    },

    /* Test 2 */
    {
        /* Key */
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        },

        /* IV */
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        },

        /* L */
        2,

        /* output */
        { 0x0657cfa0, 0x7096398b, 0x0 }
    },

    /* Test 3 */
    {
        /* Key */
        {
            0x3d, 0x4c, 0x4b, 0xe9, 0x6a, 0x82, 0xfd, 0xae,
            0xb5, 0x8f, 0x64, 0x1d, 0xb1, 0x7b, 0x45, 0x5b
        },

        /* IV */
        {
            0x84, 0x31, 0x9a, 0xa8, 0xde, 0x69, 0x15, 0xca,
            0x1f, 0x6b, 0xda, 0x6b, 0xfb, 0xd8, 0xc7, 0x66
        },

        /* L */
        2,

        /* output */
        { 0x14f1c272, 0x3279c419, 0x0 }
    },

    /* Test 4 */
    {
        /* Key */
        {
            0x4d, 0x32, 0x0b, 0xfa, 0xd4, 0xc2, 0x85, 0xbf,
            0xd6, 0xb8, 0xbd, 0x00, 0xf3, 0x9d, 0x8b, 0x41
        },

        /* IV */
        {
            0x52, 0x95, 0x9d, 0xab, 0xa0, 0xbf, 0x17, 0x6e,
            0xce, 0x2d, 0xc3, 0x15, 0x04, 0x9e, 0xb5, 0x74
        },

        /* L */
        2000,

        /* output */
        { 0xed4400e7, 0x0633e5c5, 0x7a574cdb }
    }
};

static EIA3_TV eia3_tv[5] = {

    /* Test 1 */
    {
        /* Key */
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        /* IV */
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        /* input_len */
        1,

        /* input */
        {
            0x00,
        },

        /* expected */
        0x390a91b7
    },

    /* Test 2 */
    {
        /* Key */
        {
            0x47, 0x05, 0x41, 0x25, 0x56, 0x1e, 0xb2, 0xdd,
            0xa9, 0x40, 0x59, 0xda, 0x05, 0x09, 0x78, 0x50
        },

        /* IV */
        {
            0x56, 0x1e, 0xb2, 0xdd, 0xa0, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        /* input_len */
        12,

        /* input */
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        },

        /* expected */
        0x89a58b47
    },

    /* Test 3 */
    {
        /* Key */
        {
            0xc9, 0xe6, 0xce, 0xc4, 0x60, 0x7c, 0x72, 0xdb,
            0x00, 0x0a, 0xef, 0xa8, 0x83, 0x85, 0xab, 0x0a
        },

        /* IV */
        {
            0xa9, 0x40, 0x59, 0xda, 0x54, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        /* input_len */
        73,

        /* input */
        {
            0x98, 0x3b, 0x41, 0xd4, 0x7d, 0x78, 0x0c, 0x9e,
            0x1a, 0xd1, 0x1d, 0x7e, 0xb7, 0x03, 0x91, 0xb1,
            0xde, 0x0b, 0x35, 0xda, 0x2d, 0xc6, 0x2f, 0x83,
            0xe7, 0xb7, 0x8d, 0x63, 0x06, 0xca, 0x0e, 0xa0,
            0x7e, 0x94, 0x1b, 0x7b, 0xe9, 0x13, 0x48, 0xf9,
            0xfc, 0xb1, 0x70, 0xe2, 0x21, 0x7f, 0xec, 0xd9,
            0x7f, 0x9f, 0x68, 0xad, 0xb1, 0x6e, 0x5d, 0x7d,
            0x21, 0xe5, 0x69, 0xd2, 0x80, 0xed, 0x77, 0x5c,
            0xeb, 0xde, 0x3f, 0x40, 0x93, 0xc5, 0x38, 0x81,
            0x00
        },

        /* expected */
        0x24a842b3
    },

    /* Test 4 */
    {
        /* Key */
        {
            0xc8, 0xa4, 0x82, 0x62, 0xd0, 0xc2, 0xe2, 0xba,
            0xc4, 0xb9, 0x6e, 0xf7, 0x7e, 0x80, 0xca, 0x59
        },

        /* IV */
        {
            0x05, 0x09, 0x78, 0x50, 0x84, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        /* input_len */
        260,

        /* input */
        {
            0xb5, 0x46, 0x43, 0x0b, 0xf8, 0x7b, 0x4f, 0x1e,
            0xe8, 0x34, 0x70, 0x4c, 0xd6, 0x95, 0x1c, 0x36,
            0xe2, 0x6f, 0x10, 0x8c, 0xf7, 0x31, 0x78, 0x8f,
            0x48, 0xdc, 0x34, 0xf1, 0x67, 0x8c, 0x05, 0x22,
            0x1c, 0x8f, 0xa7, 0xff, 0x2f, 0x39, 0xf4, 0x77,
            0xe7, 0xe4, 0x9e, 0xf6, 0x0a, 0x4e, 0xc2, 0xc3,
            0xde, 0x24, 0x31, 0x2a, 0x96, 0xaa, 0x26, 0xe1,
            0xcf, 0xba, 0x57, 0x56, 0x38, 0x38, 0xb2, 0x97,
            0xf4, 0x7e, 0x85, 0x10, 0xc7, 0x79, 0xfd, 0x66,
            0x54, 0xb1, 0x43, 0x38, 0x6f, 0xa6, 0x39, 0xd3,
            0x1e, 0xdb, 0xd6, 0xc0, 0x6e, 0x47, 0xd1, 0x59,
            0xd9, 0x43, 0x62, 0xf2, 0x6a, 0xee, 0xed, 0xee,
            0x0e, 0x4f, 0x49, 0xd9, 0xbf, 0x84, 0x12, 0x99,
            0x54, 0x15, 0xbf, 0xad, 0x56, 0xee, 0x82, 0xd1,
            0xca, 0x74, 0x63, 0xab, 0xf0, 0x85, 0xb0, 0x82,
            0xb0, 0x99, 0x04, 0xd6, 0xd9, 0x90, 0xd4, 0x3c,
            0xf2, 0xe0, 0x62, 0xf4, 0x08, 0x39, 0xd9, 0x32,
            0x48, 0xb1, 0xeb, 0x92, 0xcd, 0xfe, 0xd5, 0x30,
            0x0b, 0xc1, 0x48, 0x28, 0x04, 0x30, 0xb6, 0xd0,
            0xca, 0xa0, 0x94, 0xb6, 0xec, 0x89, 0x11, 0xab,
            0x7d, 0xc3, 0x68, 0x24, 0xb8, 0x24, 0xdc, 0x0a,
            0xf6, 0x68, 0x2b, 0x09, 0x35, 0xfd, 0xe7, 0xb4,
            0x92, 0xa1, 0x4d, 0xc2, 0xf4, 0x36, 0x48, 0x03,
            0x8d, 0xa2, 0xcf, 0x79, 0x17, 0x0d, 0x2d, 0x50,
            0x13, 0x3f, 0xd4, 0x94, 0x16, 0xcb, 0x6e, 0x33,
            0xbe, 0xa9, 0x0b, 0x8b, 0xf4, 0x55, 0x9b, 0x03,
            0x73, 0x2a, 0x01, 0xea, 0x29, 0x0e, 0x6d, 0x07,
            0x4f, 0x79, 0xbb, 0x83, 0xc1, 0x0e, 0x58, 0x00,
            0x15, 0xcc, 0x1a, 0x85, 0xb3, 0x6b, 0x55, 0x01,
            0x04, 0x6e, 0x9c, 0x4b, 0xdc, 0xae, 0x51, 0x35,
            0x69, 0x0b, 0x86, 0x66, 0xbd, 0x54, 0xb7, 0xa7,
            0x03, 0xea, 0x7b, 0x6f, 0x22, 0x0a, 0x54, 0x69,
            0xa5, 0x68, 0x02, 0x7e
        },

        /* expected */
        0x039532e1
    },

    /* Test 5 */
    {
        /* Key */
        {
            0x6b, 0x8b, 0x08, 0xee, 0x79, 0xe0, 0xb5, 0x98,
            0x2d, 0x6d, 0x12, 0x8e, 0xa9, 0xf2, 0x20, 0xcb
        },

        /* IV */
        {
            0x56, 0x1e, 0xb2, 0xdd, 0xe0, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        },

        /* input_len */
        709,

        /* input */
        {
            0x5b, 0xad, 0x72, 0x47, 0x10, 0xba, 0x1c, 0x56,
            0xd5, 0xa3, 0x15, 0xf8, 0xd4, 0x0f, 0x6e, 0x09,
            0x37, 0x80, 0xbe, 0x8e, 0x8d, 0xe0, 0x7b, 0x69,
            0x92, 0x43, 0x20, 0x18, 0xe0, 0x8e, 0xd9, 0x6a,
            0x57, 0x34, 0xaf, 0x8b, 0xad, 0x8a, 0x57, 0x5d,
            0x3a, 0x1f, 0x16, 0x2f, 0x85, 0x04, 0x5c, 0xc7,
            0x70, 0x92, 0x55, 0x71, 0xd9, 0xf5, 0xb9, 0x4e,
            0x45, 0x4a, 0x77, 0xc1, 0x6e, 0x72, 0x93, 0x6b,
            0xf0, 0x16, 0xae, 0x15, 0x74, 0x99, 0xf0, 0x54,
            0x3b, 0x5d, 0x52, 0xca, 0xa6, 0xdb, 0xea, 0xb6,
            0x97, 0xd2, 0xbb, 0x73, 0xe4, 0x1b, 0x80, 0x75,
            0xdc, 0xe7, 0x9b, 0x4b, 0x86, 0x04, 0x4f, 0x66,
            0x1d, 0x44, 0x85, 0xa5, 0x43, 0xdd, 0x78, 0x60,
            0x6e, 0x04, 0x19, 0xe8, 0x05, 0x98, 0x59, 0xd3,
            0xcb, 0x2b, 0x67, 0xce, 0x09, 0x77, 0x60, 0x3f,
            0x81, 0xff, 0x83, 0x9e, 0x33, 0x18, 0x59, 0x54,
            0x4c, 0xfb, 0xc8, 0xd0, 0x0f, 0xef, 0x1a, 0x4c,
            0x85, 0x10, 0xfb, 0x54, 0x7d, 0x6b, 0x06, 0xc6,
            0x11, 0xef, 0x44, 0xf1, 0xbc, 0xe1, 0x07, 0xcf,
            0xa4, 0x5a, 0x06, 0xaa, 0xb3, 0x60, 0x15, 0x2b,
            0x28, 0xdc, 0x1e, 0xbe, 0x6f, 0x7f, 0xe0, 0x9b,
            0x05, 0x16, 0xf9, 0xa5, 0xb0, 0x2a, 0x1b, 0xd8,
            0x4b, 0xb0, 0x18, 0x1e, 0x2e, 0x89, 0xe1, 0x9b,
            0xd8, 0x12, 0x59, 0x30, 0xd1, 0x78, 0x68, 0x2f,
            0x38, 0x62, 0xdc, 0x51, 0xb6, 0x36, 0xf0, 0x4e,
            0x72, 0x0c, 0x47, 0xc3, 0xce, 0x51, 0xad, 0x70,
            0xd9, 0x4b, 0x9b, 0x22, 0x55, 0xfb, 0xae, 0x90,
            0x65, 0x49, 0xf4, 0x99, 0xf8, 0xc6, 0xd3, 0x99,
            0x47, 0xed, 0x5e, 0x5d, 0xf8, 0xe2, 0xde, 0xf1,
            0x13, 0x25, 0x3e, 0x7b, 0x08, 0xd0, 0xa7, 0x6b,
            0x6b, 0xfc, 0x68, 0xc8, 0x12, 0xf3, 0x75, 0xc7,
            0x9b, 0x8f, 0xe5, 0xfd, 0x85, 0x97, 0x6a, 0xa6,
            0xd4, 0x6b, 0x4a, 0x23, 0x39, 0xd8, 0xae, 0x51,
            0x47, 0xf6, 0x80, 0xfb, 0xe7, 0x0f, 0x97, 0x8b,
            0x38, 0xef, 0xfd, 0x7b, 0x2f, 0x78, 0x66, 0xa2,
            0x25, 0x54, 0xe1, 0x93, 0xa9, 0x4e, 0x98, 0xa6,
            0x8b, 0x74, 0xbd, 0x25, 0xbb, 0x2b, 0x3f, 0x5f,
            0xb0, 0xa5, 0xfd, 0x59, 0x88, 0x7f, 0x9a, 0xb6,
            0x81, 0x59, 0xb7, 0x17, 0x8d, 0x5b, 0x7b, 0x67,
            0x7c, 0xb5, 0x46, 0xbf, 0x41, 0xea, 0xdc, 0xa2,
            0x16, 0xfc, 0x10, 0x85, 0x01, 0x28, 0xf8, 0xbd,
            0xef, 0x5c, 0x8d, 0x89, 0xf9, 0x6a, 0xfa, 0x4f,
            0xa8, 0xb5, 0x48, 0x85, 0x56, 0x5e, 0xd8, 0x38,
            0xa9, 0x50, 0xfe, 0xe5, 0xf1, 0xc3, 0xb0, 0xa4,
            0xf6, 0xfb, 0x71, 0xe5, 0x4d, 0xfd, 0x16, 0x9e,
            0x82, 0xce, 0xcc, 0x72, 0x66, 0xc8, 0x50, 0xe6,
            0x7c, 0x5e, 0xf0, 0xba, 0x96, 0x0f, 0x52, 0x14,
            0x06, 0x0e, 0x71, 0xeb, 0x17, 0x2a, 0x75, 0xfc,
            0x14, 0x86, 0x83, 0x5c, 0xbe, 0xa6, 0x53, 0x44,
            0x65, 0xb0, 0x55, 0xc9, 0x6a, 0x72, 0xe4, 0x10,
            0x52, 0x24, 0x18, 0x23, 0x25, 0xd8, 0x30, 0x41,
            0x4b, 0x40, 0x21, 0x4d, 0xaa, 0x80, 0x91, 0xd2,
            0xe0, 0xfb, 0x01, 0x0a, 0xe1, 0x5c, 0x6d, 0xe9,
            0x08, 0x50, 0x97, 0x3b, 0xdf, 0x1e, 0x42, 0x3b,
            0xe1, 0x48, 0xa2, 0x37, 0xb8, 0x7a, 0x0c, 0x9f,
            0x34, 0xd4, 0xb4, 0x76, 0x05, 0xb8, 0x03, 0xd7,
            0x43, 0xa8, 0x6a, 0x90, 0x39, 0x9a, 0x4a, 0xf3,
            0x96, 0xd3, 0xa1, 0x20, 0x0a, 0x62, 0xf3, 0xd9,
            0x50, 0x79, 0x62, 0xe8, 0xe5, 0xbe, 0xe6, 0xd3,
            0xda, 0x2b, 0xb3, 0xf7, 0x23, 0x76, 0x64, 0xac,
            0x7a, 0x29, 0x28, 0x23, 0x90, 0x0b, 0xc6, 0x35,
            0x03, 0xb2, 0x9e, 0x80, 0xd6, 0x3f, 0x60, 0x67,
            0xbf, 0x8e, 0x17, 0x16, 0xac, 0x25, 0xbe, 0xba,
            0x35, 0x0d, 0xeb, 0x62, 0xa9, 0x9f, 0xe0, 0x31,
            0x85, 0xeb, 0x4f, 0x69, 0x93, 0x7e, 0xcd, 0x38,
            0x79, 0x41, 0xfd, 0xa5, 0x44, 0xba, 0x67, 0xdb,
            0x09, 0x11, 0x77, 0x49, 0x38, 0xb0, 0x18, 0x27,
            0xbc, 0xc6, 0x9c, 0x92, 0xb3, 0xf7, 0x72, 0xa9,
            0xd2, 0x85, 0x9e, 0xf0, 0x03, 0x39, 0x8b, 0x1f,
            0x6b, 0xba, 0xd7, 0xb5, 0x74, 0xf7, 0x98, 0x9a,
            0x1d, 0x10, 0xb2, 0xdf, 0x79, 0x8e, 0x0d, 0xbf,
            0x30, 0xd6, 0x58, 0x74, 0x64, 0xd2, 0x48, 0x78,
            0xcd, 0x00, 0xc0, 0xea, 0xee, 0x8a, 0x1a, 0x0c,
            0xc7, 0x53, 0xa2, 0x79, 0x79, 0xe1, 0x1b, 0x41,
            0xdb, 0x1d, 0xe3, 0xd5, 0x03, 0x8a, 0xfa, 0xf4,
            0x9f, 0x5c, 0x68, 0x2c, 0x37, 0x48, 0xd8, 0xa3,
            0xa9, 0xec, 0x54, 0xe6, 0xa3, 0x71, 0x27, 0x5f,
            0x16, 0x83, 0x51, 0x0f, 0x8e, 0x4f, 0x90, 0x93,
            0x8f, 0x9a, 0xb6, 0xe1, 0x34, 0xc2, 0xcf, 0xdf,
            0x48, 0x41, 0xcb, 0xa8, 0x8e, 0x0c, 0xff, 0x2b,
            0x0b, 0xcc, 0x8e, 0x6a, 0xdc, 0xb7, 0x11, 0x09,
            0xb5, 0x19, 0x8f, 0xec, 0xf1, 0xbb, 0x7e, 0x5c,
            0x53, 0x1a, 0xca, 0x50, 0xa5, 0x6a, 0x8a, 0x3b,
            0x6d, 0xe5, 0x98, 0x62, 0xd4, 0x1f, 0xa1, 0x13,
            0xd9, 0xcd, 0x95, 0x78, 0x08, 0xf0, 0x85, 0x71,
            0xd9, 0xa4, 0xbb, 0x79, 0x2a, 0xf2, 0x71, 0xf6,
            0xcc, 0x6d, 0xbb, 0x8d, 0xc7, 0xec, 0x36, 0xe3,
            0x6b, 0xe1, 0xed, 0x30, 0x81, 0x64, 0xc3, 0x1c,
            0x7c, 0x0a, 0xfc, 0x54, 0x1c, 0x00, 0x00, 0x00
        },

        /* expected */
        0xfb9ab74c
    },
};

static int test_zuc(int idx)
{
    ZUC_KEY zk;
    int ret, i;
    uint32_t z;

    /* setup */
    memset(&zk, 0, sizeof(ZUC_KEY));
    zk.k = ztv[idx].key;
    memcpy(zk.iv, ztv[idx].iv, 16);
    zk.L = ztv[idx].L;

    ZUC_init(&zk);

    ret = ZUC_generate_keystream(&zk);
    if (!ret) {
        TEST_error("Fail to generate ZUC keystrean (round %d)", idx);
        return 0;
    }

    for (i = 0; i < 3; i++) {
        if (ztv[idx].output[i] != 0) {
            /*
             * in the spec, one test reads the last keystream byte
             */
            if (i == 2)
                z = (long)zk.keystream[7996] << 24 | zk.keystream[7997] << 16
                    | zk.keystream[7998] << 8 | zk.keystream[7999];
            else
                z = (long)zk.keystream[i * 4] << 24 | zk.keystream[i * 4 + 1] << 16
                    | zk.keystream[i * 4 + 2] << 8 | zk.keystream[i * 4 + 3];

            if (!TEST_uint_eq(z, ztv[idx].output[i])) {
                TEST_info("Current compared key: %d", i);
                return 0;
            }
        }
    }

    ZUC_destroy_keystream(&zk);

    return 1;
}

static int test_eia3(int idx)
{
    EIA3_CTX eia3_ctx;
    int i;
    size_t j;
    uint8_t out[4];
    uint32_t mac;

    for (i = 0; i < idx; i++) {
        EIA3_Init(&eia3_ctx, eia3_tv[i].key, eia3_tv[i].iv);
        EIA3_Update(&eia3_ctx, eia3_tv[i].input, eia3_tv[i].input_len);
        EIA3_Final(&eia3_ctx, out);

        mac = ((uint32_t)out[0] << 24) | ((uint32_t)out[1] << 16) | ((uint32_t)out[2] << 8) | (uint32_t)out[3];
        if (!TEST_uint_eq(mac, eia3_tv[i].expected)) {
            TEST_info("eia3 result: 0x%x, expected: 0x%x", mac, eia3_tv[i].expected);
            return 0;
        }

        if (eia3_tv[i].input_len > 128) {
            EIA3_Init(&eia3_ctx, eia3_tv[i].key, eia3_tv[i].iv);

            for (j = 0; j < eia3_tv[i].input_len; j += 128) {
                EIA3_Update(&eia3_ctx, &eia3_tv[i].input[j],
                            eia3_tv[i].input_len - j > 128 ? 128 : eia3_tv[i].input_len - j);
            }

            EIA3_Final(&eia3_ctx, out);

            mac = ((uint32_t)out[0] << 24) | ((uint32_t)out[1] << 16) | ((uint32_t)out[2] << 8) | (uint32_t)out[3];
            if (!TEST_uint_eq(mac, eia3_tv[i].expected)) {
                TEST_info("eia3 result: 0x%x, expected: 0x%x", mac, eia3_tv[i].expected);
                return 0;
            }
        }
    }

    return 1;
}

#endif

int setup_tests(void)
{
#ifndef OPENSSL_NO_ZUC
    ADD_ALL_TESTS(test_zuc, 4);
    ADD_ALL_TESTS(test_eia3, OSSL_NELEM(eia3_tv));
#endif
    return 1;
}
