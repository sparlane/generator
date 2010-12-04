CXXFLAGS:=-Iinclude -D_GNU_SOURCE -ggdb -g
LXXFLAGS:=`pkg-config --cflags --libs lua` -ggdb -g
CXXFILES:=main.cpp world.cpp module.cpp luainterface.cpp\
	$(addprefix generate/, element.cpp object.cpp function.cpp member.cpp rtype.cpp pointer.cpp objectMember.cpp array.cpp)

G++:=g++

OFILES:=$(CXXFILES:.cpp=.o)

%.o: %.cpp include/generator.h Makefile
	@echo "Generating: $(@) from $(<)"
	@$(G++) -c -o $(@) $(<) $(CXXFLAGS)

all: generator

generator: $(OFILES)
	@echo "$(G++) -o $(@) $(LXXFLAGS) $(OFILES)"
	@$(G++) -o $(@)   $(LXXFLAGS) $(OFILES)

clean:
	@echo "Removing un-needed files"
	@rm -f $(OFILES) generator

