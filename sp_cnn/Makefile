# CNN CPU makefile
CC := g++
BUILDDIR := build

PROJECT_ROOT_DIR = ..

CNN_DIR := $(PROJECT_ROOT_DIR)/cnn
CNN_SOURCES := $(wildcard $(CNN_DIR)/*.cpp)
CNN_OBJECTS = $(patsubst $(CNN_DIR)/%, $(BUILDDIR)/%, $(CNN_SOURCES:.cpp=.o))

UTILITIES_DIR := $(PROJECT_ROOT_DIR)/utilities
UTILITIES_SOURCES := $(wildcard $(UTILITIES_DIR)/BWImage.cpp)
UTILITIES_OBJECTS = $(patsubst $(UTILITIES_DIR)/%, $(BUILDDIR)/%, $(UTILITIES_SOURCES:.cpp=.o))

SP_CNN_SIM_DIR := .
SP_CNN_SIM_SOURCES := $(wildcard $(SP_CNN_SIM_DIR)/*.cpp)
SP_CNN_SIM_OBJECTS = $(patsubst $(SP_CNN_SIM_DIR)/%, $(BUILDDIR)/%, $(SP_CNN_SIM_SOURCES:.cpp=.o))

TEST_DIR := unit-tests
TEST_SOURCES := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%, $(BUILDDIR)/%, $(TEST_SOURCES:.cpp=.o))

NAIVE_DIR := naive-sp_cnn
NAIVE_SOURCES := $(wildcard $(NAIVE_DIR)/*.cpp)
NAIVE_OBJECTS = $(patsubst $(NAIVE_DIR)/%, $(BUILDDIR)/%, $(NAIVE_SOURCES:.cpp=.o))

SP_CNN_RUN_DIR := sp_cnn
SP_CNN_RUN_SOURCES := $(wildcard $(SP_CNN_RUN_DIR)/*.cpp)
SP_CNN_RUN_OBJECTS = $(patsubst $(SP_CNN_RUN_DIR)/%, $(BUILDDIR)/%, $(SP_CNN_RUN_SOURCES:.cpp=.o))

SOURCES := $(CNN_SOURCES) $(UTILITIES_SOURCES) $(SP_CNN_SIM_SOURCES) $(TEST_SOURCES)
OBJECTS := $(CNN_OBJECTS) $(UTILITIES_OBJECTS) $(SP_CNN_SIM_OBJECTS) $(TEST_OBJECTS)

DEPS := $(OBJECTS:.o=.deps)

TARGETS := $(TEST_DIR)/test-sp-cnn $(NAIVE_DIR)/naive-sp-cnn $(SP_CNN_RUN_DIR)/sp-cnn

override CXXFLAGS += $(patsubst %, -I%, $(PROJECT_ROOT_DIR) .) 
override CXXFLAGS += -O3 -march=corei7 -std=gnu++0x
override LDFLAGS += -static-libgcc

define cc-command
@mkdir -p $(BUILDDIR)
@echo "Compile $<"; 
$(CXX) $(CXXFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<
endef

.PHONY: clean all

all: $(TARGETS)


$(TEST_DIR)/test-sp-cnn: $(TEST_OBJECTS) $(SP_CNN_SIM_OBJECTS) $(UTILITIES_OBJECTS) $(CNN_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^
        
$(NAIVE_DIR)/naive-sp-cnn: $(NAIVE_OBJECTS) $(SP_CNN_SIM_OBJECTS) $(UTILITIES_OBJECTS) $(CNN_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(SP_CNN_RUN_DIR)/sp-cnn: $(SP_CNN_RUN_OBJECTS) $(SP_CNN_SIM_OBJECTS) $(UTILITIES_OBJECTS) $(CNN_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^
	
$(CNN_OBJECTS) : $(BUILDDIR)/%.o : $(CNN_DIR)/%.cpp
	$(cc-command)

$(UTILITIES_OBJECTS) : $(BUILDDIR)/%.o : $(UTILITIES_DIR)/%.cpp
	$(cc-command)

$(SP_CNN_SIM_OBJECTS) : $(BUILDDIR)/%.o : $(SP_CNN_SIM_DIR)/%.cpp
	$(cc-command)

$(TEST_OBJECTS) : $(BUILDDIR)/%.o : $(TEST_DIR)/%.cpp
	$(cc-command)

$(NAIVE_OBJECTS) : $(BUILDDIR)/%.o : $(NAIVE_DIR)/%.cpp
	$(cc-command)
	
$(SP_CNN_RUN_OBJECTS) : $(BUILDDIR)/%.o : $(SP_CNN_RUN_DIR)/%.cpp
	$(cc-command)

clean:
	@echo " Cleaning..."; $(RM) -r -f $(BUILDDIR) $(TARGETS)
                       
-include $(DEPS)
                        
