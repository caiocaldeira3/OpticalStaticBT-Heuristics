.PHONY: test build

build:
	cmake --build build

test:
	ctest --test-dir build --output-on-failure