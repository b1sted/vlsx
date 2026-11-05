/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <inttypes.h>

#include "app.h"
#include "dfa.h"

#include "screens/ip/v4_utils.h"

static int v4_char_class(char ch);

static const int v4_trans[V4_NUM_STATES][V4_NUM_CLASSES] = {
    /*              DIGIT          DOT    SLASH  OTHER */
    [V4_S_START] = {V4_S_OCTDIGIT, S_ERR, S_ERR, S_ERR},
    [V4_S_OCTDIGIT] = {V4_S_OCTDIGIT, V4_S_DOT, V4_S_SLASH, S_ERR},
    [V4_S_DOT] = {V4_S_OCTDIGIT, S_ERR, S_ERR, S_ERR},
    [V4_S_SLASH] = {V4_S_MSKDIGIT, S_ERR, S_ERR, S_ERR},
    [V4_S_MSKDIGIT] = {V4_S_MSKDIGIT, S_ERR, S_ERR, S_ERR},
};

static const bool v4_finals[V4_NUM_STATES] = {
    [V4_S_MSKDIGIT] = true,
};

static const dfa_t v4_validator = {
    .num_states = V4_NUM_STATES,
    .num_classes = V4_NUM_CLASSES,
    .start_state = V4_S_START,
    .transitions = (const int *)v4_trans,
    .is_final = v4_finals,
    .get_class = v4_char_class,
};

static int v4_char_class(char ch) {
    if (ch >= '0' && ch <= '9')
        return V4_C_DIGIT;
    if (ch == '.')
        return V4_C_DOT;
    if (ch == '/')
        return V4_C_SLASH;
    return V4_C_OTHER;
}

bool v4_input_accept(char ch) {
    return v4_validator.get_class(ch) != V4_C_OTHER;
}

bool v4_validate(const char *input) { return run_dfa(&v4_validator, input); }

void uint32_to_octets(uint32_t v4_adr, uint8_t *octets_arr) {
    octets_arr[0] = (v4_adr >> 24) & 0xFF;
    octets_arr[1] = (v4_adr >> 16) & 0xFF;
    octets_arr[2] = (v4_adr >> 8) & 0xFF;
    octets_arr[3] = v4_adr;
}

void v4_to_str(const uint8_t *octets_arr, char *str, size_t buf_size) {
    snprintf(str, buf_size, "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8,
             octets_arr[0], octets_arr[1], octets_arr[2], octets_arr[3]);
}

void parse_mask(const uint8_t mask_prefix, v4_field_t *mask) {
    mask->value = (mask_prefix == 0) ? 0 : (0xFFFFFFFFu << (32 - mask_prefix));

    uint32_to_octets(mask->value, mask->octets);
    v4_to_str(mask->octets, mask->string, sizeof(mask->string));
}

void v4_parse_cidr(const char *input, v4_cidr_t *out, char *err,
                   size_t err_size) {
    if (!run_dfa(&v4_validator, input)) {
        strncpy(err, "Invalid input.\nPlease correct it and try again.",
                err_size);
        goto fail;
    }

    int octets[4], mask, n;

    if (sscanf(input, "%d.%d.%d.%d/%d%n", &octets[0], &octets[1], &octets[2],
               &octets[3], &mask, &n) != 5 ||
        input[n] != '\0') {
        snprintf(err, err_size,
                 "Invalid input.\nExpected format: A.B.C.D/mask");
        goto fail;
    }

    for (int i = 0; i < 4; i++) {
        if (octets[i] < 0 || octets[i] > 255) {
            snprintf(err, err_size,
                     "Invalid input.\nThe value of octet %d is out of range: "
                     "[0, 255]",
                     i + 1);
            goto fail;
        }

        out->base.value = (out->base.value << 8) | (uint8_t)octets[i];
        out->base.octets[i] = (uint8_t)octets[i];
    }

    if (mask < 0 || mask > 32) {
        snprintf(err, err_size,
                 "Invalid input.\nMask prefix %d is out of range: [0, 32]",
                 mask);
        goto fail;
    }

    out->mask_prefix = (uint8_t)mask;

    v4_to_str(out->base.octets, out->base.string, sizeof(out->base.string));

    app.is_input_correct = true;
    return;

fail:
    app.is_input_correct = false;
    return;
}

void calc_v4_network_address(const uint32_t base_address,
                             const uint32_t subnet_mask, v4_field_t *network) {
    if (subnet_mask == 0xFFFFFFFFu) {
        memcpy(network->string, "N/A (RFC 3021)", 15);
        return;
    }

    network->value = base_address & subnet_mask;

    uint32_to_octets(network->value, network->octets);
    v4_to_str(network->octets, network->string, sizeof(network->string));
}

void calc_v4_broadcast_address(const uint32_t network_address,
                               const uint32_t subnet_mask,
                               v4_field_t *broadcast) {
    if (subnet_mask == 0xFFFFFFFFu) {
        memcpy(broadcast->string, "N/A", 4);
        return;
    }

    if (subnet_mask == 0xFFFFFFFEu) {
        memcpy(broadcast->string, "N/A (RFC 3021)", 15);
        return;
    }

    broadcast->value = network_address | ~subnet_mask;

    uint32_to_octets(broadcast->value, broadcast->octets);
    v4_to_str(broadcast->octets, broadcast->string, sizeof(broadcast->string));
}

uint32_t calc_v4_capacity(const uint8_t mask_prefix, char *out_buf,
                          size_t buf_size) {
    uint32_t v4_node_count = 0;

    if (mask_prefix == 32)
        v4_node_count = 1;
    else if (mask_prefix == 31)
        v4_node_count = 2;
    else
        v4_node_count = (UINT32_C(1) << (32 - mask_prefix)) - 2;

    if (out_buf != NULL && buf_size > 0)
        snprintf(out_buf, buf_size, "%" PRIu32 "", v4_node_count);

    return v4_node_count;
}

void calc_v4_first_node(const uint8_t mask_prefix,
                        const uint32_t network_address, v4_field_t *first) {
    first->value = (mask_prefix == 32 || mask_prefix == 31)
                       ? network_address
                       : network_address + 1;

    uint32_to_octets(first->value, first->octets);
    v4_to_str(first->octets, first->string, sizeof(first->string));
}

void calc_v4_last_node(const uint8_t mask_prefix, const uint32_t base_address,
                       const uint32_t broadcast_address, v4_field_t *last) {
    last->value = (mask_prefix == 32)   ? base_address
                  : (mask_prefix == 31) ? base_address + 1
                                        : broadcast_address - 1;

    uint32_to_octets(last->value, last->octets);
    v4_to_str(last->octets, last->string, sizeof(last->string));
}
