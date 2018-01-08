TARGETS := wp85 wp750x wp76xx localhost

export MANGOH_ROOT=$(LEGATO_ROOT)/../mangOH

.PHONY: all $(TARGETS)
all: $(TARGETS)

_3rdParties:
	(cd 3rdParty; \
		mkdir -p $(TARGET); \
		tar zxvf $(TARGET).tar.gz -C $(TARGET) --strip=1)

$(TARGETS):
	export TARGET=$@ ; \
	make _3rdParties; \
	mkapp -v -t $@ \
          --interface-search=$(LEGATO_ROOT)/interfaces/modemServices \
          mqttClient.adef

install:
	app install mqttClient.wp85.update 192.168.2.2
	
clean:
	rm -rf _build_* *.*.update
