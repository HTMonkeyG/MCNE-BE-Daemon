DIST_DIR = ./dist
SRC_DIR = ./src
 
SRC = $(wildcard $(SRC_DIR)/*.c) ./main.c
OBJ = $(patsubst %.c, $(DIST_DIR)/%.o, $(notdir $(SRC))) dist/icon.o

TARGET = main.exe
BIN_TARGET = $(DIST_DIR)/$(TARGET)

CC = gcc

$(BIN_TARGET):$(OBJ)
	$(CC) $(OBJ) -o $@ -lpsapi

$(DIST_DIR)/main.o: ./main.c
	$(CC) -c main.c -o ./dist/main.o

$(DIST_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) -c $< -o $@

dist/icon.o: $(SRC_DIR)/icon/mcne.ico $(SRC_DIR)/icon/icon.rc
	windres -i $(SRC_DIR)/icon/icon.rc -o $(DIST_DIR)/icon.o

clean:
	del .\dist\*.o
	del .\dist\main.exe