SRCDIR := src
BUILDDIR := .
COMPILE := g++
TARGET := liblbRequester.so

# List of all .cpp source files.
#CPP = $(SRCDIR)/$(wildcard *.cpp)
CPP = $(wildcard $(SRCDIR)/*.cpp)

# All .o files go to build dir.
OBJ = $(CPP:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

all: $(OBJ)
	$(COMPILE) -shared -lcurl -o $(TARGET) $^

# Include all .d files
-include $(DEP)

$(BUILDDIR)/%.o : $(SRCDIR)/%.cpp
	mkdir -p $(@D)
	$(COMPILE) -c -MMD -fPIC -Iinc -o $@ $<

clean:
	rm -f $(DEP) $(OBJ) $(TARGET)
