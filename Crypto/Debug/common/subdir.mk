################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../common/hash.c \
../common/mem.c \
../common/memutils.c \
../common/csignal.c \
../common/sym.c 

OBJS += \
./common/hash.o \
./common/mem.o \
./common/memutils.o \
./common/csignal.o \
./common/sym.o 

C_DEPS += \
./common/hash.d \
./common/mem.d \
./common/memutils.d \
./common/csignal.d \
./common/sym.d 


# Each subdirectory must supply rules for building sources it contributes
common/%.o: ../common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	mpicc $(CFLAGS) -I/home/mpiuser/git/crypto/Crypto/common -I/home/mpiuser/git/crypto/Crypto/sh -I/home/mpiuser/git/crypto/Crypto/mpi -I/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.0/include -I/usr/include -I/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.0/include-fixed -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


