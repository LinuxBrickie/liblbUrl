COMPILE := g++
CXXFLAGS := -MMD -fPIC -Iinc

SRCDIR := src
BUILDDIR := .
TARGET := liblbUrl.so

TOOLSDIR := tools
TOOLSBUILDDIR := .
TOOLSTARGET := versionInfo

GTESTDIR := gtest
GTESTBUILDDIR := .
GTESTTARGET := requesterTests

# Primary dependencies

LBENCODINGPATH := ../liblbEncoding
LBENCODINGINC := -I $(LBENCODINGPATH)/inc
LBENCODINGLD := -L$(LBENCODINGPATH) -llbEncoding

#CURLPATH := ../../../C/curl
#CURLINC := -I $(CURLPATH)/include
#CURLLD := -L$(CURLPATH)/lib/.libs -lcurl
CURLLD := -lcurl

LBHTTPDPATH := ../liblbHttpd
LBHTTPDINC := -I $(LBHTTPDPATH)/inc
LBHTTPDLD := -L$(LBHTTPDPATH) -llbHttpd

# Secondary dependencies - use -Wl,-rpath-link only
#LBENCODINGPATH := ../liblbEncoding

# List of all .cpp source files.
CPP = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/http/*.cpp) $(wildcard $(SRCDIR)/ws/*.cpp)
TOOLSCPP = $(wildcard $(TOOLSDIR)/*.cpp)
GTESTCPP = $(wildcard $(GTESTDIR)/*.cpp) $(wildcard $(GTESTDIR)/httpd/*.cpp)

# All .o files go to build dir.
OBJ = $(CPP:%.cpp=$(BUILDDIR)/%.o)
TOOLSOBJ = $(TOOLSCPP:%.cpp=$(TOOLSBUILDDIR)/%.o)
GTESTOBJ = $(GTESTCPP:%.cpp=$(GTESTBUILDDIR)/%.o)

# gcc will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)
TOOLSDEP = $(TOOLSOBJ:%.o=%.d)
GTESTDEP = $(GTESTOBJ:%.o=%.d)

debug: DEBUG = -g -DDEBUG
debug: all

all: $(TARGET) $(GTESTTARGET)

$(TARGET): $(OBJ)
	$(COMPILE) -shared $(LBENCODINGLD) $(CURLLD) -o $(TARGET) $(OBJ)

$(TOOLSTARGET): $(TOOLSOBJ)
	$(COMPILE) $(CURLLD) -o $(TOOLSTARGET) $(TOOLSOBJ)

$(GTESTTARGET): $(GTESTOBJ) $(TARGET)
	$(COMPILE) -Wl,-rpath,$(BUILDDIR) $(LBENCODINGLD) -L$(BUILDDIR) $(LBHTTPDLD) -llbUrl -lgtest -lmicrohttpd -o $(GTESTTARGET)  $(GTESTOBJ)

# Include all .d files
-include $(DEP)
-include $(TOOLSDEP)
-include $(GTESTDEP)

$(BUILDDIR)/$(SRCDIR)/%.o : $(SRCDIR)/%.cpp
	mkdir -p $(@D)
	$(COMPILE) $(DEBUG) $(LBENCODINGINC) -c $(CXXFLAGS) $(CURLINC) -o $@ $<

$(TOOLSBUILDDIR)/$(TOOLSDIR)/%.o : $(TOOLSDIR)/%.cpp
	mkdir -p $(@D)
	$(COMPILE) $(DEBUG) -c $(CXXFLAGS) $(CURLINC) -o $@ $<

$(GTESTBUILDDIR)/$(GTESTDIR)/%.o : $(GTESTDIR)/%.cpp
	mkdir -p $(@D)
	$(COMPILE) $(DEBUG) $(LBENCODINGINC) -c $(CXXFLAGS) $(CURLINC) $(LBHTTPDINC) -o $@ $<

clean:
	rm -f $(DEP) $(OBJ) $(TARGET)
	rm -f $(TOOLSDEP) $(TOOLSOBJ) $(TOOLSTARGET)
	rm -f $(GTESTDEP) $(GTESTOBJ) $(GTESTTARGET)
