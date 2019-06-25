CXXFLAGS = -xMIC-AVX512 -Wall -std=c++17 -O3 -I../xsimd/include

mac: main.cc
	$(CXX) $(CXXFLAGS) -march=native -c main.cc
	$(CXX) $(CXXFLAGS) -march=native -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

sgcc: main.cc
	g++ $(CXXFLAGS) -c main.cc
	g++ $(CXXFLAGS) -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

sint: main.cc
	icpc $(CXXFLAGS) -c main.cc
	icpc $(CXXFLAGS) -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

clean:
	rm *.elf *.as
