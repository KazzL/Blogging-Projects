#############################################################################
#
#  CC31xx Host Driver Implementation.
#  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the   
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
###########################################################################


SIMPLE_LINK_STUDIO_DIR = ..//..
SIMPLE_LINK_DIR = ..//..//..//..//simplelink
SRC_DIR = .//
MAIN_C_DIR = ..//..//..//..//examples//sls_get_time
COMPILER_INCLUDE_DIR = C://MinGW//include
SIMPLE_STUDIO_LIB_DIR = $(SIMPLE_LINK_STUDIO_DIR)

MINGW32_MAKE=1

ifdef MINGW32_MAKE 
LINK_DEBUG_DIR =    gccDebug_out
LINK_RELEASE_DIR =  gccRelease_out
OBJ_DEBUG_DIR = $(LINK_DEBUG_DIR)\Obj
OBJ_RELEASE_DIR = $(LINK_RELEASE_DIR)\Obj
else
LINK_DEBUG_DIR =    gccDebug_out
LINK_RELEASE_DIR =  gccRelease_out
OBJ_DEBUG_DIR = $(LINK_DEBUG_DIR)/Obj
OBJ_RELEASE_DIR = $(LINK_RELEASE_DIR)/Obj
endif
APP_NAME= Time.exe

# Compiler flags...
CPP_COMPILER = gcc
C_COMPILER = gcc
C_LINKER = gcc

# Include paths...
Debug_Include_Path=-I"$(COMPILER_INCLUDE_DIR)" -I"$(SIMPLE_LINK_STUDIO_DIR)" -I"$(SIMPLE_LINK_DIR)"  -I"$(SIMPLE_LINK_DIR)/Include"  -I"$(SIMPLE_LINK_DIR)/Source" 
Release_Include_Path=-I"$(COMPILER_INCLUDE_DIR)" -I"$(SIMPLE_LINK_STUDIO_DIR)" -I"$(SIMPLE_LINK_DIR)"  -I"$(SIMPLE_LINK_DIR)/Include"  -I"$(SIMPLE_LINK_DIR)/Source"

# Library paths...
Debug_Library_Path= -L"$(SIMPLE_STUDIO_LIB_DIR)" 
Release_Library_Path=-L"$(SIMPLE_STUDIO_LIB_DIR)"

# Additional libraries...
Debug_Libraries=-Wl,--start-group -l$(OutputPath)SimpleLinkStudio -l SimpleLinkStudio -l ftd2xx -Wl,--end-group
Release_Libraries=-Wl,--start-group -l SimpleLinkStudio -l ftd2xx -Wl,--end-group

# Preprocessor definitions...
Debug_Preprocessor_Definitions=-D GCC_BUILD -D _DEBUG -D _CONSOLE 
Release_Preprocessor_Definitions=-D GCC_BUILD -D NDEBUG -D _CONSOLE 

# Implictly linked object files...
Debug_Implicitly_Linked_Objects=
Release_Implicitly_Linked_Objects=

# Compiler flags...
Debug_Compiler_Flags=-W -O0  -g -w 
Release_Compiler_Flags=-O0 -w

# Builds all configurations for this project...
.PHONY: build_all_configurations
build_all_configurations: Debug Release

# Builds the Debug configuration...
.PHONY: Debug
Debug: create_folders $(OBJ_DEBUG_DIR)/main.o $(OBJ_DEBUG_DIR)/device.o $(OBJ_DEBUG_DIR)/driver.o $(OBJ_DEBUG_DIR)/flowcont.o $(OBJ_DEBUG_DIR)/netapp.o $(OBJ_DEBUG_DIR)/netcfg.o $(OBJ_DEBUG_DIR)/nonos.o $(OBJ_DEBUG_DIR)/fs.o $(OBJ_DEBUG_DIR)/socket.o $(OBJ_DEBUG_DIR)/spawn.o $(OBJ_DEBUG_DIR)/wlan.o 
	$(C_LINKER) $(OBJ_DEBUG_DIR)/main.o $(OBJ_DEBUG_DIR)/device.o $(OBJ_DEBUG_DIR)/driver.o $(OBJ_DEBUG_DIR)/flowcont.o $(OBJ_DEBUG_DIR)/netapp.o $(OBJ_DEBUG_DIR)/netcfg.o $(OBJ_DEBUG_DIR)/nonos.o $(OBJ_DEBUG_DIR)/fs.o $(OBJ_DEBUG_DIR)/socket.o $(OBJ_DEBUG_DIR)/spawn.o $(OBJ_DEBUG_DIR)/wlan.o  $(Debug_Library_Path) $(Debug_Libraries) -Wl,-rpath,./ -o $(LINK_DEBUG_DIR)/$(APP_NAME)

