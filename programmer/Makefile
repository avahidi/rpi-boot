
SRC=$(wildcard src/*.c)
INC=$(wildcard src/*.h) 
OUT=build/prog

all: $(OUT).exe


$(OUT).exe: build Makefile $(SRC) $(INC)
	gcc $(SRC) -o $(OUT).exe

#

test: all
	$(OUT).exe -o /dev/ttyUSB0
	
#

build:
	mkdir build

clean:
	rm -rf build
