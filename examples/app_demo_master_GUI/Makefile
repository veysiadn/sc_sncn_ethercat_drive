FLAGS = -static
FLAGS += -std=c99
FLAGS += `pkg-config --cflags gtk+-3.0`

LIBS += -lsncn_ctrlproto -lsncn_motor_drive
LIBS += -lethercat -lm -lpthread


# Installation directory of the  EtherCAT Master by IgH EtherLAB
ETHERCAT_MASTER_INSTALLDIR := /opt/etherlab

ETHERCAT_DRIVE_PATH := ../../.
MOTORCONTROL_PATH := ../../.

ifneq (,$(wildcard ../../../sc_sncn_motorcontrol/.))
	MOTORCONTROL_PATH := ../../../sc_sncn_motorcontrol
endif

INCLUDE_DIRS += -I $(ETHERCAT_MASTER_INSTALLDIR)/include
INCLUDE_DIRS += -I ../config_motor_master

INCLUDE_DIRS += -I $(ETHERCAT_DRIVE_PATH)/inc_ctrlproto-common
INCLUDE_DIRS += -I $(ETHERCAT_DRIVE_PATH)/lib_linux_ctrlproto/include
INCLUDE_DIRS += -I $(ETHERCAT_DRIVE_PATH)/lib_linux_motor_drive/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_profile/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_motorcontrol_common/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_pwm_symmetrical/src/common
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_hall/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_qei/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_biss/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_ams_rotary_sensor/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_gpio/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_motorcontrol/include
INCLUDE_DIRS += -I $(MOTORCONTROL_PATH)/module_misc/include

LIB_DIRS += -L $(ETHERCAT_MASTER_INSTALLDIR)/lib
LIB_DIRS += -L $(ETHERCAT_DRIVE_PATH)/lib_linux_ctrlproto/lib
LIB_DIRS += -L $(ETHERCAT_DRIVE_PATH)/lib_linux_motor_drive/lib
LIBS_DIRS += `pkg-config --libs gtk+-3.0`

ODIR = obj
SDIR = src
BINDIR = bin

OFILES += main.o
OFILES += user_application.o

# Needed for checking if Linux machine
UNAME := $(shell uname)

all:  $(OFILES)
		@echo Bulding dependency: lib_linux_ctrlproto...
		@cd $(ETHERCAT_DRIVE_PATH)/lib_linux_ctrlproto; make
		@echo 
		@echo Bulding dependency: lib_linux_motor_drive...
		@cd $(ETHERCAT_DRIVE_PATH)/lib_linux_motor_drive; make
		@echo 
		@echo Bulding application...
ifneq "$(UNAME)" "Linux"
		@echo
		@echo "------------------------------------------------------------------------------------------------------"
		@echo "||Your System isn't Linux, so the Master Application for the Linux EtherCAT Master can not be built.||"
		@echo "------------------------------------------------------------------------------------------------------"
		@echo
else
		@if test -d $(ETHERCAT_MASTER_INSTALLDIR); \
		then \
		mkdir bin -p;\
		gcc $(ODIR)/main.o $(ODIR)/user_application.o $(LIBS) $(INCLUDE_DIRS) $(LIB_DIRS) $(FLAGS) -o $(BINDIR)/$(notdir $(shell pwd)); \
		echo ;\
		echo "Run your application by typing: sudo bin/$(notdir $(shell pwd))" ;\
		echo ;\
		else echo \
		"\n\
		-----------------------------------------------------------------------------------------------------------------\n\
		||EtherCAT Master is not installed at given directory if you installed it elsewhere change location in Makefile||\n\
		-----------------------------------------------------------------------------------------------------------------\n";\
		fi
endif

main.o: $(SDIR)/main.c
	gcc -c $(SDIR)/main.c $(LIBS) $(INCLUDE_DIRS) $(LIB_DIRS) $(FLAGS) -o $(ODIR)/main.o

user_application.o: $(SDIR)/user_application.c
	gcc -c $(SDIR)/user_application.c $(LIBS) $(INCLUDE_DIRS) $(LIB_DIRS) $(FLAGS) -o $(ODIR)/user_application.o

clean:
	rm -f $(BINDIR)/$(notdir $(shell pwd))
	rm -f $(ODIR)/*
