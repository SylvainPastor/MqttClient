TARGETS := wp85 wp750x wp76xx localhost

export MANGOH_ROOT=$(LEGATO_ROOT)/../mangOH

.PHONY: all $(TARGETS)
all: $(TARGETS)

_build_sqlite.tar.gz:
	wget -O $@ www.sqlite.org/2017/sqlite-autoconf-3210000.tar.gz

sqlite: _build_sqlite.tar.gz
	rm -rf _build_sqlite
	mkdir _build_sqlite
	tar zxvf _build_sqlite.tar.gz -C _build_sqlite --strip=1
	(cd _build_sqlite; \
		./configure $(CONFIGURE_FLAGS) && \
		make)
	mkdir -p 3rdParty/{lib,include,bin}
	cp -d _build_sqlite/.libs/libsqlite3.so* 3rdParty/lib/
	cp _build_sqlite/sqlite3.h 3rdParty/include/
	cp _build_sqlite/sqlite3 3rdParty/bin/

uuid:
	rm -rf _build_uuid
	mkdir _build_uuid
	tar zxvf 3rdParty/uuid-1.6.2.tar.gz -C _build_uuid --strip=1
	(cd _build_uuid; \
		./configure $(CONFIGURE_FLAGS) --without-dce --without-cxx --without-perl --without-perl-compat --without-php --without-pgsql && \
		make)
	mkdir -p 3rdParty/{lib,include,bin}
	cp -d _build_uuid/.libs/libuuid.so* 3rdParty/lib/
	cp _build_uuid/uuid.h 3rdParty/include/
	cp _build_uuid/uuid 3rdParty/bin/

3rdParty: sqlite uuid

$(TARGETS):
	export TARGET=$@ ; \
	mkapp -v -t $@ \
          --interface-search=$(LEGATO_ROOT)/interfaces/modemServices \
          mqttClient.adef

install:
	app install mqttClient.wp85.update 192.168.2.2
	
clean:
	rm -rf _build_* *.*.update
