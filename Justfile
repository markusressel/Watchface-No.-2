setup:
	uv tool install pebble-tool --python 3.13
	pebble sdk install latest

screenshot:
    pebble screenshot --phone 192.168.2.159

build:
	pebble build

clean-emulator platforms: build kill wipe (emulator "platforms")

[arg("platforms", pattern="basalt|chalk|diorite|emery|flint")]
emulator +platforms: build
    #!/usr/bin/env bash
    for platform in {{platforms}}; do \
      echo "Installing for $platform..."; \
      pebble install --emulator "$platform" --logs; \
    done

kill:
    pebble kill

wipe: kill
    pebble wipe

deploy: build
	pebble install --phone 192.168.2.159 --logs

[arg("platform", pattern="phone|basalt|chalk|diorite|emery|flint")]
logs platform:
    #!/usr/bin/env bash
    if [[ "{{platform}}" == "phone" ]]; then \
      pebble logs --phone 192.168.2.159
    else \
      pebble logs --emulator "{{platform}}"
    fi


clean:
	pebble clean