################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/driver/Spi_Manager/SPI.c 

OBJS += \
./Core/driver/Spi_Manager/SPI.o 

C_DEPS += \
./Core/driver/Spi_Manager/SPI.d 


# Each subdirectory must supply rules for building sources it contributes
Core/driver/Spi_Manager/%.o Core/driver/Spi_Manager/%.su Core/driver/Spi_Manager/%.cyclo: ../Core/driver/Spi_Manager/%.c Core/driver/Spi_Manager/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-driver-2f-Spi_Manager

clean-Core-2f-driver-2f-Spi_Manager:
	-$(RM) ./Core/driver/Spi_Manager/SPI.cyclo ./Core/driver/Spi_Manager/SPI.d ./Core/driver/Spi_Manager/SPI.o ./Core/driver/Spi_Manager/SPI.su

.PHONY: clean-Core-2f-driver-2f-Spi_Manager

