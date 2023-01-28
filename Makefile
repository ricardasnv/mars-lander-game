# Ricardas Navickas 2020
CXX=g++

SRCDIRS=src src/core src/imgui
OBJDIR=obj
SRCSUFFIX=cpp
DEPSUFFIX=h
OBJSUFFIX=o
TARGET=lander

CXXFLAGS=-O3 -Wall -g
LFLAGS=-lGL -lGLEW -lglfw -llua

SRCS=$(wildcard $(addsuffix /*.cpp,$(SRCDIRS)))
OBJS=$(addprefix $(OBJDIR)/,$(subst .$(SRCSUFFIX),.$(OBJSUFFIX),$(notdir $(SRCS))))

# ==== ==== ==== ==== ==== ==== ==== ====

all: $(OBJDIR)/ build

$(OBJDIR)/:
	mkdir -p $(OBJDIR)/

build: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LFLAGS)

clean:
	@echo Cleaning up...;
	rm -f $(OBJDIR)/*.$(OBJSUFFIX)
	rm -f $(TARGET)

$(OBJDIR)/%.$(OBJSUFFIX): */%.$(SRCSUFFIX)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.$(OBJSUFFIX): */*/%.$(SRCSUFFIX)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

