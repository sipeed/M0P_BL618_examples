SIPEED_SOLUTION_PATH ?= .
BL_SDK_BASE ?= $(realpath $(SIPEED_SOLUTION_PATH)/../../../bouffalo_sdk)
SIPEED_SUPPORT ?= $(BL_SDK_BASE)/../sipeed_support

export BL_SDK_BASE

CHIP ?= bl616
BOARD ?= bl616dk
BOARD_DIR ?= 
CROSS_COMPILE ?= riscv64-unknown-elf-

# add custom cmake definition
#cmake_definition+=-Dxxx=sss

include $(BL_SDK_BASE)/project.build
