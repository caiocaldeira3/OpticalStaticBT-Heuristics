### Building
cmake -S . -B build
cmake --build build

### Run tests
./bin/test

### Running graph bisection algorithm
./bin/run --max-depth 20 --algorithm mloga --dataset-name datasets/tor/tor_128.txt --output-directory output/ancestral