# Compiles file main.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/main.d
$(OBJ_DEBUG_DIR)/main.o: $(MAIN_C_DIR)/main.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(MAIN_C_DIR)/main.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/main.o -DMINGW_ENV=1
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(MAIN_C_DIR)/main.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/main.d -DMINGW_ENV=1

# Compiles file $(SIMPLE_LINK_DIR)/Source/device.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/device.d
$(OBJ_DEBUG_DIR)/device.o: $(SIMPLE_LINK_DIR)/Source/device.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/device.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/device.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/device.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/device.d

# Compiles file ../SimpleLink/Source/driver.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/driver.d
$(OBJ_DEBUG_DIR)/driver.o: $(SIMPLE_LINK_DIR)/Source/driver.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/driver.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/driver.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/driver.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/driver.d

# Compiles file ../SimpleLink/Source/flowcont.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/flowcont.d
$(OBJ_DEBUG_DIR)/flowcont.o: $(SIMPLE_LINK_DIR)/Source/flowcont.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/flowcont.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/flowcont.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/flowcont.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/flowcont.d

# Compiles file ../SimpleLink/Source/netapp.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/netapp.d
$(OBJ_DEBUG_DIR)/netapp.o: $(SIMPLE_LINK_DIR)/Source/netapp.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/netapp.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/netapp.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/netapp.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/netapp.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/netcfg.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/netcfg.d
$(OBJ_DEBUG_DIR)/netcfg.o: $(SIMPLE_LINK_DIR)/Source/netcfg.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/netcfg.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/netcfg.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/netcfg.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/netcfg.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/nonos.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/nonos.d
$(OBJ_DEBUG_DIR)/nonos.o: $(SIMPLE_LINK_DIR)/Source/nonos.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/nonos.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/nonos.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/nonos.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/nonos.d

# Compiles file ../SimpleLink/Source/fs.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/fs.d
$(OBJ_DEBUG_DIR)/fs.o: $(SIMPLE_LINK_DIR)/Source/fs.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/fs.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/fs.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/fs.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/fs.d

