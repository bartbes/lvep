ifeq ($(wildcard config.mk),)
$(error Please copy config.mk.template to config.mk and set LOVE_PATH)
else
include config.mk
endif

CXXFLAGS:=-std=c++11 -g -fPIC

pkgconfig=$(eval CPPFLAGS+=$(shell pkg-config --cflags $(1)))\
		  $(eval LDLIBS+=$(shell pkg-config --libs $(1)))

CPPFLAGS+=-I$(LOVE_PATH)/src/ -I$(LOVE_PATH)/src/modules/
LDLIBS+=-llove

$(call pkgconfig,libavformat)
$(call pkgconfig,libavcodec)
$(call pkgconfig,libavutil)
$(call pkgconfig,luajit)

SOURCES=$(wildcard src/*.cpp)

.PHONY: all clean .vimrc

all: liblvep.so

clean:
	$(RM) liblvep.so src/*.o src/*.d

liblvep.so: $(SOURCES:.cpp=.o)
	$(CXX) -shared -o $@ $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS)

%.d: %.cpp
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM -MP -MQ $*.o -MF $@ $<

.vimrc:
	$(RM) $@
	echo "let g:syntastic_cpp_compiler='$(CXX)'" >> $@
	echo "let g:syntastic_cpp_compiler_options='$(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS)'" >> $@

-include $(SOURCES:.cpp=.d)
