################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/drivers/mma8451.c \
../source/drivers/proto.c \
../source/drivers/timer_pit.c \
../source/drivers/uart_comm.c

C_DEPS += \
./source/drivers/mma8451.d \
./source/drivers/proto.d \
./source/drivers/timer_pit.d \
./source/drivers/uart_comm.d

OBJS += \
./source/drivers/mma8451.o \
./source/drivers/proto.o \
./source/drivers/timer_pit.o \
./source/drivers/uart_comm.o


# Each subdirectory must supply rules for building sources it contributes
source/drivers/%.o: ../source/drivers/%.c source/drivers/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MKL46Z256VLL4_cm0plus -DCPU_MKL46Z256VLL4 -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/board" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/source" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/drivers" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/CMSIS" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/startup" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/utilities" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source-2f-drivers

clean-source-2f-drivers:
	-$(RM) ./source/drivers/mma8451.d ./source/drivers/mma8451.o ./source/drivers/proto.d ./source/drivers/proto.o ./source/drivers/timer_pit.d ./source/drivers/timer_pit.o ./source/drivers/uart_comm.d ./source/drivers/uart_comm.o

.PHONY: clean-source-2f-drivers

