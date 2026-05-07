################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/app/dsp_filter.c \
../source/app/game_logic.c 

C_DEPS += \
./source/app/dsp_filter.d \
./source/app/game_logic.d 

OBJS += \
./source/app/dsp_filter.o \
./source/app/game_logic.o 


# Each subdirectory must supply rules for building sources it contributes
source/app/%.o: ../source/app/%.c source/app/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MKL46Z256VLL4_cm0plus -DCPU_MKL46Z256VLL4 -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/board" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/source" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/drivers" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/CMSIS" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/startup" -I"/Users/y1hhnn/Documents/MCUXpressoIDE_25.6.136/workspace/MKL46Z4_Project/utilities" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source-2f-app

clean-source-2f-app:
	-$(RM) ./source/app/dsp_filter.d ./source/app/dsp_filter.o ./source/app/game_logic.d ./source/app/game_logic.o

.PHONY: clean-source-2f-app

