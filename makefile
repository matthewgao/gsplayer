CFLAGS=`pkg-config --cflags libavformat libavutil sdl2`
LIBS= `pkg-config --libs libavformat libavutil sdl2`
STD=-std=c++0x

SRC_DIR :=.
OBJ_DIR :=.
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

# SRC_FILES=display.cpp main.cpp
# OBJ_FILES=display.o main.o

# test:
# 	echo $(SRC_DIR)
# 	echo $(OBJ_DIR)
# 	echo ${SRC_FILES}
# 	echo $(OBJ_FILES)

%.o:%.cpp
	echo $(CFLAGS)
	g++ $(CFLAGS) $(LIBS) $(STD) -c -o $@ $<

main:$(OBJ_FILES)
	g++ $(CFLAGS) $(LIBS) $(STD)  -o $@ $^

all:main
	# g++ main.cpp -o main $(CFLAGS) $(LIBS)

clean:
	rm -rf main *.o