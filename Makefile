CXXFLAGS = -fpermissive -Wall -std=c++17 -O3 -fno-stack-protector -I../xsimd/include

mac: main.cc
	$(CXX) $(CXXFLAGS) -march=native -c main.cc
	$(CXX) $(CXXFLAGS) -march=native -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

stamp: main.cc
	g++ $(CXXFLAGS) -c main.cc
	g++ $(CXXFLAGS) -oprog.elf main.o
	objdump -d -r main.o > dump.as
	rm main.o
	time ./prog.elf

clean:
	rm *.elf *.as
