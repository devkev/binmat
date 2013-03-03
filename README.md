 binmat
========

A C library for efficient handling of 2D binary matrices.

by Kevin Pulo, <kev@pulo.com.au>  
<https://github.com/devkev/binmat>  
Copyright (c) 2013  
Licensed under the GNU GPL v3 or higher, as per the COPYING file.


Overview and motivation
------------------------

Some applications need to store and manipulate a 2D matrix of binary (ie.
boolean) values.  For example, an unweighted graph can be stored as an adjacency
matrix - the _(i,j)_-th element is a boolean indicating whether or not the
_i_-th node is connected to the _j_-th.  Given such a data structure,
_k_-connectivity (for each node, the other nodes that can be reached in at most
_k_ edges or "hops") can easily be computed by taking the _k_-th power of the
adjacency matrix.

However, for large matrices, storing each element as a byte (or worse, a 32 or
64 bit integer) is incredibly inefficient and wasteful, both in time and space.
For example, a 10k square matrix requires 400Mb of RAM if stored as 32 bit int
values.

Binmat is a library that bit-packs these matrices, so that the above 10k square
matrix requires just 12.5Mb (the minimum space possible to store such a dense
matrix).

More than that, binmat takes advantage of extremely fast bit-operations when
multiplying matrices.  The usual series of multiplications and additions
required to compute each element are replaced by bitwise AND and OR operations.
Furthermore, on 64-bit hardware each bitwise operation can replace up to _64_
multiplications or additions, reducing operations that can take hundreds of
clock cycles down to just a single cycle.  This gives rise to some very
considerable performance increases, especially when taking the power of a
matrix.  Binary exponentiation is implemented to further improve the performance
of higher matrix powers.


Building
---------

binmat currently uses a straightforward GNU make Makefile, and so a simple
`make` (or `gmake`) should work on all modern Unixy operating systems.

There are no dependencies beyond a C compiler and associated toolchain.


Installing
-----------

After building, simply copy `libbinmat.so` and/or `libbinmat.a` to a location
that the linker can find (eg. `/usr/local/lib`), and `libbinmat.h` to a location
that the C preprocessor/compiler can find (eg. `/usr/local/include`).

Alternatively, you can specify `-I/path/to/libbinmat` when compiling and
`-L/path/to/libbinmat` when linking.

To use binmat, simply do

    #include "libbinmat.h"

in your code, and then use `-lbinmat` when linking.


API
----

If the `HAVE_UNSIGNED_LONG_LONG_INT` preprocessor symbol is defined, then binmat
will use `unsigned long long` for its underlying chunk storage, otherwise
`unsigned long` will be used.  Alternatively, the `BINMAT_DATA_TYPE` macro can
be defined, and that will be used instead, eg. `-DBINMAT_DATA_TYPE='unsigned
char'`.  This type must be unsigned.


### Typedefs

* `binmat_data_t`  
   The underlying bit storage, aka the "_chunk_"
   (`unsigned long long` or `unsigned long`).

* `binmat_index_t`  
   The type of indices into arrays of `binmat_data_t`
   (`unsigned int`).

* `binmat_bool_t`  
   The native C type used to indicate true/false
   (`unsigned char`).


### Consts

* `const binmat_data_t one`  
   The constant `1U` as a `binmat_data_t` type.

* `const binmat_data_t zero`  
   The constant `0U` as a `binmat_data_t` type.


### Macros

* `binmat_chunkbytes`  
   The number of bytes in a chunk.

* `binmat_chunkbits`  
   The number of bits in a chunk.

* `binmat_numchunks(n)`  
   The number of chunks required to store _n_ bits.

* `binmat_numbytes(n)`  
   The number of bytes required to store _n_ bits.

* `binmat_numbits(n)`  
   The number of bits required to store _n_ bits (will be greater than _n_ when
   _n_ is not a multiple of `binmat_chunkbits`).

* `binmat_numbytes_matrix(n)`  
   The number of bytes required to store an _n_ by _n_ matrix.

* `binmat_finalchunkbits(n)`  
   The number of bits used in the final chunk (for _n_ bits total).  If _n_ is a
   multiple of `binmat_chunkbits`, then this will be `binmat_chunkbits` (not 0).

* `binmat_finalchunkmask(n)`  
   A mask of the bits used in the final chunk.

* `binmat_binary_bufn_t(x,n)`  
   Declare a `char` array called _x_ of sufficient size to render _n_ chunks.

* `binmat_binary_buf_t(x)`  
   Declare a `char` array called _x_ of sufficient size to render a single chunk.


### Functions

* `binmat_data_t *binmat_alloc(binmat_index_t n)`  
   Allocate an _n_ by _n_ matrix (initialised to all zeros) and return a
   reference to it.

* `void binmat_free(binmat_data_t *m)`  
   Free an allocated matrix _m_.

* `binmat_bool_t binmat_getbit(const binmat_data_t *m, binmat_index_t n,
                             binmat_index_t row, binmat_index_t col)`  
   Returns the (_row_,_col_) bit of the _n_ by _n_ matrix _m_.

* `void binmat_setbit(binmat_data_t *m, binmat_index_t n, binmat_index_t row,
                    binmat_index_t col, binmat_bool_t value)`  
   Sets the (_row_,_col_) bit of the _n_ by _n_ matrix _m_ to be _value_.

* `char *binmat_format_chunk(binmat_data_t x, char *buf)`  
   Helper function that renders the chunk _x_ into _buf_ (and returns a
   reference to it).  The buffer must be of sufficient length (see the
   `binmat_binary_buf_t()` macro).

