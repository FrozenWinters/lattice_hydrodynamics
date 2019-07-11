CXXFLAGS = -O3 -Wall -std=c++17

mac: src/main.cc
	$(CXX) $(CXXFLAGS) -march=native -c src/main.cc
	$(CXX) $(CXXFLAGS) -march=native -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

sgcc: main.cc
	g++ $(CXXFLAGS) -fpermissive -march=native -c main.cc
	g++ $(CXXFLAGS) -fpermissive -march=native -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

sint: main.cc
	icpc $(CXXFLAGS) -xMIC-AVX512 -c main.cc
	icpc $(CXXFLAGS) -xMIC-AVX512 -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

clean:
	rm *.elf *.as
