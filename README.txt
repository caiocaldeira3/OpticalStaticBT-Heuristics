### Generate build files
cmake -S . -B build

### Build project
cmake --build build

### Run tests
ctest --test-dir build --output-on-failure

### Running graph bisection algorithm
./bin/run --max-depth 20 --algorithm mloga --dataset-name datasets/tor/tor_128.txt --output-directory output/ancestral