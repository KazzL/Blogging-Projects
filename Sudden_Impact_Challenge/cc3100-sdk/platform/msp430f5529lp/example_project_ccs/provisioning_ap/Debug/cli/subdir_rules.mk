################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
cli/cli_uart.obj: D:/CC3xxx/CC3100/CC3100\ SDK/cc3100\ 0.5\ pre/SDK/platform/cli_uart.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/CCS/ccsv5/tools/compiler/msp430_4.2.1/bin/cl430" -vmspx --abi=coffabi --include_path="C:/ti/CCS/ccsv5/ccs_base/msp430/include" --include_path="D:/CC3xxx/CC3100/CC3100 SDK/cc3100 0.5 pre/simplelink/include" --include_path="D:/CC3xxx/CC3100/CC3100 SDK/cc3100 0.5 pre/simplelink/source" --include_path="D:/CC3xxx/CC3100/CC3100 SDK/cc3100 0.5 pre/SDK/platform/" --include_path="C:/ti/CCS/ccsv5/tools/compiler/msp430_4.2.1/include" --advice:power=all -g --define=__MSP430F5529__ --define=__CCS__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="cli/cli_uart.pp" --obj_directory="cli" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


