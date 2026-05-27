setup:
	uv tool install pebble-tool --python 3.13
	pebble sdk install latest

build:
	pebble build

clean-emulator: build kill wipe emulator

emulator: build
	pebble install --emulator basalt

kill:
    pebble kill

wipe: kill
    pebble wipe

deploy: build
	pebble install --phone 192.168.2.159 --logs

logs:
    pebble logs --phone 192.168.2.159

clean:
	pebble clean