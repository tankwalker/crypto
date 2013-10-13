################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../sh/shell.c \

SHELL_OBJS += \
./sh/shell.o 

C_DEPS += \
./sh/io.d 


# Each subdirectory must supply rules for building sources it contributes
sh/%.o: ../sh/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc $(CFLAGS) -I/home/mpiuser/git/crypto/Crypto/common -I/home/mpiuser/git/crypto/Crypto/sh -I/home/mpiuser/git/crypto/Crypto/mpi -I/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.0/include -I/usr/include -I/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.0/include-fixed -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '