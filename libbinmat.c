
#include "libbinmat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const binmat_data_t one = (binmat_data_t)1U;
const binmat_data_t zero = (binmat_data_t)0U;

#if defined(BINMAT_DEBUG)
//static int binmat_do_debug = 1;
//static int binmat_do_debug = 0;
//static int binmat_do_debug = (getenv("BINMAT_DEBUG") != NULL);
static int binmat_do_debug = 2;
static const char *binmat_debugfname = "binmatdebug.txt";
static FILE *binmat_debugf = NULL;

int binmat_dprintf(const char *format, ...) {
	int rc = 0;
	if (binmat_do_debug == 2) {
		binmat_do_debug = (getenv("BINMAT_DEBUG") != NULL);
	}
	if (binmat_do_debug) {
		if (NULL == binmat_debugf) {
			binmat_debugf = fopen(binmat_debugfname, "w");
			if (NULL == binmat_debugf) {
				fprintf(stderr, "binmat_dprintf: Unable to open debug output file!\n");
				fflush(stderr);
			} else {
				fprintf(binmat_debugf, "binmat_dprintf: Opened file\n");
				fflush(binmat_debugf);
			}
		}
		if (NULL != binmat_debugf) {
			va_list ap;
			va_start(ap, format);
			rc = vfprintf(binmat_debugf, format, ap);
			va_end(ap);
			fflush(binmat_debugf);
		}
	}
	return rc;
}
#endif


binmat_data_t *binmat_alloc(binmat_index_t n) {
	binmat_data_t *m = NULL;
	binmat_dprintf("binmat_alloc: Allocating matrix size %lu bits (nmemb = %lu, size = %lu)... ", n, n * binmat_numchunks(n), binmat_chunkbytes);
	//m = malloc(binmat_numbytes(n));
	// Just in case (particularly when n is not a multiple of binmat_chunkbits)
	m = calloc(n * binmat_numchunks(n), binmat_chunkbytes);
	binmat_dprintf("at %p\n", m);
	return m;
}

void binmat_free(binmat_data_t *m) {
	binmat_dprintf("binmat_free: Freeing matrix at %p\n", m);
	free(m);
}

binmat_bool_t binmat_getbit(const binmat_data_t *m, binmat_index_t n, binmat_index_t row, binmat_index_t col) {
	binmat_index_t bitnum = col % binmat_chunkbits;

	//binmat_data_t mask = (((binmat_data_t)1U) << bitnum);
	//return (m[row * n + col / binmat_chunkbits] & mask) ? 1 : 0;

	//binmat_dprintf("binmat_getbit: matrix %p (n=%lu): (%lu,%lu), index %lu, bitnum %lu\n", m, n, row, col, row * binmat_numchunks(n) + col / binmat_chunkbits, bitnum);

	//return (m[row * binmat_numchunks(n) + col / binmat_chunkbits] & (((binmat_data_t)1U) << bitnum)) >> bitnum;
	return (m[row * binmat_numchunks(n) + col / binmat_chunkbits] & (binmat_data_t)(one << bitnum)) >> bitnum;
}

void binmat_setbit(binmat_data_t *m, binmat_index_t n, binmat_index_t row, binmat_index_t col, binmat_bool_t value) {
	binmat_index_t bitnum = col % binmat_chunkbits;
	//binmat_data_t mask = (((binmat_data_t)1U) << bitnum);
	binmat_data_t mask = (one << bitnum);

	//binmat_dprintf("binmat_setbit: matrix %p (n=%lu): (%lu,%lu), index %lu, bitnum %lu\n", m, n, row, col, row * binmat_numchunks(n) + col / binmat_chunkbits, bitnum);

	if (value) {
		m[row * binmat_numchunks(n) + col / binmat_chunkbits] |= mask;
	} else {
		m[row * binmat_numchunks(n) + col / binmat_chunkbits] &= ~ mask;
	}
}


char *binmat_format_chunk(binmat_data_t x, char *buf) {
	binmat_data_t mask = 1U;
	//binmat_data_t mask = 1;  // this *ought* to work
    binmat_index_t i;

    for (i = 0; i < binmat_chunkbits; i++, mask <<= 1) {
        //buf[i] = (x & mask) ? '1' : '0';
        buf[i] = ((x & mask) >> i) + '0';
    }
	buf[i] = '\0';

    return buf;
}


