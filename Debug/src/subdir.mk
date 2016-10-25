################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Dictionary.cpp \
../src/MultiByteCharList.cpp \
../src/NMD.cpp \
../src/PRDC.cpp \
../src/main.cpp \
../src/util.cpp 

OBJS += \
./src/Dictionary.o \
./src/MultiByteCharList.o \
./src/NMD.o \
./src/PRDC.o \
./src/main.o \
./src/util.o 

CPP_DEPS += \
./src/Dictionary.d \
./src/MultiByteCharList.d \
./src/NMD.d \
./src/PRDC.d \
./src/main.d \
./src/util.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -fopenmp -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


