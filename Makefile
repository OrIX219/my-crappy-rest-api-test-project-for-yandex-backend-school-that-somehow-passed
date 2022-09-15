SRC_DIR = src
OBJ_DIR = .o
DEP_DIR = .d
BIN_DIR = bin

TARGET = $(BIN_DIR)/Main
INCLUDE = -Iinclude -Iinclude/asio
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP = $(SRC:$(SRC_DIR)/%.cpp=$(DEP_DIR)/%.d)
CXXFLAGS = -std=c++17 -O2
LDFLAGS = -lpqxx -lpq
DEPFLAGS = -MMD -MF $(DEP_DIR)/$*.Td

COMPILE.cpp = $(CXX) $(CXXFLAGS) -c $< $(DEPFLAGS) -o $@
LINK.o = $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@
POSTCOMPILE = mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d 

.PHONY: all clean

all: $(TARGET)

clean:
		rm -rf $(BIN_DIR) $(OBJ_DIR) $(DEP_DIR)

-include $(DEP)

$(TARGET): $(OBJ) | $(BIN_DIR)
		$(LINK.o) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp	| $(OBJ_DIR)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d | $(OBJ_DIR) $(DEP_DIR)
		$(COMPILE.cpp) $(INCLUDE)
		$(POSTCOMPILE)

$(BIN_DIR) $(OBJ_DIR) $(DEP_DIR):
		mkdir -p $@

.PRECIOUS: $(DEP_DIR)/%.d
$(DEP_DIR)/%.d: ; 