void binmat_print_matrix_slow(FILE *f, const binmat_data_t *m, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			fprintf(f, "%d", binmat_getbit(m, n, row, col));
		}
		fprintf(f, "\n");
	}
}

void binmat_print_matrix_fast(FILE *f, const binmat_data_t *m, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t k;
	binmat_index_t offset;
	binmat_binary_buf_t(buf);

	for (row = 0, offset = 0; row < n; row++, offset += binmat_numchunks(n)) {
		for (k = 0; k < binmat_numchunks(n); k++) {
			fprintf(f, "%s", binmat_format_chunk(m[offset + k], buf));
		}
		fprintf(f, "\n");
	}
}

void binmat_print_matrix_hex(FILE *f, const binmat_data_t *m, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t k;
	binmat_index_t offset;
	char fmt[16];

	//snprintf(fmt, sizeof(fmt) - 1, "0x%%0%lux ", binmat_chunkbytes);
	strcpy(fmt, "0x%x");

	for (row = 0, offset = 0; row < n; row++, offset += binmat_numchunks(n)) {
		for (k = 0; k < binmat_numchunks(n); k++) {
			fprintf(f, fmt, m[offset + k]);
		}
		fprintf(f, "\n");
	}
}

void binmat_transpose(binmat_data_t *output, const binmat_data_t *input, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;
	binmat_data_t *input_copy = NULL;
	const binmat_data_t *real_input;

	binmat_dprintf("binmat_transpose: Transposing matrix at %p, result at %p, size %lu\n", input, output, n);
	if (input == output) {
		binmat_dprintf("binmat_transpose: In place transposition, using temporary matrix\n");
		input_copy = binmat_alloc(n);
		binmat_copy(input_copy, input, n);
		real_input = input_copy;
	} else {
		real_input = input;
	}

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			binmat_setbit(output, n, col, row, binmat_getbit(real_input, n, row, col));
		}
	}

	binmat_dprintf("binmat_transpose: Freeing temporary matrix\n");
	binmat_free(input_copy);
}

int binmat_are_identical(const binmat_data_t *a, const binmat_data_t *b, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;

	binmat_dprintf("binmat_are_identical: Checking matrices at %p and %p, size %lu\n", a, b, n);
	for (row = 0; row < n; row++) {
		for (col = 0; col < binmat_numchunks(n) - 1; col++) {
			if (a[row * binmat_numchunks(n) + col] != b[row * binmat_numchunks(n) + col]) {
				//printf("ding1\n");
				return 0;
			}
		}
		if ((a[row * binmat_numchunks(n) + col] & binmat_finalchunkmask(n)) != (b[row * binmat_numchunks(n) + col] & binmat_finalchunkmask(n))) {
			//printf("ding2\n");
			return 0;
		}
	}
	return 1;
}

void binmat_copy(binmat_data_t *a, const binmat_data_t *b, binmat_index_t n) {
	binmat_dprintf("binmat_copy: Copying matrix at %p to %p, size %lu\n", b, a, n);
	memcpy(a, b, n * binmat_numchunks(n) * binmat_chunkbytes);
}

void binmat_multiply_slow(binmat_data_t *output, const binmat_data_t *a, const binmat_data_t *b, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;
	binmat_index_t k;

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			binmat_data_t temp = 0;
			for (k = 0; k < n; k++) {
				temp += binmat_getbit(a, n, row, k) * binmat_getbit(b, n, k, col);
			}
			binmat_setbit(output, n, row, col, temp ? 1 : 0);
		}
	}
}

