.PHONY: build emulator kill wipe deploy clean

setup:
	just setup

build:
	just build

emulator:
	just emulator

deploy:
	just install

clean:
	just clean