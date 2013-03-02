
/*
 * Copyright (C) 2013 Kevin Pulo <kev@pulo.com.au>.
 *
 * This file is part of binmat.
 *
 * binmat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * binmat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with binmat.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBBINMAT_H
#define LIBBINMAT_H

#include <stdio.h>


#if defined(BINMAT_DATA_TYPE)
typedef BINMAT_DATA_TYPE binmat_data_t;
#else
#ifdef HAVE_UNSIGNED_LONG_LONG_INT
typedef unsigned long long binmat_data_t;
#define BINMAT_PRINTF_MODIFIER "ll"
#else
typedef unsigned long binmat_data_t;
#define BINMAT_PRINTF_MODIFIER "l"
#endif
/*typedef unsigned char binmat_data_t;*/
/*typedef unsigned int binmat_data_t;*/
#endif

extern const binmat_data_t one;
extern const binmat_data_t zero;

typedef unsigned int binmat_index_t;
typedef unsigned char binmat_bool_t;

#define binmat_chunkbytes (sizeof(binmat_data_t))
#define binmat_chunkbits (binmat_chunkbytes*8)
#define binmat_numchunks(n) ( ((n)%binmat_chunkbits==0) ? (n)/binmat_chunkbits : (n)/binmat_chunkbits + 1 )
#define binmat_numbytes(n) (binmat_numchunks(n) * binmat_chunkbytes)
#define binmat_numbits(n) (binmat_numchunks(n) * binmat_chunkbits)
#define binmat_numbytes_matrix(n) ((n) * binmat_numbytes(n))
#define binmat_finalchunkbits(n) ( ((n)%binmat_chunkbits==0) ? binmat_chunkbits : (n)%binmat_chunkbits )
#define binmat_finalchunkmask(n) ( ((n)%binmat_chunkbits==0) ? ((binmat_data_t)(zero - 1)) : ((one << binmat_finalchunkbits(n)) - 1) )

#define binmat_binary_bufn_t(x,n) char x[(n) * binmat_chunkbits + 1]
#define binmat_binary_buf_t(x) binmat_binary_bufn_t(x,1)


#if defined(BINMAT_DEBUG)
int binmat_dprintf(const char *format, ...);
#else
#define binmat_dprintf(...)
#endif


binmat_data_t *binmat_alloc(binmat_index_t n);
void binmat_free(binmat_data_t *m);

binmat_bool_t binmat_getbit(const binmat_data_t *m, binmat_index_t n, binmat_index_t row, binmat_index_t col);
void binmat_setbit(binmat_data_t *m, binmat_index_t n, binmat_index_t row, binmat_index_t col, binmat_bool_t value);

char *binmat_format_chunk(binmat_data_t x, char *buf);
void binmat_print_matrix_slow(FILE *f, const binmat_data_t *m, binmat_index_t n);
void binmat_print_matrix_fast(FILE *f, const binmat_data_t *m, binmat_index_t n);
void binmat_print_matrix_hex(FILE *f, const binmat_data_t *m, binmat_index_t n);

void binmat_transpose(binmat_data_t *output, const binmat_data_t *input, binmat_index_t n);

int binmat_are_identical(const binmat_data_t *a, const binmat_data_t *b, binmat_index_t n);

void binmat_copy(binmat_data_t *a, const binmat_data_t *b, binmat_index_t n);

void binmat_multiply_slow(binmat_data_t *output, const binmat_data_t *a, const binmat_data_t *b, binmat_index_t n);

/* b MUST BE PRE-TRANSPOSED */
void binmat_multiply(binmat_data_t *output, const binmat_data_t *a, const binmat_data_t *b, binmat_index_t n);

void binmat_power(binmat_data_t *output, const binmat_data_t *a, const binmat_data_t *trans, binmat_index_t n, unsigned int pow);
void binmat_power_slow(binmat_data_t *output, const binmat_data_t *a, binmat_index_t n, unsigned int pow);


#endif
