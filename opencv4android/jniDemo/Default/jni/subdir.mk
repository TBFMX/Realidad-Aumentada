################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../jni/code.cc 

CC_DEPS += \
./jni/code.d 

OBJS += \
./jni/code.o 


# Each subdirectory must supply rules for building sources it contributes
jni/%.o: ../jni/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/opencv -I/home/abel/android-ndk-r9d/platforms/android-18/arch-arm/usr/include -I/home/abel/android-ndk-r9d/platforms/android-18/arch-arm/usr/include/linux -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


