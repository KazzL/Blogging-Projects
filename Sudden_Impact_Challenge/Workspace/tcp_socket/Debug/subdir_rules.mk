################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmspx --abi=coffabi -O4 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/Users/SAPL-025/Documents/element14-projects-software/Sudden_Impact_Challenge/cc3100-sdk/examples/common" --include_path="C:/Users/SAPL-025/Documents/element14-projects-software/Sudden_Impact_Challenge/cc3100-sdk/simplelink/include" --include_path="C:/Users/SAPL-025/Documents/element14-projects-software/Sudden_Impact_Challenge/cc3100-sdk/simplelink/source" --include_path="C:/Users/SAPL-025/Documents/element14-projects-software/Sudden_Impact_Challenge/cc3100-sdk/platform/msp430f5529lp" --include_path="c:/ti/ccsv6/tools/compiler/msp430_4.3.3/include" -g --define=__CCS__ --define=_USE_CLI_ --define=__MSP430F5529__ --diag_warning=225 --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


