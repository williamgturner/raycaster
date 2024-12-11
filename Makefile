APP_NAME = raycaster
BUILD_DIR = ./bin
C_FILES = ./src/*.c

APP_INCLUDES:= -I./Library/Frameworks/SDL2.framework
LIBRARIES:= -L./Library/Frameworks/SDL2.framework/Headers -lSDL2
TEST:= -F/Library/Frameworks -framework SDL2

build:
	gcc $(C_FILES) -o $(BUILD_DIR)/$(APP_NAME) -rpath /Library/Frameworks $(APP_INCLUDES) $(TEST)

clean:
	rm -f ./bin/raycaster

run:
	./bin/raycaster

all:
	make clean
	make build
	make run