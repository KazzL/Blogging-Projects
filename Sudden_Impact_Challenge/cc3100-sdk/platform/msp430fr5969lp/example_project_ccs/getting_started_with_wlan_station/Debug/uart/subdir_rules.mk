################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
uart/uart.obj: D:/CC3xxx/CC3100/ReleaseInstallers/v052_June26/CC3100SDK/cc3100-sdk/platform/msp430fr5969/uart.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs_v6/ccsv6/tools/compiler/msp430_4.3.1/bin/cl430" -vmspx --abi=coffabi --include_path="C:/ti/ccs_v6/ccsv6/ccs_base/msp430/include" --include_path="D:/CC3xxx/CC3100/ReleaseInstallers/v052_June26/CC3100SDK/cc3100-sdk/platform/msp430fr5969/" --include_path="D:/CC3xxx/CC3100/ReleaseInstallers/v052_June26/CC3100SDK/cc3100-sdk/simplelink/include" --include_path="D:/CC3xxx/CC3100/ReleaseInstallers/v052_June26/CC3100SDK/cc3100-sdk/simplelink/source" --include_path="C:/ti/ccs_v6/ccsv6/tools/compiler/msp430_4.3.1/include" -g --define=__CCS__ --define=__MSP430FR5969__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="uart/uart.pp" --obj_directory="uart" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


