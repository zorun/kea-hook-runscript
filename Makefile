
OBJECTS = src/load.o src/runscript.o src/callouts.o src/version.o
CXXFLAGS = -I /usr/include/kea -fPIC -Wno-deprecated
LDFLAGS = -L /usr/lib/kea/lib -shared -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions

kea-hook-runscript.so: $(OBJECTS)
	g++ -o $@ $(CXXFLAGS) $(LDFLAGS) $(OBJECTS)

%.o: %.cc
	g++ -c $(CXXFLAGS) -o $@ $<

src/callouts.o: src/runscript.h
src/load.o: src/common.h
src/runscript.o: src/common.h

clean:
	rm -f src/*.o
	rm -f kea-auth-radius.so
