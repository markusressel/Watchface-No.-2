.PHONY: build emulator deploy clean

build:
	pebble build

emulator: build
	pebble install --emulator basalt

deploy: build
	pebble install --phone 192.168.2.159

clean:
	pebble clean