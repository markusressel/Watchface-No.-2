setup:
	uv tool install pebble-tool --python 3.13
	pebble sdk install latest

screenshot:
    pebble screenshot --phone 192.168.2.159

build:
	pebble build

clean-emulator platform: build kill wipe (emulator "platform")

emulator platform: build
	pebble install --emulator {{platform}} --logs

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