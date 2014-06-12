################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../jni/jni_part.cpp 

OBJS += \
./jni/jni_part.o 

CPP_DEPS += \
./jni/jni_part.d 


# Each subdirectory must supply rules for building sources it contributes
jni/%.o: ../jni/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DANDROID=1 -I"/home/abel/android-ndk-r9d/platforms/android-18/arch-arm/usr/include" -I"/home/abel/android-ndk-r9d/sources/cxx-stl/gnu-libstdc++/4.6/include" -I"/home/abel/android-ndk-r9d/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include" -I/home/abel/workspace/OpenCV-2.4.9-android-sdk/sdk/native/jni/include -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


