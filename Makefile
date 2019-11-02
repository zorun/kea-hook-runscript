
KEA_MSG_COMPILER ?= kea-msg-compiler
KEA_INCLUDE ?= /usr/include/kea
KEA_LIB ?= /usr/lib

OBJECTS = src/messages.o src/logger.o src/load.o src/runscript.o src/callouts.o src/version.o
DEPS = $(OBJECTS:.o=.d)
CXXFLAGS = -I $(KEA_INCLUDE) -fPIC -Wno-deprecated -std=c++11
LDFLAGS = -L $(KEA_LIB) -shared -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions

kea-hook-runscript.so: $(OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(OBJECTS)

%.o: %.cc
	$(CXX) -MMD -MP -c $(CXXFLAGS) -o $@ $<

# Compile messages (for logging)
src/messages.h src/messages.cc: s-messages
s-messages: src/messages.mes
	$(KEA_MSG_COMPILER) -d src/ $<
	touch $@

clean:
	rm -f src/*.o
	rm -f src/messages.h src/messages.cc s-messages
	rm -f kea-hook-runscript.so

-include $(DEPS)