// b MUST BE PRE-TRANSPOSED
void binmat_multiply(binmat_data_t *output, const binmat_data_t *a, const binmat_data_t *b, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;
	binmat_index_t k;
	binmat_index_t arowoffset;
	binmat_index_t bcoloffset;
	binmat_index_t colchunk;
	binmat_index_t chunkbit;
	binmat_data_t result;
	binmat_data_t r;
	//binmat_data_t r2;
	binmat_data_t this;
	binmat_data_t mask;
	binmat_binary_buf_t(s_result);
	binmat_binary_buf_t(s_r);
	binmat_binary_buf_t(s_r2);
	binmat_binary_buf_t(s_this);
	binmat_binary_buf_t(s_mask);

	binmat_dprintf("binmat_multiply: output = %p, a = %p, b = %p, n = %lu\n", output, a, b, n);

	// is this necessary......?
	for (k = 0; k < n*binmat_numchunks(n); k++) {
		output[k] = 0;
	}

	binmat_dprintf("binmat_multiply: Starting main loop from row = %lu to %lu, arowoffset = %lu (increment %lu)\n", 0, n, 0, binmat_numchunks(n));
	for (row = 0, arowoffset = 0; row < n; row++, arowoffset += binmat_numchunks(n)) {
		// Process a row.
		binmat_dprintf("binmat_multiply:   row = %lu, arowoffset = %lu\n", row, arowoffset);
		col = 0;
		bcoloffset = 0;
		binmat_dprintf("binmat_multiply:   col = %lu, bcoloffset = %lu\n", col, bcoloffset);
		binmat_dprintf("binmat_multiply:   looping from colchunk = %lu to %lu\n", 0, binmat_numchunks(n));
		for (colchunk = 0; colchunk < binmat_numchunks(n); colchunk++) {
			// Process a "chunk" of columns.  The number of columns in the "chunk" is chunkbits.
			// This is because each column processed is 1 bit of the result, and we need chunkbits results
			// stored in result before we can store it in the output array.
			binmat_dprintf("binmat_multiply:     colchunk = %lu, bcoloffset = %lu\n", colchunk, bcoloffset);
			result = 0;
			//printf("result = %s\n", binmat_format_chunk(result, s_result));
			//for (chunkbit = 0; chunkbit < binmat_chunkbits; chunkbit++, bcoloffset++) {
			binmat_dprintf("binmat_multiply:     looping from chunkbit = %lu to %lu, bcoloffset increment %lu\n", 0, binmat_chunkbits, binmat_numchunks(n));
			for (chunkbit = 0; chunkbit < binmat_chunkbits && col < n; chunkbit++, col++, bcoloffset += binmat_numchunks(n)) {
			//for (chunkbit = 0; chunkbit < binmat_chunkbits; chunkbit++, col++, bcoloffset += binmat_numchunks(n)) {
				// Process an actual column.
				binmat_dprintf("binmat_multiply:       chunkbit = %lu, col = %lu, bcoloffset = %lu\n", chunkbit, col, bcoloffset);

				r = 0;
				//printf("r = %s\n", binmat_format_chunk(r, s_r));
				binmat_dprintf("binmat_multiply:       looping from k = %lu to %lu\n", 0, binmat_numchunks(n));
				for (k = 0; k < binmat_numchunks(n) - 1; k++) {
					binmat_dprintf("binmat_multiply:         k = %lu, a index = %lu, b index = %lu\n", k, arowoffset + k, bcoloffset + k);
					this = a[arowoffset + k] & b[bcoloffset + k];
					//printf("  %s\n", binmat_format_chunk(a[arowoffset + k], s_r));
					//printf("& %s\n", binmat_format_chunk(b[bcoloffset + k], s_r));
					//printf("= %s\n", binmat_format_chunk(this, s_r));
					r |= a[arowoffset + k] & b[bcoloffset + k];
					//printf("r = %s\n", binmat_format_chunk(r, s_r));
				}
				binmat_dprintf("binmat_multiply:         k = %lu, a index = %lu, b index = %lu\n", k, arowoffset + k, bcoloffset + k);
				this = a[arowoffset + k] & b[bcoloffset + k] & binmat_finalchunkmask(n);
				r |= a[arowoffset + k] & b[bcoloffset + k] & binmat_finalchunkmask(n);
				binmat_dprintf("binmat_multiply:       done looping\n");

				// now OR all of the bits in r.

				/*
				r2 = 0;
				//mask = ((TYPE)1u);
				mask = 1U;
				//printf("r2 = %s, mask = %s\n", binmat_format_chunk(r2, s_r2), binmat_format_chunk(mask, s_mask));
				for (k = 0; k < binmat_chunkbits; k++, mask <<= 1) {
					this = (r & mask);
					r2 |= (r & mask) >> k;
					////printf("r2 = %s, mask = %s, this = %s\n", binmat_format_chunk(r2, s_r2), binmat_format_chunk(mask, s_mask), binmat_format_chunk(this, s_this));
				}
				*/

				//printf("r2 = %s, mask = %s\n", binmat_format_chunk(r2, s_r2), binmat_format_chunk(mask, s_mask));
				//r2 = r;
				this = r;
				for (k = 0; k < binmat_chunkbits; k++) {
					this >>= 1;
					r |= this;
					////printf("r2 = %s, mask = %s, this = %s\n", binmat_format_chunk(r2, s_r2), binmat_format_chunk(mask, s_mask), binmat_format_chunk(this, s_this));
				}
				//r2 <<= (binmat_chunkbits - 1);
				//r2 >>= (binmat_chunkbits - 1);
				//r2 &= 1U;
				r &= 1U;

				// now store that result in the chunkbit'th bit of result
				//r2 <<= chunkbit;
				r <<= chunkbit;
				//printf("r2 = %s\n", binmat_format_chunk(r2, s_r2));
				//result |= r2;
				result |= r;
				//printf("result = %s\n", binmat_format_chunk(result, s_result));
			}
			binmat_dprintf("binmat_multiply:     done looping\n");
			//printf("storing result\n");
			binmat_dprintf("binmat_multiply:     storing result in output index %lu\n", arowoffset + colchunk);
			// result now holds all the results for these chunkbits bits, store it
			output[arowoffset + colchunk] = result;
			//print_matrix(output);
		}
		binmat_dprintf("binmat_multiply:   done looping\n");
	}
	binmat_dprintf("binmat_multiply: DONE\n");
}

