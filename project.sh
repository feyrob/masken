set -e
echo "#############################################################################"
enable_pch=false
enable_compile=false
enable_run=false

if [ $1 = "pch" ]; then
	enable_pch=true
fi

if [ $1 = "iterate" ]; then
	enable_compile=true
fi

if [ $1 = "build" ]; then
	enable_pch=true
	enable_compile=true
fi

if [ $1 = "run" ]; then
	enable_run=true
fi

if [ $1 = "brun" ]; then
	enable_pch=true
	enable_compile=true
	enable_run=true
fi


start_time=$(date +%s.%N)


if [ $enable_pch = "true" ]; then
	echo pch
	mkdir -p temp
	time clang++ -g -std=c++1y -x c++-header src/platform.h -o temp/platform.h.pch -Wno-pragma-once-outside-header
fi

if [ $enable_compile = "true" ]; then
	echo compile
	mkdir -p build
	time clang++ -g -std=c++1y -include temp/platform.h src/unity.cpp -o build/masken -lglfw 
fi


build_end_time=$(date +%s.%N)
build_duration=$(echo "$build_end_time - $start_time" | bc)
echo build time: $build_duration

if [ $enable_run = "true" ]; then
	echo run
	build/masken
fi

