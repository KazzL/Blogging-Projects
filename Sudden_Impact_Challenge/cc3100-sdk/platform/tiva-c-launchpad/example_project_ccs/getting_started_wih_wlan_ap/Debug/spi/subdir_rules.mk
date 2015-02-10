################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
spi/spi.obj: D:/CC3xxx/CC3100/CC3100\ SDK/CC3100\ 1.0\ pre/SDK/platform/tiva-c-launchpad/spi.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/CCS/ccsv6/tools/compiler/arm_5.1.5/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --opt_for_speed=2 --include_path="C:/ti/CCS/ccsv6/tools/compiler/arm_5.1.5/include" --include_path="D:/CC3xxx/CC3100/CC3100 SDK/CC3100 1.0 pre/SDK/examples/common" --include_path="C:/ti/TivaWare_C_Series-2.1.0.12573/" --include_path="D:/CC3xxx/CC3100/CC3100 SDK/CC3100 1.0 pre/SDK/simplelink/include" --include_path="D:/CC3xxx/CC3100/CC3100 SDK/CC3100 1.0 pre/SDK/simplelink/source" --include_path="D:/CC3xxx/CC3100/CC3100 SDK/CC3100 1.0 pre/SDK/platform/tiva-c-launchpad/" -g --define=PART_TM4C123GH6PM --define=TARGET_IS_BLIZZARD_RA1=1 --define=_USE_CLI_ --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="spi/spi.pp" --obj_directory="spi" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