* `void binmat_print_matrix_slow(FILE *f, const binmat_data_t *m,
                               binmat_index_t n)`  
   Uses a slow, obvious algorithm (ie. loop over the bits and call
   `binmat_getbit()`) to print the _n_ by _n_ matrix _m_ to the stdio FILE _f_.

* `void binmat_print_matrix_fast(FILE *f, const binmat_data_t *m,
                               binmat_index_t n)`  
   Use a faster algorithm to print the _n_ by _n_ matrix _m_ to the stdio FILE
   _f_.  **Currently broken**

* `void binmat_print_matrix_hex(FILE *f, const binmat_data_t *m,
                              binmat_index_t n)`  
   Print the _n_ by _n_ matrix _m_ to the stdio FILE _f_ in a more compact hex
   notation.  **Currently broken**

* `void binmat_transpose(binmat_data_t *output, const binmat_data_t *input,
                       binmat_index_t n)`  
   Transpose the _n_ by _n_ matrix _input_ and store the output in _output_
   (which must already have been allocated).  _output_ can be the same as
   _input_, in which case a temporary matrix will be allocated and used.

* `int binmat_are_identical(const binmat_data_t *a, const binmat_data_t *b,
                          binmat_index_t n)`  
   Compare the two _n_ by _n_ matrices _a_ and _b_, and return true if they are
   identical and false otherwise.

* `void binmat_copy(binmat_data_t *a, const binmat_data_t *b,
                  binmat_index_t n)`  
   Copy the _n_ by _n_ matrix _b_ into _a_ (which must already have been
   allocated).

* `void binmat_multiply(binmat_data_t *output, const binmat_data_t *a,
                      const binmat_data_t *b, binmat_index_t n)`  
   Uses a fast algorithm to multiply together the _n_ by _n_ matrices _a_ and
   _b_.  Note that the _b_ matrix must have been transposed before being passed
   to this function.

* `void binmat_multiply_slow(binmat_data_t *output, const binmat_data_t *a,
                           const binmat_data_t *b, binmat_index_t n)`  
   Uses the simple, naive matrix multiplication algorithm (ie. using
   `binmat_getbit()` and `binmat_setbit()`) to multiply the _n_ by _n_ matrices
   _a_ and _b_ together, storing the result in _output_.  Mostly useful to
   check the result of `binmat_multiply()`.

* `void binmat_power(binmat_data_t *output, const binmat_data_t *a,
                   const binmat_data_t *trans, binmat_index_t n,
                   unsigned int pow)`  
   Takes the _pow_-th power of the _n_ by _n_ matrix _a_ (with _trans_ its
   transpose), storing the result in _output_.

* `void binmat_power_slow(binmat_data_t *output, const binmat_data_t *a,
                        binmat_index_t n, unsigned int pow)`  
   Uses the slow, naive algorithm (ie. repeated application of
   `binmat_multiply_slow()`) to take the _pow_-th power of the _n_ by _n_ matrix
   _a_, storing the result in _output_.


Testing
--------

The `binmat-test` program is used to test binmat's operations.  In addition to
confirming that fast binmat multiplication and powering matches that of the
naive implementations, it also implements naive multiplication and power using
straightforward "traditional" int-based matrices, and compares the results
against them.  This also means that it can be used for benchmarking, to test the
performance improvements possible with binmat (although the limiting factor
quickly becomes the time and space needed for the traditional matrices).

It takes the following command line options.  _n_ indicates a positive integer,
and _d_ indicates a (double) floating point value.

* `-n`, `--size` _n_  
  Specify the size of the (square) matrix (default: 151).

* `-p`, `--power` _n_  
  Specify the power to which the matrix should be raised (default: 3).

* `-w`, `--warmups` _n_  
  Specify the number of times to power the matrix as a warmup (ie. untimed)
  (default: 3).

* `-r`, `--reps` _n_  
  Specify the number of times to power the matrix (timed) (default: 10).

* `-t`, `--trad`  
  Include traditional matrix operations and comparisons (default).

* `-T`, `--no-trad`  
  Do not include traditional matrix operations and comparisons.

* `-tw`, `--trad-warmups` _n_  
  Specify the number of times to power the traditional matrix as a warmup
  (default: 1).

* `-tr`, `--trad-reps` _n_  
  Specify the number of times to power the traditional matrix (default: 1).

* `-d`, `--density` _d_  
  Specify the density of ones in the random matrix (between 0.0 and 1.0)
  (default: 0.01).

* `-s`, `--seed` _n_  
  Specify the PRNG seed value (default: `time(NULL)`).

* `-v`, `--verbose`, `--print`  
  Include verbose output (ie. print the matrices).

* `-V`, `--no-verbose`, `--silent`  
  Do not print the actual matrix contents (default).

* `-l`, `--limit` _d_  
  The maximum amount of memory (in Mb) that will be allocated (default: 1024).


License
--------

binmat is copyright (c) 2013 Kevin Pulo, <kev@pulo.com.au>.  
Licensed under the GNU GPL v3 or higher, as per the COPYING file.

However, if you're interested in using binmat in a way that is not compatible
with the GPLv3+ (whether Free/Open Source or not), please feel free to contact
me at the above email address to discuss the possibility of dual licensing or
alternative arrangements.


Feedback
---------

Comments, feature suggestions, bug reports, patches, etc are most welcome.  Feel
free to submit issues, pull requests, etc on github.


