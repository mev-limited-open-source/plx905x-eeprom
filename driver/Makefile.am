## $Id$

## Process this file with automake to produce Makefile.in

## this is so that Automake includes the C compiling definitions, and
## includes the source files in the distribution.
EXTRA_PROGRAMS = automake_dummy
automake_dummy_SOURCES = kcompat.h kcompat_pci.h plx905x.c Makefile.kernel
generated_sources =

## there is no *just* object file support in automake.  This is close enough
module_DATA = plx905x.o
export_objs = plx905x.o

# where the kernel sources are located
KERNEL_LOCATION=@kerneldir@

PLX905X_TOP_SRCDIR = @abs_top_srcdir@
PLX905X_TOP_BUILDDIR = @abs_top_builddir@
PLX905X_BUILDDIR = @abs_builddir@

# some magic for using linux kernel settings
# when compiling module(s)
PLX905X_EXTRA_CFLAGS = -DEXPORT_SYMTAB $(DEFS) -I$(PLX905X_BUILDDIR) \
	-I$(PLX905X_TOP_BUILDDIR) -I$(PLX905X_TOP_SRCDIR)/include
export PLX905X_EXTRA_CFLAGS KERNEL_LOCATION module_DATA export_objs

.PHONY: FORCE

$(automake_dummy_SOURCES): FORCE
	@test "$(srcdir)" == "." || test -e "$@" || ln -vs "$(srcdir)/$@" .

# Add FORCE in case the kernel has changed.
$(module_DATA): $(generated_sources) $(automake_dummy_SOURCES) FORCE
	mv Makefile Makefile.automake
	cp Makefile.kernel Makefile
	$(MAKE) -C $(KERNEL_LOCATION) SUBDIRS=$(PLX905X_BUILDDIR) M=$(PLX905X_BUILDDIR) modules
	mv Makefile.automake Makefile

install-moduleDATA: $(module_DATA)
	$(mkinstalldirs) $(DESTDIR)$(moduledir)
	@list='$(module_DATA:.o=.@kernelext@)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(moduledir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(moduledir)/$$f; \
	done

uninstall-moduleDATA:
	@list='$(module_DATA:.o=.@kernelext@)'; for p in $$list; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(moduledir)/$$f"; \
	  rm -f $(DESTDIR)$(moduledir)/$$f; \
	done

#if SANDBOXED
#else
#install-exec-local: mkdev
#uninstall-local: rmdev
#endif
#
#mkdev:
#	test -c $(DESTDIR)$(devdir)/lirc || ($(mkinstalldirs) $(DESTDIR)$(devdir) && @mknod@ $(DESTDIR)$(devdir)/lirc c @lirc_major@ 0)
#
#rmdev:
#	-test -c $(DESTDIR)$(devdir)/lirc && $(RM) $(DESTDIR)$(devdir)/lirc

if SANDBOXED
else
install-data-local: install-moduleDATA
	-@depmod@ -a
endif

MOSTLYCLEANFILES = $(module_DATA) $(module_DATA:.o=.@kernelext@) \
	Modules.symvers Module.symvers Module.markers modules.order
CLEANFILES = $(module_DATA:.o=.mod.c) $(generated_sources)

clean-local:
	-rm -f .*.cmd .*.flags *.mod
	-rm -rf .tmp_versions
	-if test "$(srcdir)" != "."; then \
		for f in $(automake_dummy_SOURCES); do \
			if test "$$f" -ef "$(srcdir)/$$f"; then \
				rm -f "$$f"; \
			fi; \
		done; \
	fi

FORCE:
