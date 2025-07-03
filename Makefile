CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -O0

SRC = bitcask.c kv.c keydir.c
OBJ = $(SRC:.c=.o)

TEST_SOURCES := $(wildcard tests/test_*/test.c)
TEST_BINS := $(patsubst %.c, %, $(TEST_SOURCES))
TEST_DIRS := $(dir $(TEST_SOURCES))

.PHONY: all lib tests clean create_test_dirs test-run

all: lib tests

lib: $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build each test binary
$(TEST_BINS): %: %.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

tests: create_test_dirs $(TEST_BINS)

create_test_dirs:
	@for dir in $(TEST_DIRS); do \
		mkdir -p $$dir/data; \
	done

clean:
	rm -f $(OBJ)
	rm -f $(TEST_BINS)
	@for dir in $(TEST_DIRS); do \
		rm -rf $$dir/data; \
	done

run-tests: tests
	@echo "Running tests..."
	@$(foreach test, $(TEST_BINS), \
		printf "[RUN]  %s\n" $(test); \
		./$(test) && echo "[PASS] $(test)" || echo "[FAIL] $(test)"; \
	)

valgrind-tests: tests
	@echo "Running tests with Valgrind..."
	@success=1; \
	for test in $(TEST_BINS); do \
		printf "[VALGRIND] %s\n" $$test; \
		valgrind --leak-check=full --error-exitcode=1 ./$$test > /dev/null 2>&1; \
		if [ $$? -eq 0 ]; then \
			echo "[VALGRIND PASS] $$test"; \
		else \
			echo "[VALGRIND FAIL] $$test"; \
			success=0; \
		fi; \
	done; \
	if [ $$success -eq 0 ]; then \
		echo "Some tests failed Valgrind checks."; \
		exit 1; \
	else \
		echo "All tests passed Valgrind checks."; \
	fi

