################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
cli_uart/cli_uart.obj: C:/Users/Kas\ Windows/Documents/element14-projects-software/Sudden_Impact_Challenge/platform/msp430f5529lp/cli_uart.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmspx --abi=coffabi -O4 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/Users/Kas Windows/Documents/element14-projects-software/Sudden_Impact_Challenge/examples/common" --include_path="C:/Users/Kas Windows/Documents/element14-projects-software/Sudden_Impact_Challenge/simplelink/include" --include_path="C:/Users/Kas Windows/Documents/element14-projects-software/Sudden_Impact_Challenge/simplelink/source" --include_path="C:/Users/Kas Windows/Documents/element14-projects-software/Sudden_Impact_Challenge/platform/msp430f5529lp" --include_path="c:/ti/ccsv6/tools/compiler/msp430_4.3.3/include" -g --define=__CCS__ --define=_USE_CLI_ --define=__MSP430F5529__ --diag_warning=225 --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="cli_uart/cli_uart.pp" --obj_directory="cli_uart" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


