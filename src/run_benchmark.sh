#!/usr/bin/env bash

set -euo pipefail

# Cache sizes to test, in bytes
sizes=(4096 8192 16384 32768 65536)



for size in "${sizes[@]}"; do

    echo "Running benchmark with L1D size: $size bytes"

    # Generate the cache configuration
    cat > cache_config.txt <<EOF
4096 16 2 0 1
$size 64 8 0 1
524288 64 8 0 1
EOF

    ./cachesim 

done

echo "All benchmarks completed!"