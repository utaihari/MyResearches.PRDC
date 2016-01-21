################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Dictionary.cpp \
../src/Lzw.cpp \
../src/main.cpp \
../src/tools.cpp 

OBJS += \
./src/Dictionary.o \
./src/Lzw.o \
./src/main.o \
./src/tools.o 

CPP_DEPS += \
./src/Dictionary.d \
./src/Lzw.d \
./src/main.d \
./src/tools.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


