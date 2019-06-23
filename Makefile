CXXFLAGS = -std=c++17 -O3 -march=native -fno-stack-protector -I../../xsimd/include

mac: main.cc
	$(CXX) $(CXXFLAGS) -c main.cc
	$(CXX) $(CXXFLAGS) -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

stamp: main.cc
	icpc $(CXXFLAGS) -xMIC-AVX512 -c main.cc
	icpc $(CXXFLAGS) -xMIC-AVX512 -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

clean:
	rm *.elf *.as
