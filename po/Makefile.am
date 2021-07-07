#
# Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AUTOMAKE_OPTIONS = -Wno-portability

POFILES = AUTO

# Same list as Tasking manager
 translations = bn de fa_IR gl it lt nl_NL pt sl vi cs en fi hu ja mg pl ru ta  zh_TW

# translations = de fr

# This is GNU make specific, so if you are building on a BSD
# machine, you probably want to install GNU make in additional to
# BSD make.
pofiles := $(patsubst %,%.po,$(translations))
gmofiles := $(patsubst %,%.gmo,$(translations))

CLEANFILES = \
	$(gmofiles) 

dirs = \
	validate \
	osmstats \
	data

EXTRA_DIST = $(underpass.pot)

all: $(gmofiles)

if HAS_GETTEXT

SRCS = $(shell find $(patsubst %,../%,$(dirs)) -name '*.cc')
underpass.pot: $(SRCS)
	cd $(top_srcdir); \
	pwd; \
	rm -f po/POTFILES; \
	for i in $(SRCS); do \
	  echo "$$i" |sed 's%../%%' >> $@;\
	done; \
	$(XGETTEXT) --language=C++ --files-from=po/POTFILES \
	  -o po/underpass.pot \
	  --default-domain=underpass --force \
	  --flag=_:1:pass-c-format --keyword=_ --keyword=_q \
	  --copyright-holder='Humanitarian OpenStreetMap Team';  \
	rm -f po/POTFILES

update-po: 
	$(MAKE) underpass.pot
	for i in $(pofiles); do \
	  echo "Merging $$i..." ; \
	  $(MSGMERGE) --update $(srcdir)/$$i $(srcdir)/underpass.pot; \
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

# install-hook: rename the .gmo files to underpass.mo at installation time
install-data-hook: $(gmofiles)
	@for i in $(translations); do \
	  $(mkinstalldirs) "$(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/"; \
	  if test -f $$i.gmo; then \
	    $(INSTALL_DATA) $$i.gmo $(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/underpass.mo; \
	  fi; \
	done

uninstall-hook:
	for i in $(translations); do \
	  rm -f "$(DESTDIR)$(datadir)/locale/$$i/LC_MESSAGES/underpass.mo"; \
	done
