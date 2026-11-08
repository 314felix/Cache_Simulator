#!/usr/bin/env bash

set -euo pipefail

# Cache sizes to test, in bytes
sizes=(65536 131072 262144 524288 1048576 2097152)
ways=(1 2 4 8 16) 
block_sizes=(4 8 16 32 64 128 256)


for size in "${sizes[@]}"; do

    echo "Running benchmark with L1D size: $size ways"

    # Generate the cache configuration
    cat > cache_config.txt <<EOF
4096 16 2 0 1
32768 64 8 0 1
$size 64 8 0 1
EOF

    ./cachesim 

done

echo "All benchmarks completed!"