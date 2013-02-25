
#include "libbinmat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#define BINMAT_TEST__BSD_SOURCE
#endif
#include <sys/time.h>
#ifdef BINMAT_TEST__BSD_SOURCE
#undef _BSD_SOURCE
#undef BINMAT_TEST__BSD_SOURCE
#endif


#if !defined(TRAD)
#define TRAD unsigned int
//#define TRAD unsigned char



void init_trad(TRAD *output, const binmat_data_t *input, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			output[row * n + col] = binmat_getbit(input, n, row, col);
		}
	}
}

void print_trad_binary(const TRAD *m, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			printf("%u", m[row * n + col] ? 1 : 0);
		}
		printf("\n");
	}
}

void print_trad(const TRAD *m, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;

	// FIXME: columns
	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			printf("%u", m[row * n + col]);
			if (col < n-1) {
				printf(",");
			}
		}
		printf("\n");
	}
}

int are_identical_trad(const binmat_data_t *a, const TRAD *b, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			if (binmat_getbit(a, n, row, col) != (b[row * n + col] ? 1 : 0)) {
				return 0;
			}
		}
	}
	return 1;
}

void copy_trad(TRAD *a, const TRAD *b, binmat_index_t n) {
	memcpy(a, b, n*n*sizeof(TRAD));
}

void transpose_trad(TRAD *output, const TRAD *input, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;
	TRAD *input_copy = NULL;
	const TRAD *real_input;

	//binmat_dprintf("binmat_transpose: Transposing matrix at %p, result at %p, size %lu\n", input, output, n);
	if (input == output) {
		//binmat_dprintf("binmat_transpose: In place transposition, using temporary matrix\n");
		input_copy = malloc(sizeof(TRAD) * n * n);
		copy_trad(input_copy, input, n);
		real_input = input_copy;
	} else {
		real_input = input;
	}

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			output[row * n + col] = real_input[col * n + row];
		}
	}

	free(input_copy);
}

int are_identical_trad_pure(const TRAD *a, const TRAD *b, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			if (a[row * n + col] != b[row * n + col]) {
				return 0;
			}
		}
	}
	return 1;
}

void multiply_trad(TRAD *output, const TRAD *a, const TRAD *b, binmat_index_t n) {
	binmat_index_t row;
	binmat_index_t col;
	binmat_index_t k;
	TRAD *t;

	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			t = &output[row * n + col];
			*t = 0;
			for (k = 0; k < n; k++) {
				*t += a[row * n + k] * b[k * n + col];
			}
		}
	}
}

void power_trad(TRAD *output, const TRAD *a, binmat_index_t n, unsigned int pow) {
	unsigned int i;
	TRAD *temp = malloc(sizeof(TRAD) * n * n);

	if (pow == 0) {
		// identity, yawn
	} else if (pow == 1) {
		// copy it
	} else {
		multiply_trad(output, a, a, n);
		for (i = 2; i < pow; i++) {
			multiply_trad(temp, output, a, n);
			copy_trad(output, temp, n);
		}
	}
	free(temp);
}

#endif



