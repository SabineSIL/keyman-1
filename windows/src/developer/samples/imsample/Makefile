#
# imsample Makefile
#

VERSION_TXT_PATH=..\..

!include ..\..\..\Defines.mak

build: dirs version.res
    $(MSBUILD) imsample.sln $(MSBUILD_BUILD)
    $(COPY) $(WIN32_TARGET_PATH)\imsample.dll $(PROGRAM)\developer
    $(COPY) $(WIN32_TARGET_PATH)\imsample.pdb $(PROGRAM)\developer

signcode:
    $(SIGNCODE) /d "Keyman IMX Sample" $(PROGRAM)\developer\imsample.dll

wrap-symbols:
    $(SYMSTORE) $(PROGRAM)\developer\imsample.dll /t keyman-developer
    $(SYMSTORE) $(DEBUGPATH)\developer\imsample.pdb /t keyman-developer

clean: def-clean
    $(MSBUILD) imsample.sln $(MSBUILD_CLEAN)

install:
    @rem

!include ..\..\..\Target.mak