# Compiles file ../SimpleLink/Source/socket.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/socket.d
$(OBJ_DEBUG_DIR)/socket.o: $(SIMPLE_LINK_DIR)/Source/socket.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/socket.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/socket.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/socket.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/socket.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/spawn.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/spawn.d
$(OBJ_DEBUG_DIR)/spawn.o: $(SIMPLE_LINK_DIR)/Source/spawn.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/spawn.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/spawn.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/spawn.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/spawn.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/wlan.c for the Debug configuration...
-include $(OBJ_DEBUG_DIR)/wlan.d
$(OBJ_DEBUG_DIR)/wlan.o: $(SIMPLE_LINK_DIR)/Source/wlan.c
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/wlan.c $(Debug_Include_Path) -o $(OBJ_DEBUG_DIR)/wlan.o
	$(C_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/wlan.c $(Debug_Include_Path) > $(OBJ_DEBUG_DIR)/wlan.d



# Builds the Release configuration...
.PHONY: Release
Release: create_folders  $(OBJ_RELEASE_DIR)/main.o $(OBJ_RELEASE_DIR)/device.o $(OBJ_RELEASE_DIR)/driver.o $(OBJ_RELEASE_DIR)/flowcont.o $(OBJ_RELEASE_DIR)/netapp.o $(OBJ_RELEASE_DIR)/netcfg.o $(OBJ_RELEASE_DIR)/nonos.o $(OBJ_RELEASE_DIR)/fs.o $(OBJ_RELEASE_DIR)/socket.o $(OBJ_RELEASE_DIR)/spawn.o $(OBJ_RELEASE_DIR)/wlan.o 
	$(C_LINKER) $(OBJ_RELEASE_DIR)/main.o $(OBJ_RELEASE_DIR)/device.o  $(OBJ_RELEASE_DIR)/driver.o $(OBJ_RELEASE_DIR)/flowcont.o $(OBJ_RELEASE_DIR)/netapp.o $(OBJ_RELEASE_DIR)/netcfg.o $(OBJ_RELEASE_DIR)/nonos.o $(OBJ_RELEASE_DIR)/fs.o $(OBJ_RELEASE_DIR)/socket.o $(OBJ_RELEASE_DIR)/spawn.o $(OBJ_RELEASE_DIR)/wlan.o  $(Release_Library_Path) $(Release_Libraries) -Wl,-rpath,./ -o $(LINK_RELEASE_DIR)/$(APP_NAME)

# Compiles file main.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/main.d
$(OBJ_RELEASE_DIR)/main.o: $(MAIN_C_DIR)/main.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(MAIN_C_DIR)/main.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/main.o -DMINGW_ENV=1
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(MAIN_C_DIR)/main.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/main.d -DMINGW_ENV=1

# Compiles file $(SIMPLE_LINK_DIR)/Source/device.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/device.d
$(OBJ_RELEASE_DIR)/device.o: $(SIMPLE_LINK_DIR)/Source/device.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/device.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/device.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/device.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/device.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/driver.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/driver.d
$(OBJ_RELEASE_DIR)/driver.o: $(SIMPLE_LINK_DIR)/Source/driver.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/driver.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/driver.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/driver.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/driver.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/flowcont.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/flowcont.d
$(OBJ_RELEASE_DIR)/flowcont.o: $(SIMPLE_LINK_DIR)/Source/flowcont.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/flowcont.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/flowcont.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/flowcont.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/flowcont.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/netapp.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/netapp.d
$(OBJ_RELEASE_DIR)/netapp.o: $(SIMPLE_LINK_DIR)/Source/netapp.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/netapp.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/netapp.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/netapp.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/netapp.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/netcfg.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/netcfg.d
$(OBJ_RELEASE_DIR)/netcfg.o: $(SIMPLE_LINK_DIR)/Source/netcfg.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/netcfg.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/netcfg.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/netcfg.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/netcfg.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/nonos.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/nonos.d
$(OBJ_RELEASE_DIR)/nonos.o: $(SIMPLE_LINK_DIR)/Source/nonos.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/nonos.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/nonos.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/nonos.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/nonos.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/fs.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/fs.d
$(OBJ_RELEASE_DIR)/fs.o: $(SIMPLE_LINK_DIR)/Source/fs.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/fs.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/fs.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/fs.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/fs.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/socket.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/socket.d
$(OBJ_RELEASE_DIR)/socket.o: $(SIMPLE_LINK_DIR)/Source/socket.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/socket.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/socket.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/socket.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/socket.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/spawn.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/spawn.d
$(OBJ_RELEASE_DIR)/spawn.o: $(SIMPLE_LINK_DIR)/Source/spawn.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/spawn.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/spawn.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/spawn.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/spawn.d

# Compiles file $(SIMPLE_LINK_DIR)/Source/wlan.c for the Release configuration...
-include $(OBJ_RELEASE_DIR)/wlan.d
$(OBJ_RELEASE_DIR)/wlan.o: $(SIMPLE_LINK_DIR)/Source/wlan.c
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c $(SIMPLE_LINK_DIR)/Source/wlan.c $(Release_Include_Path) -o $(OBJ_RELEASE_DIR)/wlan.o
	$(C_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM $(SIMPLE_LINK_DIR)/Source/wlan.c $(Release_Include_Path) > $(OBJ_RELEASE_DIR)/wlan.d
	

ifdef MINGW32_MAKE 

# Creates the intermediate and output folders for each configuration...
.PHONY: create_folders
create_folders:
	if not exist $(LINK_DEBUG_DIR)	mkdir $(LINK_DEBUG_DIR)
	if not exist $(OBJ_DEBUG_DIR)	mkdir $(OBJ_DEBUG_DIR)
	if not exist $(LINK_RELEASE_DIR)	mkdir $(LINK_RELEASE_DIR)
	if not exist $(OBJ_RELEASE_DIR)	mkdir $(OBJ_RELEASE_DIR)

# Cleans intermediate and output files (objects, libraries, executables)...
.PHONY: clean
clean:     
	if exist $(LINK_DEBUG_DIR)   rd  $(LINK_DEBUG_DIR) /s /q
	if exist $(LINK_RELEASE_DIR) rd  $(LINK_RELEASE_DIR) /s /q

else

# Creates the intermediate and output folders for each configuration...
.PHONY:create_folders
create_folders:
	test -d $(LINK_DEBUG_DIR)        || mkdir $(LINK_DEBUG_DIR)    
	test -d $(OBJ_DEBUG_DIR)         || mkdir $(OBJ_DEBUG_DIR)     
	test -d $(LINK_RELEASE_DIR)      || mkdir $(LINK_RELEASE_DIR)  
	test -d $(OBJ_RELEASE_DIR)       || mkdir $(OBJ_RELEASE_DIR)   
# Cleans intermediate and output files (objects, libraries, executables)...
.PHONY:clean
clean:
	if test -d $(LINK_DEBUG_DIR)    ;  then rm -rf     del  $(LINK_DEBUG_DIR)    ;fi;
	if test -d $(LINK_RELEASE_DIR)  ;  then rm -rf     del  $(LINK_RELEASE_DIR)  ;fi;
endif
.PHONY: all
all: build_all_configurations
