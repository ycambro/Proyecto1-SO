# Rutas
INCLUDE_DIR = -Iinclude
SRC_DIR = src
PTHREADS_SRC = $(SRC_DIR)/mypthreads/my_thread.c $(SRC_DIR)/mypthreads/my_mutex.c

# Tests
TEST_DIR = tests
TESTS = test_round_robin test_lottery test_lottery_bias test_realtime

# Ejecutables y sus schedulers correspondientes
EXEC_test_round_robin    = $(TEST_DIR)/test_round_robin.c    $(PTHREADS_SRC) $(SRC_DIR)/schedulers/round_robin.c
EXEC_test_lottery        = $(TEST_DIR)/test_lottery.c        $(PTHREADS_SRC) $(SRC_DIR)/schedulers/lottery.c
EXEC_test_lottery_bias   = $(TEST_DIR)/test_lottery_bias.c   $(PTHREADS_SRC) $(SRC_DIR)/schedulers/lottery.c
EXEC_test_realtime       = $(TEST_DIR)/test_realtime.c       $(PTHREADS_SRC) $(SRC_DIR)/schedulers/realtime.c

# Compilador
CC = gcc
CFLAGS = -Wall -g

# Regla por defecto
all: $(TESTS)

# Reglas de compilación
test_round_robin:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_round_robin) $(INCLUDE_DIR)

test_lottery:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_lottery) $(INCLUDE_DIR)

test_realtime:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_realtime) $(INCLUDE_DIR)

# Ejecutar pruebas individualmente
run_round: test_round_robin
	./test_round_robin

run_lottery: test_lottery
	./test_lottery


run_realtime: test_realtime
	./test_realtime

# Limpiar binarios
clean:
	rm -f $(TESTS)
