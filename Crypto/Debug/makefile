################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

# Profiling macros
PERF_STAT := task-clock,cpu-clock,context-switches,cpu-migrations,page-faults,alignment-faults,cycles,stalled-cycles-frontend,stalled-cycles-backend,instructions,branches,branch-misses,cache-references,cache-misses,L1-dcache-loads,L1-dcache-stores,L1-icache-loads,L1-icache-load-misses,L1-icache-prefetch,L1-icache-prefetch-misses
LOG_LOCAL := perf.stat
LOG_WIDE := perf_wide.stat
LOG_RAW := perf_raw.stat
TEST_CS := abcdefghijklmnopqrstuvwxyz

# Hash values for passwords to test
HASH_INTEL := 0x4e5bbaeafc82ab7aa1385bea8ef5d30a
HASH_FALCON := 0xfa0d1a60ef6616bb28038515c8ea4cb2

# Default profiling macro values
proc := 1
repeat := 5

# Compiling macros
CFLAGS := -O2 -pg
RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include sh/subdir.mk
-include mpi/subdir.mk
-include common/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Crypto Shell

# Tool invocations
Crypto: $(OBJS) $(CRYPTO_OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	mpicc  --show $(CFLAGS) -o "launchMPI" $(CRYPTO_OBJS) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	
# Tool invocations
Shell: $(OBJS) $(SHELL_OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc $(CFLAGS) -o "Crypto" $(SHELL_OBJS) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS) $(SHELL_OBJS) $(CRYPTO_OBJS) $(EXECUTABLES) Crypto
	-@echo ' '

test:
	@echo 'Test di correttezza per attacco brute force locale su tre processi, passwd = intel'
	@(mpirun -np 3 --hostfile runners.host ./launchMPI $(HASH_INTEL) 5 $(TEST_CS) 0 1 0)
	@echo 'Test terminato'
	@echo ' '

profile_raw:
	@echo 'Verrà eseguito il profiling sulla media delle $(repeat) esecuzioni di attacco brute force locale senza uso della libreria MPI'
	@echo 'Numero di processi = 1, passwd = intel'
	@(perf stat -o $(LOG_RAW) -r$(repeat) -e $(PERF_STAT) ./launchMPI $(HASH_INTEL) 5 $(TEST_CS) 0 0 0)
	@echo 'Profiling terminato'
	@echo 'Log delle statistiche sul file $(LOG_RAW)'
	@echo ' '

profile_local:
	@echo 'Verrà eseguito il profiling sulla medie delle $(repeat) esecuzioni di attacco brute force locale'
	@echo 'Numero di processi = $(proc),  passwd = intel'
	@(perf stat -o perf.stat -r$(repeat) -e $(PERF_STAT) mpirun -np $(proc) --hostfile runners.host ./launchMPI $(HASH_INTEL) 5 $(TEST_CS) 0 0 0)
	@echo 'Profiling terminato'
	@echo 'Log delle statistiche sul file perf.stat'
	@echo ' '

profile_wide:
	@echo 'Verrà eseguito il profiling sull'attacco brute force distribuito'
	@echo 'Numero di processi = $(proc),  passwd = falcon'
	@(perf stat -o $(LOG_WIDE) -e $(PERF_STAT) mpirun -np $(proc) --hostfile runners.host ./launchMPI $(HASH_FALCON) 6 $(TEST_CS) 0 0 0)
	@echo 'Profiling terminato'
	@echo 'Log delle statistiche sul file $(LOG_WIDE)'
	@echo ' '

.PHONY: all clean dependents test profile_raw profile_local profile_wide
.SECONDARY:

-include ../makefile.targets