void binmat_power(binmat_data_t *output, const binmat_data_t *a, const binmat_data_t *trans, binmat_index_t n, unsigned int pow) {
	binmat_index_t i;
	//binmat_data_t *temp = binmat_alloc(n);
	//binmat_data_t *temp[2] = { binmat_alloc(n), binmat_alloc(n) };
	//binmat_data_t *temp[2] = { output, binmat_alloc(n) };
	binmat_data_t *temp[2];
	unsigned int which = 1 - pow % 2;
	unsigned int other = 1 - which;

	binmat_dprintf("binmat_power: output = %p, a = %p, trans = %p, n = %lu, pow = %u\n", output, a, trans, n, pow);
	if (pow == 0) {
		// FIXME: identity, yawn
		binmat_dprintf("binmat_power: identity, yawn\n");
	} else if (pow == 1) {
		// just copy it
		binmat_dprintf("binmat_power: just copy it\n");
		binmat_copy(output, a, n);
	} else {
		binmat_dprintf("binmat_power: which = %u, other = %u\n", which, other);
		temp[0] = output;
		temp[1] = binmat_alloc(n);
		binmat_dprintf("binmat_power: temp[0] = %p\n", temp[0]);
		binmat_dprintf("binmat_power: temp[1] = %p\n", temp[1]);
		//binmat_multiply(output, a, trans, n);
		binmat_multiply(temp[other], a, trans, n);
		binmat_dprintf("binmat_power: Entering main loop...\n");
		for (i = 2; i < pow; i++) {
			binmat_dprintf("binmat_power: i = %u, which = %u, other = %u\n", i, which, other);
			//binmat_multiply(temp[which], output, trans, n);
			//binmat_copy(output, temp[which], n);
			binmat_multiply(temp[which], temp[other], trans, n);
			other = which;
			which = 1 - which;
		}
		binmat_dprintf("binmat_power: Done main loop, freeing temp matrix.\n");
		//free(temp[0]);
		free(temp[1]);
	}
	binmat_dprintf("binmat_power: Done.\n");
}

void power_slow(binmat_data_t *output, const binmat_data_t *a, binmat_index_t n, unsigned int pow) {
	unsigned int i;
	//TYPE temp[N*n];
	binmat_data_t *temp = binmat_alloc(n);

	if (pow == 0) {
		// identity, yawn
	} else if (pow == 1) {
		// just copy it
		binmat_copy(output, a, n);
	} else {
		binmat_multiply_slow(output, a, a, n);
		for (i = 2; i < pow; i++) {
			binmat_multiply_slow(temp, output, a, n);
			binmat_copy(output, temp, n);
		}
	}
	free(temp);
}

