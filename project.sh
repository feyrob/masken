start_time=$(date +%s.%N)
ex/precompile_platform_header.sh

ex/build.sh

end_time=$(date +%s.%N)

# duration=$(echo "$end_time - $start_time" | bc)

