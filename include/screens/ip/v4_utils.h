/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef V4_UTILS_H
#define V4_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "input.h"

typedef enum {
    V4_C_DIGIT,
    V4_C_DOT,
    V4_C_SLASH,
    V4_C_OTHER,
    V4_NUM_CLASSES,
} v4_class_t;

typedef enum {
    V4_S_START,
    V4_S_OCTDIGIT,
    V4_S_DOT,
    V4_S_SLASH,
    V4_S_MSKDIGIT,
    V4_NUM_STATES,
} v4_state_t;

typedef struct {
    uint32_t value;
    uint8_t octets[4];
    char string[TINY_BUFFER];
} v4_field_t;

typedef struct {
    v4_field_t base;
    uint8_t mask_prefix;
} v4_cidr_t;

bool v4_input_accept(char ch);
bool v4_validate(const char *input);

void uint32_to_octets(uint32_t v4_adr, uint8_t *octets_arr);
void v4_to_str(const uint8_t *octets_arr, char *str, size_t buf_size);

void v4_parse_cidr(const char *input, v4_cidr_t *out, char *err,
                   size_t err_size);
void parse_mask(const uint8_t mask_prefix, v4_field_t *mask);

void calc_v4_network_address(const uint32_t base_address,
                             const uint32_t subnet_mask, v4_field_t *network);
void calc_v4_broadcast_address(const uint32_t network_address,
                               const uint32_t subnet_mask,
                               v4_field_t *broadcast);

uint32_t calc_v4_capacity(const uint8_t mask_prefix, char *out_buf,
                          size_t buf_size);
void calc_v4_first_node(const uint8_t mask_prefix,
                        const uint32_t network_address, v4_field_t *first);
void calc_v4_last_node(const uint8_t mask_prefix, const uint32_t base_address,
                       const uint32_t broadcast_address, v4_field_t *last);

#endif /* V4_UTILS */
