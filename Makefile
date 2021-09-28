upload:
	pio run --target upload -e esp32dev
test_native:
	pio test -e native

# Port to monitor on
PORT = /dev/cu.SLAB_USBtoUART # on MacOS
monitor:
	pio device monitor --port $(PORT)
