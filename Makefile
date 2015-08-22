# sub directories
SUBDIRS = src src/core src/render src/scene src/scripting
# sources
SRCS = $(foreach dir, $(SUBDIRS), $(wildcard $(dir)/*.cxx) )
# executable name
REL_EXE = Release/cengine
DBG_EXE = Debug/cengine
# objects
REL_OBJS = $(SRCS:%.cxx=Release/%.o)
DBG_OBJS = $(SRCS:%.cxx=Debug/%.o)
# dependencies
REL_DEPS = $(REL_OBJS:%.o=%.d)
DBG_DEPS = $(DBG_OBJS:%.o=%.d)
# output directory
ifeq ($(MAKECMDGOALS),debug)
OUTDIR = Debug
EXE = $(DBG_EXE)
OBJS = $(DBG_OBJS)
DEPS = $(DBG_DEPS)
else
OUTDIR = Release
EXE = $(REL_EXE)
OBJS = $(REL_OBJS)
DEPS = $(REL_DEPS)
endif

# standard
CXXFLAGS += -std=c++11
# optimisation
ifeq ($(MAKECMDGOALS),debug)
CXXFLAGS += -DDEBUG -O0 -g3
else
CXXFLAGS += -O3 -fno-omit-frame-pointer
endif
# architecture
CXXFLAGS += -march=native
# threading
CXXFLAGS += -pthread
# warnings
CXXFLAGS += -Wall -Wextra -Wconversion
# defines
CXXFLAGS += -DOVERRIDE= -DPROTECTED= -DPRIVATE= -DPUBLIC= -DSTATIC= -DVIRTUAL= -DEXPLICIT=
# dependency generation
CXXFLAGS += -MMD -MP

# external libs
LIBS += -lX11 -ltcmalloc -lIL -lILU -lGL -lGLEW -lpthread

# all target
all: $(EXE)

# debug target
debug: $(EXE)

# sources -> objects
$(OUTDIR)/%.o: %.cxx Makefile
	@mkdir -p $(@D)
	@echo 'Building: $<'
	@$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

# objects -> exe
$(EXE): $(OBJS)
	@echo 'Linking...'
	@$(CXX) -o "$@" $(OBJS) $(LIBS)
	@echo 'Finished: $@'

# clean target
clean:
	@rm -f $(DBG_OBJS) $(DBG_DEPS) $(DBG_EXE) $(REL_OBJS) $(REL_DEPS) $(REL_EXE)

# phonies
.PHONY: clean all

# include dependencies
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
