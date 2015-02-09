################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
main.obj: D:/CC3xxx/CC3100/CC3100\ SDK/CC3100\ 1.0/SDK/examples/get_time/main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/CCS/ccsv6/tools/compiler/msp430_4.3.2/bin/cl430" -vmspx --abi=eabi -O2 --include_path="C:/ti/CCS/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/CCS/ccsv6/tools/compiler/msp430_4.3.2/include" --advice:power=all --define=__MSP430FR5739__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