int main(int argc, char *argv[]) {
	binmat_index_t n;

	binmat_data_t *input;
	binmat_data_t *trans;
	binmat_data_t *output;
	binmat_data_t *final;
	binmat_data_t *transcheck;
	binmat_data_t *output_slow;

	TRAD *tinput;
	TRAD *toutput;
	TRAD *tfinal;
	TRAD *ttemp;
	TRAD *ttrans;
	TRAD *ttranscheck;

	unsigned int p;

	double density;
	unsigned int row;
	unsigned int col;

	struct timeval start;
	struct timeval end;
	struct timeval diff;
	struct timeval diff_trad;

	unsigned int rep;
	unsigned int warmups;
	unsigned int reps;
	unsigned int warmups_trad;
	unsigned int reps_trad;
	unsigned int seed;

	int do_trad;



	//n = 64;
	//n = 2048;
	//p = 6;
	n = 20;
	p = 3;

	if (argc > 1) {
		n = atoi(argv[1]);
		if (argc > 2) {
			p = atoi(argv[2]);
		}
	}

	do_trad = 1;

	//warmups = 3;
	//reps = 10;
	warmups = 1;
	reps = 1;
	if (do_trad) {
		warmups_trad = 1;
		reps_trad = 1;
	}

	//density = 0.2;
	//density = 0.075;
	density = 0.01;
	//density = 0.095;
	seed = 0;
	//seed = 10;


	printf("binmat-test: %ux%u matrix, to %u power\n", n, n, p);
	printf("binmat-test: binmat_chunkbytes = %lu, binmat_chunkbits = %lu\n", binmat_chunkbytes, binmat_chunkbits);


	printf("binmat-test: Allocating binmats...\n");
	input = binmat_alloc(n);
	trans = binmat_alloc(n);
	output = binmat_alloc(n);
	final = binmat_alloc(n);
	transcheck = binmat_alloc(n);
	output_slow = binmat_alloc(n);
	printf("binmat-test: done\n");

	printf("binmat-test: memsetting binmats...\n");
	memset(input, 0xF8, binmat_numbytes(n));
	memset(trans, 0xF9, binmat_numbytes(n));
	memset(output, 0xFA, binmat_numbytes(n));
	memset(final, 0xFB, binmat_numbytes(n));
	printf("binmat-test: done\n");

	if (do_trad) {
		printf("binmat-test: Allocating trads...\n");
		tinput = malloc(sizeof(TRAD) * n * n);
		toutput = malloc(sizeof(TRAD) * n * n);
		tfinal = malloc(sizeof(TRAD) * n * n);
		ttemp = malloc(sizeof(TRAD) * n * n);
		ttrans = malloc(sizeof(TRAD) * n * n);
		ttranscheck = malloc(sizeof(TRAD) * n * n);
		printf("binmat-test: done\n");

		printf("binmat-test: memsetting trads...\n");
		memset(tinput, 0xFC, sizeof(TRAD) * n * n);
		memset(toutput, 0xFD, sizeof(TRAD) * n * n);
		memset(tfinal, 0xFE, sizeof(TRAD) * n * n);
		memset(ttrans, 0xF0, sizeof(TRAD) * n * n);
		memset(ttranscheck, 0xF1, sizeof(TRAD) * n * n);
		printf("binmat-test: done\n");
	}



	// setup input
	srand(seed);
	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			binmat_setbit(input, n, row, col, ( ((double)rand()) / ((double)RAND_MAX) < density ));
		}
	}


	printf("Input (hex):\n");
	//binmat_print_matrix_hex(stdout, input, n);
	printf("\n");
	printf("Input:\n");
	//binmat_print_matrix_slow(stdout, input, n);
	printf("\n");
	printf("Input (fast print):\n");
	//binmat_print_matrix_fast(stdout, input, n);
	printf("\n");


	// setup trans
	binmat_transpose(trans, input, n);
	printf("Trans:\n");
	//binmat_print_matrix_slow(stdout, trans, n);
	printf("\n");


	printf("finalchunkbits(%u) = %lu\n", n, binmat_finalchunkbits(n));
	printf("finalchunkmask(%u) = 0x%"BINMAT_PRINTF_MODIFIER"x\n", n, binmat_finalchunkmask(n));
	printf("finalchunkbits(%lu) = %lu\n", binmat_chunkbits, binmat_finalchunkbits(binmat_chunkbits));
	printf("finalchunkmask(%lu) = 0x%"BINMAT_PRINTF_MODIFIER"x\n", binmat_chunkbits, binmat_finalchunkmask(binmat_chunkbits));
	printf("finalchunkbits(%lu) = %lu\n", 5*binmat_chunkbits, binmat_finalchunkbits(5*binmat_chunkbits));
	printf("finalchunkmask(%lu) = 0x%"BINMAT_PRINTF_MODIFIER"x\n", 5*binmat_chunkbits, binmat_finalchunkmask(5*binmat_chunkbits));
	printf("\n");

	// check trans
	binmat_transpose(transcheck, trans, n);
	printf("Transcheck: %s\n", binmat_are_identical(input, transcheck, n) ? "Identical" : "DIFFERENT!");
	//binmat_print_matrix_slow(stdout, transcheck, n);
	printf("\n");



	binmat_multiply_slow(output_slow, input, input, n);
	printf("Multiply (slow):\n");
	//binmat_print_matrix_slow(stdout, output_slow, n);
	printf("\n");


	binmat_multiply(output, input, trans, n);
	printf("Multiply: %s\n", binmat_are_identical(output, output_slow, n) ? "Identical" : "DIFFERENT!");
	//binmat_print_matrix_slow(stdout, output, n);
	printf("\n");

	for (rep = 0; rep < warmups; rep++) {
		binmat_power(final, input, trans, n, p);
	}
	gettimeofday(&start, NULL);
	for (rep = 0; rep < reps; rep++) {
		binmat_power(final, input, trans, n, p);
	}
	gettimeofday(&end, NULL);
	printf("Power (%u):\n", p);
	timersub(&end, &start, &diff);
	printf("Time for %u reps: %ld.%06lds\n", reps, diff.tv_sec, diff.tv_usec);
	//binmat_print_matrix_slow(stdout, final, n);
	printf("\n");






	if (do_trad) {
		init_trad(tinput, input, n);

		printf("Trad Input: %s\n", are_identical_trad(input, tinput, n) ? "Identical" : "DIFFERENT!");
		//print_trad_binary(tinput, n);
		printf("\n");

		transpose_trad(ttrans, tinput, n);

		printf("Trad Trans: %s\n", are_identical_trad(trans, ttrans, n) ? "Identical" : "DIFFERENT!");
		//print_trad_binary(ttrans, n);
		printf("\n");
		printf("\n");
		//print_trad(ttrans, n);
		printf("\n");

		transpose_trad(ttranscheck, ttrans, n);
		printf("Done\n");

		printf("Trad Transcheck: %s %s\n", are_identical_trad(transcheck, ttranscheck, n) ? "Identical" : "DIFFERENT!", are_identical_trad_pure(tinput, ttranscheck, n) ? "Identical" : "DIFFERENT!");
		//print_trad_binary(ttranscheck, n);
		printf("\n");
		printf("\n");
		//print_trad(ttranscheck, n);
		printf("\n");

		multiply_trad(toutput, tinput, tinput, n);

		printf("Trad Multiply: %s\n", are_identical_trad(output, toutput, n) ? "Identical" : "DIFFERENT!");
		//print_trad_binary(toutput, n);
		printf("\n");
		printf("\n");
		//print_trad(toutput, n);
		printf("\n");

		multiply_trad(ttemp, toutput, tinput, n);
		copy_trad(toutput, ttemp, n);

		printf("Trad Multiply (2):\n");
		//print_trad_binary(toutput, n);
		printf("\n");
		printf("\n");
		//print_trad(toutput, n);
		printf("\n");

		power_trad(tfinal, tinput, n, 3);
		printf("Trad Power (3): %s\n", are_identical_trad_pure(toutput, tfinal, n) ? "Identical" : "DIFFERENT!");
		//print_trad(tfinal, n);
		printf("\n");


		for (rep = 0; rep < warmups_trad; rep++) {
			power_trad(tfinal, tinput, n, p);
		}
		gettimeofday(&start, NULL);
		for (rep = 0; rep < reps_trad; rep++) {
			power_trad(tfinal, tinput, n, p);
		}
		gettimeofday(&end, NULL);
		printf("Trad Power (%u): %s\n", p, are_identical_trad(final, tfinal, n) ? "Identical" : "DIFFERENT!");
		timersub(&end, &start, &diff_trad);
		//print_trad_binary(tfinal, n);
		printf("\n");
		printf("\n");
		//print_trad(tfinal, n);
		printf("\n");
	}

	printf("     Power (%u): Time for %u reps: %ld.%06lds\n", p, reps, diff.tv_sec, diff.tv_usec);
	if (do_trad) {
		printf("Trad power (%u): Time for %u reps: %ld.%06lds\n", p, reps_trad, diff_trad.tv_sec, diff_trad.tv_usec);
	}



	printf("binmat-test: freeing binmats...\n");
	binmat_free(input);
	binmat_free(trans);
	binmat_free(output);
	binmat_free(final);
	binmat_free(transcheck);
	binmat_free(output_slow);
	printf("binmat-test: done\n");

	if (do_trad) {
		printf("binmat-test: freeing trads...\n");
		free(tinput);
		free(toutput);
		free(tfinal);
		free(ttemp);
		printf("binmat-test: done\n");
	}

	return 0;
}

