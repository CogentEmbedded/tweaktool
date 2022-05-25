ifeq ($(TARGET_CPU),A72)

include $(PRELUDE)

TARGET = nng
TARGETTYPE = prebuilt
PREBUILT = lib/J7/$(TARGET_CPU)/$(TARGET_OS)/libnng.a
PREFIX = lib

include $(FINALE)

# Workaround for libnng.a -> nng.a loosing its prefix
#
# Concerto does not handle Linux libraries properly. The following decalaration:
#
# $(_MODULE)_BIN := $($(_MODULE)_TDIR)/$(TARGET)$(suffix $(PREBUILT))
#
# in finale.mak assumes that the target filename is $(TARGET)$(SUFFIX) which is
# not true for Linux.

# I cannot use $(_MODULE) here so I replace it with $(TARGET)

$(TARGET)_BIN := $(TDIR)/$(PREFIX)$(TARGET)$(suffix $(PREBUILT))
build:: $($(TARGET)_BIN)

#In Windows, copy does not update the timestamp, so we have to do an extra step below to update the timestamp
define $(TARGET)_PREBUILT
$($(TARGET)_BIN): $(SDIR)/$(1) $(TDIR)/.gitignore
	@echo Copying Prebuilt binary $(SDIR)/$(1) to $($(TARGET)_BIN)
	-$(Q)$(COPY) $(call PATH_CONV,$(SDIR)/$(1) $($(TARGET)_BIN))
ifeq ($(HOST_OS),Windows_NT)
	-$(Q)cd $(TDIR) && $(COPY) $(call PATH_CONV,$($(TARGET)_BIN))+,,
endif

$(TARGET)_CLEAN_LNK =
endef

$(eval $(call $(TARGET)_PREBUILT,$(PREBUILT)))

endif
