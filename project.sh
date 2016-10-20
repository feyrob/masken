enable_pch=false
enable_compile=false

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


end_time=$(date +%s.%N)
duration=$(echo "$end_time - $start_time" | bc)
echo $duration

