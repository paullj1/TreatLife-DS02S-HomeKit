
MAKE_DIR   := $(PWD)
FQBN       ?= esp8266:esp8266:generic:xtal=160,vt=flash,exception=disabled,ResetMethod=nodemcu,CrystalFreq=26,FlashFreq=40,FlashMode=dout,eesz=2M,led=2,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=115200
V          ?= 0
VFLAG      =

ifeq "$(V)" "1"
VFLAG      =-v
endif

ifndef SERIAL_DEV
  ifneq (,$(wildcard /dev/ttyUSB0))
    SERIAL_DEV = /dev/ttyUSB0
  else ifneq (,$(wildcard /dev/ttyACM0))
    SERIAL_DEV = /dev/ttyACM0
  else ifneq (,$(wildcard /dev/tty.usbserial-1410))
    SERIAL_DEV = /dev/tty.usbserial-1410
  else
    SERIAL_DEV = unknown
  endif
endif

BUILD_DIR  := $(subst :,.,build/esp8266:esp8266:generic)

NAME       := $(shell basename $(PWD))
SRC        := $(NAME).ino
HDRS       := $(wildcard *.h)
BIN        := $(BUILD_DIR)/$(SRC).bin
DEPLOY     := $(NAME).bin
ELF        := $(BUILD_DIR)/$(SRC).elf
LIBRARIES  := ~/Documents/Arduino/libraries/

$(info FQBN       is [${FQBN}])
$(info V          is [${V}])
$(info VFLAG      is [${VFLAG}])
$(info MAKE_DIR   is [${MAKE_DIR}])
$(info BUILD_DIR  is [${BUILD_DIR}])
$(info NAME       is [${NAME}])
$(info SRC        is [${SRC}])
$(info HDRS       is [${HDRS}])
$(info BIN        is [${BIN}])
$(info DEPLOY     is [${DEPLOY}])
$(info SERIAL_DEV is [${SERIAL_DEV}])

all: $(ELF)
.PHONY: all

compile: $(ELF)
.PHONY: compile

$(ELF): $(SRC) $(HDRS)
	arduino-cli compile -b $(FQBN) $(VFLAG) --libraries $(LIBRARIES) -e ./
	mv $(BIN) $(DEPLOY)

upload:
	@if [ ! -c $(SERIAL_DEV) ] ; \
	then echo "---> ERROR: Serial Device not available, please set the SERIAL_DEV environment variable" ; \
	else echo "---> Uploading sketch\n"; \
	arduino-cli upload -b $(FQBN) -p $(SERIAL_DEV) $(VFLAG); \
	fi

clean:
	@echo "---> Cleaning the build directory"
	rm -rf build $(DEPLOY)

requirements:
	@if [ -e ../requirements.txt ]; \
	then while read -r i ; do echo ; \
	  echo "---> Installing " '"'$$i'"' ; \
	  arduino-cli lib install "$$i" ; \
	done < ../requirements.txt ; \
	else echo "---> MISSING requirements.txt file"; \
	fi
