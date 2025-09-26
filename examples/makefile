CC=gcc

# RELEASE
CFLAGS=-pedantic -Wall -std=c99 -D_GNU_SOURCE -fPIC -I<path/to/homebrew/include>

LDFLAGS=-L<path/to/homebrew/lib>
LDLIBS=-lm -lcurl

.DEFAULT: basic

basic: mk_dir
	@echo ""
	@echo "Compiling problem file ..."
	$(CC) ${CFLAGS} ${LDFLAGS} -o basic_example basic_example.c ../src/pull_horizons.c ${LDLIBS}
	@echo ""
	@echo "Problem file compiled successfully."
	./basic_example
	@echo ""
	@echo "Problem file executed."

irreg: mk_dir
	@echo ""
	@echo "Compiling problem file ..."
	$(CC) ${CFLAGS} ${LDFLAGS} -o irregular_example irregular_example.c ../src/pull_horizons.c ${LDLIBS}
	@echo ""
	@echo "Problem file compiled successfully."
	./irregular_example
	@echo ""
	@echo "Problem file executed."

mk_dir:
	mkdir -p outputs

clean:
	@echo "Cleaning up local directory ..."
	@-rm -vf basic_example
	@-rm -vf irregular_example