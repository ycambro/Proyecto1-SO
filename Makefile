# Rutas
INCLUDE_DIR = -Iinclude -Iexternal/inih
SRC_DIR = src
PTHREADS_SRC = $(SRC_DIR)/mypthreads/my_thread.c $(SRC_DIR)/mypthreads/my_mutex.c
INI_SRC = $(SRC_DIR)/config_parser.c external/inih/ini.c

# Tests
TEST_DIR = tests
TESTS = test_round_robin test_lottery test_lottery_bias test_realtime

# Ejecutables y sus schedulers correspondientes
EXEC_test_round_robin    = $(TEST_DIR)/test_round_robin.c    $(PTHREADS_SRC) $(SRC_DIR)/schedulers/round_robin.c
EXEC_test_lottery        = $(TEST_DIR)/test_lottery.c        $(PTHREADS_SRC) $(SRC_DIR)/schedulers/lottery.c
EXEC_test_lottery_bias   = $(TEST_DIR)/test_lottery_bias.c   $(PTHREADS_SRC) $(SRC_DIR)/schedulers/lottery.c
EXEC_test_realtime       = $(TEST_DIR)/test_realtime.c       $(PTHREADS_SRC) $(SRC_DIR)/schedulers/realtime.c

# Servidor y monitor individuales
SERVER_SRC = $(SRC_DIR)/animacion/server.c
MONITOR_SRC = $(SRC_DIR)/animacion/monitor.c
SERVER_BIN = server
MONITOR_BIN = animar

# Compilador y flags
CC = gcc
CFLAGS = -Wall -g

# Scheduler por defecto
SCHED_SRC = $(SRC_DIR)/schedulers/scheduler.c

# Compilar servidor
$(SERVER_BIN): $(SERVER_SRC) $(PTHREADS_SRC) $(SCHED_SRC) $(INI_SRC)
	@echo "🔧 Compilando servidor con scheduler despachador"
	$(CC) -o $@ $^ $(INCLUDE_DIR)

# Compilar monitor clásico
$(MONITOR_BIN): $(MONITOR_SRC)
	@echo "🎥 Compilando monitor"
	$(CC) -o $@ $^ -lncurses $(INCLUDE_DIR)


# Compilar solo el monitor (sin ejecutarlo)
build_monitor: $(MONITOR_BIN)
	@echo "✅ Monitor compilado correctamente."
 
# Regla por defecto
all: $(TESTS) $(SERVER_BIN) $(MONITOR_BIN) animar

# Reglas de compilación de pruebas
test_round_robin:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_round_robin) $(INCLUDE_DIR)

test_lottery:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_lottery) $(INCLUDE_DIR)

test_lottery_bias:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_lottery_bias) $(INCLUDE_DIR)

test_realtime:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_realtime) $(INCLUDE_DIR)

# Ejecutar pruebas
run_round: test_round_robin
	./test_round_robin

run_lottery: test_lottery
	./test_lottery

run_realtime: test_realtime
	./test_realtime

# Ejecutar servidor y monitor clásico
run_server: $(SERVER_BIN)
	./$(SERVER_BIN)

run_monitor: $(MONITOR_BIN)
	./$(MONITOR_BIN)

# Limpiar
clean:
	rm -f $(SERVER_BIN) $(MONITOR_BIN) $(TESTS) $(ANIMAR_BIN)
	@echo "🧹 Limpiado completo."