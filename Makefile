# ========================================
# Rutas y Configuración
# ========================================
INCLUDE_DIR = -Iinclude -Iexternal/inih
SRC_DIR = src
TEST_DIR = tests

PTHREADS_SRC = $(SRC_DIR)/mypthreads/my_thread.c $(SRC_DIR)/mypthreads/my_mutex.c
INI_SRC = $(SRC_DIR)/config_parser.c external/inih/ini.c
SCHED_SRC = $(SRC_DIR)/schedulers/scheduler.c

# ========================================
# Binarios
# ========================================
SERVER_SRC = $(SRC_DIR)/animacion/server.c
MONITOR_SRC = $(SRC_DIR)/animacion/monitor.c

SERVER_BIN = server
ANIMAR_BIN = animar  # Nombre del binario de monitor (rebautizado)

# ========================================
# Compilación principal
# ========================================
all: $(TESTS) $(SERVER_BIN) $(ANIMAR_BIN)

# ========================================
# Compilar Servidor
# ========================================
$(SERVER_BIN): $(SERVER_SRC) $(PTHREADS_SRC) $(SCHED_SRC) $(INI_SRC)
	@echo "🔧 Compilando servidor con scheduler despachador"
	$(CC) -o $@ $^ $(INCLUDE_DIR)

# ========================================
# Compilar Monitor (animar)
# ========================================
$(ANIMAR_BIN): $(MONITOR_SRC)
	@echo "🎥 Compilando animar (monitor)"
	$(CC) -o $@ $^ -lncurses $(INCLUDE_DIR)

build_monitor: $(ANIMAR_BIN)
	@echo "✅ Monitor (animar) compilado correctamente."

# ========================================
# Pruebas Unitarias
# ========================================
TESTS = test_round_robin test_lottery test_lottery_bias test_realtime

EXEC_test_round_robin    = $(TEST_DIR)/test_round_robin.c    $(PTHREADS_SRC) $(SRC_DIR)/schedulers/round_robin.c
EXEC_test_lottery        = $(TEST_DIR)/test_lottery.c        $(PTHREADS_SRC) $(SRC_DIR)/schedulers/lottery.c
EXEC_test_lottery_bias   = $(TEST_DIR)/test_lottery_bias.c   $(PTHREADS_SRC) $(SRC_DIR)/schedulers/lottery.c
EXEC_test_realtime       = $(TEST_DIR)/test_realtime.c       $(PTHREADS_SRC) $(SRC_DIR)/schedulers/realtime.c

test_round_robin:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_round_robin) $(INCLUDE_DIR)

test_lottery:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_lottery) $(INCLUDE_DIR)

test_lottery_bias:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_lottery_bias) $(INCLUDE_DIR)

test_realtime:
	$(CC) $(CFLAGS) -o $@ $(EXEC_test_realtime) $(INCLUDE_DIR)

# ========================================
# Ejecutar Pruebas
# ========================================
run_round: test_round_robin
	./test_round_robin

run_lottery: test_lottery
	./test_lottery

run_realtime: test_realtime
	./test_realtime

# ========================================
# Ejecutar servidor y monitor
# ========================================
run_server: $(SERVER_BIN)
	./$(SERVER_BIN)

run_monitor: $(ANIMAR_BIN)
	./$(ANIMAR_BIN)

# ========================================
# Limpieza
# ========================================
clean:
	rm -f $(SERVER_BIN) $(ANIMAR_BIN) $(TESTS)
	@echo "🧹 Limpieza completa."
