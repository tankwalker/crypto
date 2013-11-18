################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

# Other
SYSTEM_TIME := $(shell date +%Y%m%d)

# Profiling macros
PERF_STAT := task-clock,cpu-clock,context-switches,cpu-migrations,page-faults,alignment-faults,cycles,stalled-cycles-frontend,stalled-cycles-backend,instructions,branches,branch-misses,cache-references,cache-misses,L1-dcache-loads,L1-dcache-stores,L1-icache-loads,L1-icache-load-misses,L1-icache-prefetch,L1-icache-prefetch-misses
LOG_LOCAL := perf_local_$(SYSTEM_TIME).stat
LOG_WIDE := perf_wide_$(SYSTEM_TIME).stat
LOG_RAW := perf_raw_$(SYSTEM_TIME).stat
TEST_CS := abcdefghijklmnopqrstuvwxyz
PERF_FLAGS := --append

# Hash values for passwords to test
HASH_INTEL := 0x4e5bbaeafc82ab7aa1385bea8ef5d30a
HASH_FALCON := 0xfa0d1a60ef6616bb28038515c8ea4cb2
HASH_5Z := 0x95ebc3c7b3b9f1d2c40fec14415d3cb8
HASH_6Z := 0x453e41d218e071ccfb2d1c99ce23906a

# TAU compiling macro
TAU_CC := /home/mpiuser/tau-2.22.2/x86_64/bin/tau_cc.sh
TAU_OPT := '-optCompInst -optNoRevert -optPdtInst -optCompile=-g -optPdtCOpts=-g -optMpi'
TAU_MAKE := /home/mpiuser/tau-2.22.2/x86_64/lib/Makefile.tau-memory-papi-mpi-pthread-pdt-profile-trace
TAU_GOAL := $(TAU_CC) -tau_options=$(TAU_OPT) -tau_makefile=$(TAU_MAKE)
TAU_COMPILING := 0

# Default profiling macro values
proc := 1
repeat := 5

# Executable names
MPI_EXEC := "launchMPI"
SHELL_EXEC := "crypto"

# Compiling macros
CFLAGS := -O2 -pg -g
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

ifeq ($(TAU_COMPILING),1)
CCC := $(TAU_GOAL)
else
CCC := mpicc $(CFLAGS)
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: crypto shell

# Tool invocations
crypto: $(OBJS) $(CRYPTO_OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	$(CCC) -o $(MPI_EXEC) $(CRYPTO_OBJS) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	
# Tool invocations
shell: $(OBJS) $(SHELL_OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc $(CFLAGS) -o $(SHELL_EXEC) $(SHELL_OBJS) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS) $(SHELL_OBJS) $(CRYPTO_OBJS) $(EXECUTABLES) $(MPI_EXEC) $(SHELL_EXEC)
	-@echo ' '

test:
	@echo 'Test di correttezza per attacco brute force locale su tre processi, passwd = intel'
	@(mpirun -np $(proc) --hostfile runners.host ./$(MPI_EXEC) $(HASH_INTEL) 5 $(TEST_CS) 0 1 0)
	@echo 'Test terminato'
	@echo ' '

gprofile:
	@echo 'Profiling con gprof'
	@(gprof ./$(MPI_EXEC) $(HASH_INTEL) 5 $(TEST_CS) 0 0 0)	
	@echo 'Profile terminato'
	@echo ' '

profile_raw:
	@echo 'Verrà eseguito il profiling sulla media delle $(repeat) esecuzioni di attacco brute force locale senza uso della libreria MPI'
	@echo 'Numero di processi = 1, passwd = intel'
	@(perf stat -o $(LOG_RAW) -r$(repeat) -e $(PERF_STAT) ./$(MPI_EXEC) $(HASH_INTEL) 5 $(TEST_CS) 0 0 0)
	@echo 'Profiling terminato'
	@echo 'Log delle statistiche sul file $(LOG_RAW)'
	@echo ' '

profile_local:
	@echo 'Verrà eseguito il profiling sulla medie delle $(repeat) esecuzioni di attacco brute force locale'
	@echo 'Numero di processi = $(proc),  passwd = zzzzz'
	@(perf stat -o $(LOG_LOCAL) $(PERF_FLAGS) -r$(repeat) -e $(PERF_STAT) mpirun -np $(proc) ./$(MPI_EXEC) $(HASH_6Z) 6 $(TEST_CS) 0 0 0)
	@echo 'Profiling terminato'
	@echo 'Log delle statistiche sul file $(LOG_LOCAL)'
	@echo ' '

profile_wide:
	@echo "Verrà eseguito il profiling sull'attacco brute force distribuito"
	@echo 'Numero di processi = $(proc),  passwd = zzzzzz'
	@(perf stat -o $(LOG_WIDE) $(PERF_FLAGS) -e $(PERF_STAT) mpirun -np $(proc) --hostfile runners.host ./$(MPI_EXEC) $(HASH_6Z) 6 $(TEST_CS) 0 0 0)
	@echo 'Profiling terminato'
	@echo 'Log delle statistiche sul file $(LOG_WIDE)'
	@echo ' '

profile_all:
	for number in 2 3 4 6 8 ; do \
	make profile_local proc=$$number ; \
	done

profile_wall:
	for number in 2 3 4 5 6 8 10 12; do \
	make profile_wide proc=$$number; \
	done 

vampir:
	@(mpirun -np $(proc) vtrun -v ./$(MPI_EXEC) $(HASH_ZZZZZ) 5 $(TEST_CS) 0 0 0)

.PHONY: all clean dependents test profile_raw profile_local profile_wide profile_all pofile_wall
.SECONDARY:

-include ../makefile.targets
