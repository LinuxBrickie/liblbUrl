COMPILE := g++
CXXFLAGS := -MMD -fPIC -Iinc

SRCDIR := src
BUILDDIR := .
TARGET := liblbUrl.so

GTESTDIR := gtest
GTESTBUILDDIR := .
GTESTTARGET := requesterTests

# List of all .cpp source files.
CPP = $(wildcard $(SRCDIR)/*.cpp)
GTESTCPP = $(wildcard $(GTESTDIR)/*.cpp)

# All .o files go to build dir.
OBJ = $(CPP:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
GTESTOBJ = $(GTESTCPP:$(GTESTDIR)/%.cpp=$(GTESTBUILDDIR)/%.o)

# gcc will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)
GTESTDEP = $(GTESTOBJ:%.o=%.d)

all: src gtest

src: $(OBJ)
	$(COMPILE) -shared -lcurl -o $(TARGET) $^

gtest: $(GTESTOBJ)
	$(COMPILE) -L$(BUILDDIR) -llbUrl -lgtest_main -lgtest -o $(GTESTTARGET) $^

# Include all .d files
-include $(DEP)
-include $(GTESTDEP)

$(BUILDDIR)/%.o : $(SRCDIR)/%.cpp
	mkdir -p $(@D)
	$(COMPILE) -c $(CXXFLAGS) -o $@ $<

$(GTESTBUILDDIR)/%.o : $(GTESTDIR)/%.cpp
	mkdir -p $(@D)
	$(COMPILE) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(DEP) $(OBJ) $(TARGET)
	rm -f $(GTESTDEP) $(GTESTOBJ) $(GTESTTARGET)
