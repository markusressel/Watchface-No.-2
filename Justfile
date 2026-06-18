setup:
    uv tool install pebble-tool --python 3.13
    pebble sdk install latest

[arg("platform", pattern="phone|basalt|chalk|diorite|emery|flint")]
screenshot platform:
    #!/usr/bin/env bash
    if [[ "{{ platform }}" == "phone" ]]; then \
      pebble screenshot --phone 192.168.2.159; \
    else \
      pebble screenshot --emulator "{{ platform }}"; \
    fi

generate:
    ./scripts/generate.py

build: generate
    ./scripts/build.py

release: generate
    PEBBLE_RELEASE=1 ./scripts/build.py --minify

clean-emulator platforms: kill wipe

[arg("mode", pattern="debug|release")]
[arg("platforms", pattern="phone|basalt|chalk|diorite|emery|flint")]
run mode *platforms:
    ./scripts/run.py {{ platforms }} --{{ mode }}

[arg("platform", pattern="basalt|chalk|diorite|emery|flint")]
app-config platform:
    ./scripts/config.py --emulator "{{ platform }}"

kill:
    pebble kill

wipe: kill
    pebble wipe

[arg("mode", pattern="debug|release")]
[arg("platform", pattern="phone|basalt|chalk|diorite|emery|flint")]
deploy mode platform:
    ./scripts/run.py {{ platform }} --logs --{{ mode }}

[arg("platform", pattern="phone|basalt|chalk|diorite|emery|flint")]
logs platform:
    #!/usr/bin/env bash
    if [[ "{{ platform }}" == "phone" ]]; then \
      pebble logs --phone 192.168.2.159; \
    else \
      pebble logs --emulator "{{ platform }}"; \
    fi

clean:
    pebble clean
    find . -name "*.gcda" -delete
    find . -name "*.gcno" -delete
    rm -f coverage.html coverage.css

test: test-c test-js

test-js:
    ./node_modules/.bin/jest tests/js

test-c:
    ./scripts/tests.py

coverage:
    #!/usr/bin/env bash
    set -e
    # Clean old coverage data
    find . -name "*.gcda" -delete
    find . -name "*.gcno" -delete
    
    # Run tests with coverage
    ./scripts/tests.py --coverage
    
    echo ""
    echo "--- Coverage Report ---"
    if command -v gcovr >/dev/null 2>&1; then
      gcovr -r . --filter src/c --print-summary --html --html-details -o coverage.html
      echo "HTML report generated at coverage.html"
    else
      echo "gcovr not found, falling back to basic gcov..."
      # Run gcov on all data files found in tests/build and filter for src/c files
      find tests/build -name "*.gcda" -exec gcov -n -o tests/build/ {} + | grep -A 1 "File 'src/c/" || true
      echo ""
      echo "Tip: Install gcovr (e.g. 'pip install gcovr') for a better summary and HTML reports."
    fi
