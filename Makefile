O ?= out
OPTEE_CLIENT_EXPORT_PATH ?= ../optee_client/out/export/usr

ifeq ($(wildcard $(OPTEE_CLIENT_EXPORT_PATH)/include/tee_client_api.h),)
$(error OPTEE_CLIENT_EXPORT_PATH is not set or invalid)
endif

ifneq ($V,1)
export q := @
export echo := @echo
else
export q :=
export echo := @:
endif
ifneq ($(filter 4.%,$(MAKE_VERSION)),)  # make-4
ifneq ($(filter %s ,$(firstword x$(MAKEFLAGS))),)
export echo := @:
endif
else                                    # make-3.8x
ifneq ($(findstring s, $(MAKEFLAGS)),)
export echo := @:
endif
endif

VERSION = $(shell git describe --always --dirty=-dev 2>/dev/null || echo Unknown)

CROSS_COMPILE ?= aarch64-linux-gnu-
CROSS_COMPILE_HOST ?= $(CROSS_COMPILE)
CC = $(CROSS_COMPILE_HOST)gcc

srcs := tee-stats.c

objs := $(patsubst %.c,$(O)/%.o, $(srcs))

CFLAGS += -DVERSION="$(VERSION)"
CFLAGS += -I. -I$(OPTEE_CLIENT_EXPORT_PATH)/include

LDFLAGS += -L$(OPTEE_CLIENT_EXPORT_PATH)/lib -lteec -lm

.PHONY: all
all: $(O)/tee-stats

$(O)/tee-stats: $(objs)
	$(echo) '  CC      $@'
	$(q)$(CC) -o $@ $+ $(LDFLAGS)

$(O)/%.o: $(CURDIR)/%.c
	$(q)mkdir -p $(O)
	$(echo) '  CC      $@'
	$(q)$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@echo '  CLEAN  $(O)'
	$(q)rm -f $(O)/tee-stats
	$(q)rm -f $(objs)
	$(q)if [ -e $(O) ]; then rmdir --ignore-fail-on-non-empty $(O); fi
	

.PHONY: install
install: all
	$(echo) '  INSTALL $(DESTDIR)/bin/tee-stats'
	$(q)mkdir -p $(DESTDIR)/bin
	$(q)cp -a $(O)/tee-stats $(DESTDIR)/bin

.PHONY: uninstall
uninstall:
	$(echo) '  UNINST  $(DESTDIR)/bin/tee-stats'
	$(q)rm -f $(DESTDIR)/bin/tee-stats

