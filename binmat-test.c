
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
#include <time.h>


#if !defined(TRAD)
#define TRAD unsigned int
//#define TRAD unsigned char
#endif



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


/* This is basically reimplementing getopt(). */
/* After going through this exercise I'll probably switch to getopt_long(). */
int ul_arg(int argc, char *argv[], int *i, const char *arg, unsigned long *result) {
	char *endpt;

	if (i == NULL || result == NULL) {
		return 0;
	}

	if (!strcmp(argv[*i], arg)) {
		if (*i >= argc-1) {
			fprintf(stderr, "binmat-test: Error: missing argument to %s\n", arg);
			exit(1);
		}
		(*i)++;
		*result = strtoul(argv[*i], &endpt, 0);
		if (*endpt) {
			fprintf(stderr, "binmat-test: Error converting argument to %s\n", arg);
			exit(1);
		}
		return 1;
	}
	return 0;
}

int ui_arg(int argc, char *argv[], int *i, const char *arg, unsigned int *result) {
	int rc;
	unsigned long r;

	if (result == NULL) {
		return 0;
	}

	rc = ul_arg(argc, argv, i, arg, &r);
	if (rc) {
		*result = r;
	}
	return rc;
}

int d_arg(int argc, char *argv[], int *i, const char *arg, double *result) {
	char *endpt;

	if (i == NULL || result == NULL) {
		return 0;
	}

	if (!strcmp(argv[*i], arg)) {
		if (*i >= argc-1) {
			fprintf(stderr, "binmat-test: Error: missing argument to %s\n", arg);
			exit(1);
		}
		(*i)++;
		*result = strtod(argv[*i], &endpt);
		if (*endpt) {
			fprintf(stderr, "binmat-test: Error converting argument to %s\n", arg);
			exit(1);
		}
		return 1;
	}
	return 0;
}

int b_arg(int argc, char *argv[], int *i, const char *arg, int *result, int invert) {
	if (i == NULL || result == NULL) {
		return 0;
	}

	if (!strcmp(argv[*i], arg)) {
		*result = ! invert;
		return 1;
	}
	return 0;
}


static int warnings = 0;

const char *check(int result) {
	if (!result) {
		warnings++;
	}
	return result ? "Identical" : "DIFFERENT!";
}


