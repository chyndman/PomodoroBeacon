all: compile

SRCS := $(wildcard src/*)
POMOBCN_INO := PomodoroBeacon.ino
COMPILE_STAMP := .compile-stamp

TARGET_FQBN := SparkFun:samd:samd51_thing_plus
TARGET_PORT ?= /dev/ttyACM0

compile: $(COMPILE_STAMP)

$(COMPILE_STAMP): $(POMOBCN_INO) $(SRCS)
	rm -f $(COMPILE_STAMP)
	arduino-cli compile --warnings all --fqbn $(TARGET_FQBN) --libraries lib/ .
	touch $@

upload: $(COMPILE_STAMP)
	arduino-cli upload -p $(TARGET_PORT) --fqbn $(TARGET_FQBN) .
