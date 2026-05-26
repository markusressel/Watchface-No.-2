.PHONY: build emulator kill wipe deploy clean

setup:
	just setup

build:
	just build

emulator: build
	just emulator

deploy: build
	just install

clean:
	just clean