CC = g++
CXXFLAGS = -std=c++20 -I. -Isrc -Wall -pedantic -g
LIBRARIES = -lbcm2835
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
SRCS = $(SRC_DIR)/EPD_Driver.cpp main.cpp
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.cpp=.o)))
N_APP = epd_driver_small

.DEFAULT_GOAL := $(N_APP)

.PHONY: all clean run

all: $(N_APP)

$(N_APP): $(OBJS) | $(BIN_DIR)
	$(CC) $(CXXFLAGS) -o $(BIN_DIR)/$@ $^ $(LIBRARIES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@ -MMD -MP

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@ -MMD -MP

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: $(N_APP)
	sudo $(BIN_DIR)/$(N_APP)

-include $(OBJS:.o=.d)
