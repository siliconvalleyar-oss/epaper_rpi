CC = g++
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g
LIBRARIES = -pthread -lbcm2835

SRC_DIR = src
LIB_DIR = libs
OBJ_DIR = obj
BIN_DIR = bin

SRCS     = $(wildcard $(SRC_DIR)/*.cpp)
LIB_DIRS = $(wildcard $(LIB_DIR)/*)
LIB_SRCS = $(foreach dir,$(LIB_DIRS),$(wildcard $(dir)/*.cpp))
OBJS     = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.cpp=.o) $(notdir $(LIB_SRCS:.cpp=.o))))
APP      = $(BIN_DIR)/dino_game

.PHONY: all clean run

all: $(APP)

$(APP): $(OBJS) | $(BIN_DIR)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBRARIES)
	@echo "Binary created: $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@ -MMD -MP
	@echo "Compiled: $< -> $@"

# Plantilla de compilación para subdirectorios de LIB_DIR
define compile_template
$(OBJ_DIR)/%.o: $(LIB_DIR)/$(1)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $$< -o $$@ -MMD -MP
	@echo "Compiled library: $$< -> $$@"
endef

$(foreach libdir,$(LIB_DIRS),$(eval $(call compile_template,$(notdir $(libdir)))))

$(BIN_DIR) $(OBJ_DIR):
	@test -d $@ || mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	rm -f log/*

run: $(APP)
	sudo $<

-include $(OBJS:.o=.d)