int main(int argc, char *argv[]) {
	binmat_index_t n = 0;

	binmat_data_t *input = NULL;
	binmat_data_t *trans = NULL;
	binmat_data_t *output = NULL;
	binmat_data_t *final = NULL;
	binmat_data_t *transcheck = NULL;
	binmat_data_t *output_slow = NULL;

	TRAD *tinput = NULL;
	TRAD *toutput = NULL;
	TRAD *tfinal = NULL;
	TRAD *ttemp = NULL;
	TRAD *ttrans = NULL;
	TRAD *ttranscheck = NULL;

	unsigned int p = 0;

	double density = 0;
	unsigned int row = 0;
	unsigned int col = 0;

	struct timeval start = { .tv_sec = 0, .tv_usec = 0 };
	struct timeval end = { .tv_sec = 0, .tv_usec = 0 };
	struct timeval diff = { .tv_sec = 0, .tv_usec = 0 };
	struct timeval diff_trad = { .tv_sec = 0, .tv_usec = 0 };

	unsigned int rep = 0;
	unsigned int warmups = 0;
	unsigned int reps = 0;
	unsigned int warmups_trad = 0;
	unsigned int reps_trad = 0;
	unsigned int seed = 0;

	int do_trad = 0;
	int do_print = 0;

	int i = 0;



	// Defaults
	n = 151;
	p = 3;

	warmups = 3;
	reps = 10;
	warmups_trad = 1;
	reps_trad = 1;

	density = 0.01;
	seed = time(NULL);

	do_trad = 1;
	do_print = 0;


	for (i = 1; i < argc; i++) {
		       if (ui_arg(argc, argv, &i, "-n", &n)) {
		} else if (ui_arg(argc, argv, &i, "--size", &n)) {

		} else if (ui_arg(argc, argv, &i, "-p", &p)) {
		} else if (ui_arg(argc, argv, &i, "--power", &p)) {

		} else if (ui_arg(argc, argv, &i, "-s", &seed)) {
		} else if (ui_arg(argc, argv, &i, "--seed", &seed)) {

		} else if (ui_arg(argc, argv, &i, "-w", &warmups)) {
		} else if (ui_arg(argc, argv, &i, "--warmup", &warmups)) {
		} else if (ui_arg(argc, argv, &i, "--warmups", &warmups)) {

		} else if (ui_arg(argc, argv, &i, "-r", &reps)) {
		} else if (ui_arg(argc, argv, &i, "--reps", &reps)) {

		} else if (ui_arg(argc, argv, &i, "-tw", &warmups_trad)) {
		} else if (ui_arg(argc, argv, &i, "--trad-warmup", &warmups_trad)) {
		} else if (ui_arg(argc, argv, &i, "--trad-warmups", &warmups_trad)) {

		} else if (ui_arg(argc, argv, &i, "-tr", &reps_trad)) {
		} else if (ui_arg(argc, argv, &i, "--trad-reps", &reps_trad)) {

		} else if (d_arg(argc, argv, &i, "-d", &density)) {
		} else if (d_arg(argc, argv, &i, "--density", &density)) {

		} else if (b_arg(argc, argv, &i, "-t", &do_trad, 0)) {
		} else if (b_arg(argc, argv, &i, "--trad", &do_trad, 0)) {
		} else if (b_arg(argc, argv, &i, "-T", &do_trad, 1)) {
		} else if (b_arg(argc, argv, &i, "--no-trad", &do_trad, 1)) {

		} else if (b_arg(argc, argv, &i, "-v", &do_print, 0)) {
		} else if (b_arg(argc, argv, &i, "--verbose", &do_print, 0)) {
		} else if (b_arg(argc, argv, &i, "--print", &do_print, 0)) {
		} else if (b_arg(argc, argv, &i, "--no-silent", &do_print, 0)) {
		} else if (b_arg(argc, argv, &i, "-V", &do_print, 1)) {
		} else if (b_arg(argc, argv, &i, "--no-verbose", &do_print, 1)) {
		} else if (b_arg(argc, argv, &i, "--no-print", &do_print, 1)) {
		} else if (b_arg(argc, argv, &i, "--silent", &do_print, 1)) {

		} else {
			fprintf(stderr, "binmat-test: Error: unknown arg \"%s\"\n", argv[i]);
			exit(1);
		}
	}


	fprintf(stderr, "binmat-test: %ux%u matrix, to %u power\n", n, n, p);
	fprintf(stderr, "binmat-test: binmat_chunkbytes = %lu, binmat_chunkbits = %lu\n", binmat_chunkbytes, binmat_chunkbits);
	fprintf(stderr, "binmat-test: density %lf\n", density);
	fprintf(stderr, "binmat-test: do_trad? %s\n", do_trad ? "yes" : "no");

	fprintf(stderr, "finalchunkbits(%u) = %lu\n", n, binmat_finalchunkbits(n));
	fprintf(stderr, "finalchunkmask(%u) = 0x%"BINMAT_PRINTF_MODIFIER"x\n", n, binmat_finalchunkmask(n));
	fprintf(stderr, "finalchunkbits(%lu) = %lu\n", binmat_chunkbits, binmat_finalchunkbits(binmat_chunkbits));
	fprintf(stderr, "finalchunkmask(%lu) = 0x%"BINMAT_PRINTF_MODIFIER"x\n", binmat_chunkbits, binmat_finalchunkmask(binmat_chunkbits));
	fprintf(stderr, "finalchunkbits(%lu) = %lu\n", 5*binmat_chunkbits, binmat_finalchunkbits(5*binmat_chunkbits));
	fprintf(stderr, "finalchunkmask(%lu) = 0x%"BINMAT_PRINTF_MODIFIER"x\n", 5*binmat_chunkbits, binmat_finalchunkmask(5*binmat_chunkbits));
	fprintf(stderr, "\n");


	fprintf(stderr, "binmat-test: Allocating binmats...\n");
	input = binmat_alloc(n);
	trans = binmat_alloc(n);
	output = binmat_alloc(n);
	final = binmat_alloc(n);
	transcheck = binmat_alloc(n);
	output_slow = binmat_alloc(n);
	fprintf(stderr, "binmat-test: done\n");

	fprintf(stderr, "binmat-test: memsetting binmats...\n");
	memset(input, 0xF8, binmat_numbytes(n));
	memset(trans, 0xF9, binmat_numbytes(n));
	memset(output, 0xFA, binmat_numbytes(n));
	memset(final, 0xFB, binmat_numbytes(n));
	fprintf(stderr, "binmat-test: done\n");

	if (do_trad) {
		fprintf(stderr, "binmat-test: Allocating trads...\n");
		tinput = malloc(sizeof(TRAD) * n * n);
		toutput = malloc(sizeof(TRAD) * n * n);
		tfinal = malloc(sizeof(TRAD) * n * n);
		ttemp = malloc(sizeof(TRAD) * n * n);
		ttrans = malloc(sizeof(TRAD) * n * n);
		ttranscheck = malloc(sizeof(TRAD) * n * n);
		fprintf(stderr, "binmat-test: done\n");

		fprintf(stderr, "binmat-test: memsetting trads...\n");
		memset(tinput, 0xFC, sizeof(TRAD) * n * n);
		memset(toutput, 0xFD, sizeof(TRAD) * n * n);
		memset(tfinal, 0xFE, sizeof(TRAD) * n * n);
		memset(ttrans, 0xF0, sizeof(TRAD) * n * n);
		memset(ttranscheck, 0xF1, sizeof(TRAD) * n * n);
		fprintf(stderr, "binmat-test: done\n");
	}



	// setup input
	srand(seed);
	fprintf(stderr, "Input: ");
	for (row = 0; row < n; row++) {
		for (col = 0; col < n; col++) {
			binmat_setbit(input, n, row, col, ( ((double)rand()) / ((double)RAND_MAX) < density ));
		}
	}
	fprintf(stderr, "done\n");


	if (do_print) {
		binmat_print_matrix_slow(stderr, input, n);
		fprintf(stderr, "\n");
		// Fast printing and hex printing are currently buggy.
		//fprintf(stderr, "Input (fast print):\n");
		//binmat_print_matrix_fast(stderr, input, n);
		//fprintf(stderr, "\n");
		//fprintf(stderr, "Input (hex):\n");
		//binmat_print_matrix_hex(stderr, input, n);
		//fprintf(stderr, "\n");
	}


	// setup trans
	fprintf(stderr, "Trans: ");
	binmat_transpose(trans, input, n);
	fprintf(stderr, "done\n");
	if (do_print) {
		binmat_print_matrix_slow(stderr, trans, n);
		fprintf(stderr, "\n");
	}


	// check trans
	fprintf(stderr, "Transcheck: ");
	binmat_transpose(transcheck, trans, n);
	fprintf(stderr, "%s\n", check(binmat_are_identical(input, transcheck, n)));
	if (do_print) {
		binmat_print_matrix_slow(stderr, transcheck, n);
		fprintf(stderr, "\n");
	}



	fprintf(stderr, "Multiply (slow): ");
	binmat_multiply_slow(output_slow, input, input, n);
	fprintf(stderr, "done\n");
	if (do_print) {
		binmat_print_matrix_slow(stderr, output_slow, n);
		fprintf(stderr, "\n");
	}


	fprintf(stderr, "Multiply: ");
	binmat_multiply(output, input, trans, n);
	fprintf(stderr, "%s\n", check(binmat_are_identical(output, output_slow, n)));
	if (do_print) {
		binmat_print_matrix_slow(stderr, output, n);
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "Power (%u) (%u warmups): ", p, warmups);
	for (rep = 0; rep < warmups; rep++) {
		binmat_power(final, input, trans, n, p);
	}
	fprintf(stderr, "done\n");
	fprintf(stderr, "Power (%u) (%u reps): ", p, reps);
	gettimeofday(&start, NULL);
	for (rep = 0; rep < reps; rep++) {
		binmat_power(final, input, trans, n, p);
	}
	gettimeofday(&end, NULL);
	fprintf(stderr, "done\n");
	if (do_print) {
		binmat_print_matrix_slow(stderr, final, n);
		fprintf(stderr, "\n");
	}
	timersub(&end, &start, &diff);
	fprintf(stderr, "Time for %u reps: %ld.%06lds\n", reps, diff.tv_sec, diff.tv_usec);
	fprintf(stderr, "\n");


	if (do_trad) {
		fprintf(stderr, "Trad Input: ");
		init_trad(tinput, input, n);
		fprintf(stderr, "%s\n", check(are_identical_trad(input, tinput, n)));
		if (do_print) {
			print_trad_binary(tinput, n);
			fprintf(stderr, "\n");
			//print_trad(ttrans, n);
			//fprintf(stderr, "\n");
		}

		fprintf(stderr, "Trad Trans: ");
		transpose_trad(ttrans, tinput, n);
		fprintf(stderr, "%s\n", check(are_identical_trad(trans, ttrans, n)));
		if (do_print) {
			print_trad_binary(ttrans, n);
			fprintf(stderr, "\n");
		}

		fprintf(stderr, "Trad Transcheck: ");
		transpose_trad(ttranscheck, ttrans, n);
		fprintf(stderr, "%s %s\n", check(are_identical_trad(transcheck, ttranscheck, n)), check(are_identical_trad_pure(tinput, ttranscheck, n)));
		if (do_print) {
			print_trad_binary(ttranscheck, n);
			fprintf(stderr, "\n");
		}

		fprintf(stderr, "Trad Multiply: ");
		multiply_trad(toutput, tinput, tinput, n);
		fprintf(stderr, "%s\n", check(are_identical_trad(output, toutput, n)));
		if (do_print) {
			print_trad_binary(toutput, n);
			fprintf(stderr, "\n");
		}

		fprintf(stderr, "Trad Multiply (2): ");
		multiply_trad(ttemp, toutput, tinput, n);
		copy_trad(toutput, ttemp, n);
		fprintf(stderr, "done\n");
		if (do_print) {
			print_trad_binary(toutput, n);
			fprintf(stderr, "\n");
		}

		fprintf(stderr, "Trad Power (3): ");
		power_trad(tfinal, tinput, n, 3);
		fprintf(stderr, "%s\n", check(are_identical_trad_pure(toutput, tfinal, n)));
		if (do_print) {
			print_trad(tfinal, n);
			fprintf(stderr, "\n");
		}


		fprintf(stderr, "Trad Power (%u) (%u warmups): ", p, warmups_trad);
		for (rep = 0; rep < warmups_trad; rep++) {
			power_trad(tfinal, tinput, n, p);
		}
		fprintf(stderr, "done\n");
		fprintf(stderr, "Trad Power (%u) (%u reps): ", p, reps_trad);
		gettimeofday(&start, NULL);
		for (rep = 0; rep < reps_trad; rep++) {
			power_trad(tfinal, tinput, n, p);
		}
		gettimeofday(&end, NULL);
		fprintf(stderr, "%s\n", check(are_identical_trad(final, tfinal, n)));
		timersub(&end, &start, &diff_trad);
		if (do_print) {
			print_trad_binary(tfinal, n);
			fprintf(stderr, "\n");
		}

		fprintf(stderr, "     Power (%u): Time for %u reps: %ld.%06lds\n", p, reps, diff.tv_sec, diff.tv_usec);
		fprintf(stderr, "Trad power (%u): Time for %u reps: %ld.%06lds\n", p, reps_trad, diff_trad.tv_sec, diff_trad.tv_usec);
	}



	if (warnings) {
		fprintf(stderr, "\n");
		fprintf(stderr, "!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!\n");
		fprintf(stderr, "   %d CHECK%s GAVE DIFFERENT RESULTS!\n", warnings, (warnings > 1 ? "S" : ""));
		fprintf(stderr, "!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!\n");
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "binmat-test: freeing binmats...\n");
	binmat_free(input);
	binmat_free(trans);
	binmat_free(output);
	binmat_free(final);
	binmat_free(transcheck);
	binmat_free(output_slow);
	fprintf(stderr, "binmat-test: done\n");

	if (do_trad) {
		fprintf(stderr, "binmat-test: freeing trads...\n");
		free(tinput);
		free(toutput);
		free(tfinal);
		free(ttemp);
		fprintf(stderr, "binmat-test: done\n");
	}

	return 0;
}

