################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mpi/crypto.c \
../mpi/part.c  \
../mpi/dictionary.c

CRYPTO_OBJS += \
./mpi/crypto.o \
./mpi/part.o \
./mpi/dictionary.o 

C_DEPS += \
./mpi/crypto.d \
./mpi/part.d \
./mpi/dictionary.d 


# Each subdirectory must supply rules for building sources it contributes
mpi/crypto.o: ../mpi/crypto.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	mpicc -I/home/mpiuser/git/crypto/Crypto/common -I/home/mpiuser/git/crypto/Crypto/sh -I/home/mpiuser/git/crypto/Crypto/mpi -I/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.0/include -I/usr/include -I/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.0/include-fixed -O0 -g3 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"mpi/crypto.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

mpi/%.o: ../mpi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	mpicc -I/home/mpiuser/git/crypto/Crypto/common -I/home/mpiuser/git/crypto/Crypto/sh -I/home/mpiuser/git/crypto/Crypto/mpi -I/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.0/include -I/usr/include -I/usr/lib/gcc/x86_64-unknown-linux-gnu/4.8.0/include-fixed -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


