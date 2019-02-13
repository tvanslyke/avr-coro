release/firmware.hex: release/firmware.elf
	avr-objcopy -O ihex -R .eeprom release/firmware.elf release/firmware.hex

build_firmware:
	cd src/ && $(MAKE)

release/firmware.elf: build_firmware
	cp src/firmware.elf release/firmware.elf

arduino/libarduino.a: ./arduino/*.h ./arduino/*.cpp ./arduino/*.c
	cd arduino/ && $(MAKE)

install: release/firmware.elf
	avrdude -v -D -patmega328p -carduino -b57600 -C/usr/share/arduino/hardware/tools/avrdude.conf -P/dev/ttyUSB0 -Uflash:w:release/firmware.hex:i

serial:
	picocom --echo --omap=crlf --baud=115200 /dev/ttyUSB0
	# python3 serialcomm.py

coro_asm:
	cd src && $(MAKE) coro_asm

clean:
	cd src/ && $(MAKE) clean
	rm release/firmware.elf
	rm release/firmware.hex

