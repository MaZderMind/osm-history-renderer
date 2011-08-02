#------------------------------------------------------------------------------
#
#  osm history importer makefile
#
#------------------------------------------------------------------------------

CXX = g++
#CXX = clang

CXXFLAGS = -g -O3 -Wall -Wextra -pedantic
CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
#CXXFLAGS += -Wredundant-decls -Wdisabled-optimization
#CXXFLAGS += -Wpadded -Winline

# path to osmium
CXXFLAGS += -I../../osmium/include

# compile & link against expat to have xml reading support
LDFLAGS += -lexpat

# compile & link against postgres to have database access
LDFLAGS += -lpq

# compile & link against proj4 for converting wgs84 to sperical mercator
LDFLAGS += -lproj

# compile &  link against libs needed for protobuf reading and writing
LDFLAGS += -lz -lprotobuf-lite -losmpbf -lpthread

# compile &  link against geos for multipolygon extracts
CXXFLAGS += -DOSMIUM_WITH_GEOS
LDFLAGS += -lgeos

.PHONY: all clean install

all: osm-history-importer

osm-history-importer: importer.cpp handler.hpp last_entity_tracker.hpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

install:
	install -m 755 -g root -o root -d $(DESTDIR)/usr/bin
	install -m 755 -g root -o root osm-history-importer $(DESTDIR)/usr/bin/osm-history-importer

clean:
	rm -f *.o core osm-history-importer

check:
	cppcheck --enable=all *.cpp

# This will try to compile each include file on its own to detect missing
# #include directives. Note that if this reports [OK], it is not enough
# to be sure it will compile in production code. But if it reports [FAILED]
# we know we are missing something.
check-includes:
	echo "check includes report:" >check-includes-report; \
	for FILE in *.hpp *.cpp; do \
		echo "$${FILE}:" >>check-includes-report; \
		echo -n "$${FILE} "; \
		if `g++ -I include $${FILE} 2>>check-includes-report`; then \
			echo "[OK]"; \
		else \
			echo "[FAILED]"; \
		fi; \
		rm -f $${FILE}.gch; \
	done

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --suffix=none --recursive include/\*.hpp examples/\*.cpp examples/\*.hpp osmjs/\*.cpp test/\*.cpp

#deb:
#	debuild -I -us -uc

#deb-clean:
#	debuild clean
