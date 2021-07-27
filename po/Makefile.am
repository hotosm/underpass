#
# Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
#
# This file is part of Underpass.
#
#     Underpass is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     Underpass is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = -Wno-portability

POFILES = AUTO

translations = cs da de en_GB es fi fr it ja el km nb ne ru zh_CN zh_TW


# This is GNU make specific, so if you are building Gnash on a BSD
# machine, you probably want to install GNU make in additional to
# BSD make.
pofiles := $(patsubst %,%.po,$(translations))
gmofiles := $(patsubst %,%.gmo,$(translations))

CLEANFILES = \
	$(gmofiles) 

dirs =  osmstats data validate

EXTRA_DIST = $(pofiles) underpass.pot 

all: $(gmofiles)

if HAS_GETTEXT

SRCS = $(shell find $(patsubst %,../%,$(dirs)) -name '*.cpp')
gnash.pot: $(SRCS)
	cd $(top_srcdir); \
	pwd; \
	rm -f po/POTFILES; \
	for i in $(SRCS); do \
	  echo "$$i" |sed 's%../%%' >> $@;\
	done; \
	$(XGETTEXT) --language=C++ --files-from=po/POTFILES \
	  -o po/gnash.pot \
	  --default-domain=gnash --force \
	  --flag=_:1:pass-c-format --keyword=_ --keyword=_q \
	  --copyright-holder='Free Software Foundation, Inc.';  \
	rm -f po/POTFILES

update-po: 
	$(MAKE) gnash.pot
	for i in $(pofiles); do \
	  echo "Merging $$i..." ; \
	  $(MSGMERGE) --update $(srcdir)/$$i $(srcdir)/gnash.pot; \
	done

SUFFIXES = .po .gmo
.po.gmo:
	$(GMSGFMT) --statistics --directory=$(srcdir) -o $*.gmo $(@:%.gmo=%.po) 

else # not HAS_GETTEXT

$(gmofiles): force
	@echo "WARNING: can't compile $@ translations w/out gettext"

update-po: force
	@echo "WARNING: can't update translation files (.po) w/out gettext"

endif # not HAS_GETTEXT

force:

# install-hook: rename the .gmo files to gnash.mo at installation time
install-data-hook: $(gmofiles)
	@for i in $(translations); do \
	  $(mkinstalldirs) "$(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/"; \
	  if test -f $$i.gmo; then \
	    $(INSTALL_DATA) $$i.gmo $(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/gnash.mo; \
	  fi; \
	done

uninstall-hook:
	for i in $(translations); do \
	  rm -f "$(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/gnash.mo"; \
	done
