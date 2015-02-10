################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/CCS/ccsv6/tools/compiler/arm_5.1.5

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./board/board.obj" \
"./cli_uart/cli_uart.obj" \
"./simplelink/source/wlan.obj" \
"./simplelink/source/spawn.obj" \
"./simplelink/source/socket.obj" \
"./simplelink/source/nonos.obj" \
"./simplelink/source/netcfg.obj" \
"./simplelink/source/netapp.obj" \
"./simplelink/source/fs.obj" \
"./simplelink/source/flowcont.obj" \
"./simplelink/source/driver.obj" \
"./simplelink/source/device.obj" \
"./spi/spi.obj" \
"./uart/uart.obj" \
"./tm4c123gh6pm_startup_ccs.obj" \
"./main.obj" \
"../tm4c123gh6pm.cmd" \
-l"libc.a" \
-lC:/ti/TivaWare_C_Series-2.1.0.12573\driverlib\ccs\Debug\driverlib.lib \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include uart/subdir_vars.mk
-include spi/subdir_vars.mk
-include simplelink/source/subdir_vars.mk
-include cli_uart/subdir_vars.mk
-include board/subdir_vars.mk
-include subdir_rules.mk
-include uart/subdir_rules.mk
-include spi/subdir_rules.mk
-include simplelink/source/subdir_rules.mk
-include cli_uart/subdir_rules.mk
-include board/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
getting_started_with_wlan_station.out \

EXE_OUTPUTS__QUOTED += \
"getting_started_with_wlan_station.out" \

BIN_OUTPUTS += \
getting_started_with_wlan_station.hex \

BIN_OUTPUTS__QUOTED += \
"getting_started_with_wlan_station.hex" \


# All Target
all: getting_started_with_wlan_station.out

# Tool invocations
getting_started_with_wlan_station.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"C:/ti/CCS/ccsv6/tools/compiler/arm_5.1.5/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -Ooff --opt_for_speed=2 -g --define=PART_TM4C123GH6PM --define=_USE_CLI_ --define=TARGET_IS_BLIZZARD_RA1=1 --display_error_number --diag_warning=225 --diag_wrap=off -z -m"getting_started_with_wlan_station.map" --heap_size=0 --stack_size=600 -i"C:/ti/CCS/ccsv6/tools/compiler/arm_5.1.5/lib" -i"C:/ti/CCS/ccsv6/tools/compiler/arm_5.1.5/include" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="getting_started_with_wlan_station_linkInfo.xml" --rom_model -o "getting_started_with_wlan_station.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

getting_started_with_wlan_station.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/CCS/ccsv6/tools/compiler/arm_5.1.5/bin/armhex"  -o "getting_started_with_wlan_station.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)$(BIN_OUTPUTS__QUOTED)
	-$(RM) "main.pp" "tm4c123gh6pm_startup_ccs.pp" "uart\uart.pp" "spi\spi.pp" "simplelink\source\device.pp" "simplelink\source\driver.pp" "simplelink\source\flowcont.pp" "simplelink\source\fs.pp" "simplelink\source\netapp.pp" "simplelink\source\netcfg.pp" "simplelink\source\nonos.pp" "simplelink\source\socket.pp" "simplelink\source\spawn.pp" "simplelink\source\wlan.pp" "cli_uart\cli_uart.pp" "board\board.pp" 
	-$(RM) "main.obj" "tm4c123gh6pm_startup_ccs.obj" "uart\uart.obj" "spi\spi.obj" "simplelink\source\device.obj" "simplelink\source\driver.obj" "simplelink\source\flowcont.obj" "simplelink\source\fs.obj" "simplelink\source\netapp.obj" "simplelink\source\netcfg.obj" "simplelink\source\nonos.obj" "simplelink\source\socket.obj" "simplelink\source\spawn.obj" "simplelink\source\wlan.obj" "cli_uart\cli_uart.obj" "board\board.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

