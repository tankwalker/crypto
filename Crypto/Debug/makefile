################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

OFLAG := -O1
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
	mpicc  $(OFLAG) -o "launchMPI" $(CRYPTO_OBJS) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	
# Tool invocations
Shell: $(OBJS) $(SHELL_OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  $(OFLAG) -o "Crypto" $(SHELL_OBJS) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS) $(SHELL_OBJS) $(CRYPTO_OBJS) $(EXECUTABLES) Crypto
	-@echo ' '

test:
	@echo 'Test su passwd = falcon, processi = 1'
	@(mpirun -np 4 --hostfile runners.host ./launchMPI fa0d1a60ef6616bb28038515c8ea4cb2 6 abcdefghijklmnopqrstuvwxyz 0 1 0)

profile:
	@echo 'Profiling di attacco brute force distribuito. processi = 4,  passwd = falcon'
	@echo 'Redirezione sul file perf.stat'
	@(perf stat -e task-clock,cpu-clock,context-switches,cpu-migrations,page-faults,alignment-faults,cycles,stalled-cycles-frontend,stalled-cycles-backend,instructions,branches,branch-misses,cache-references,cache-misses,L1-dcache-loads,L1-dcache-stores,L1-icache-loads,L1-icache-load-misses,L1-icache-prefetch,L1-icache-prefetch-misses mpirun -np 4 --hostfile runners.host ./launchMPI fa0d1a60ef6616bb28038515c8ea4cb2 6 abcdefghijklmnopqrstuvwxyz 0 1 0) 2>perf.stat


.PHONY: all clean dependents profile
.SECONDARY:

-include ../makefile.targets
