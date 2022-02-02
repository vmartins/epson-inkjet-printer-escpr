## Makefile.am -- an automake template for a Makefile.in file
## EPSON ESC/P-R Printer Driver for Linux
## Copyright (C) Seiko Epson Corporation 2000-2015.
##  This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.

SUBDIRS = \
	lib \
	ppd \
	@INTERNAL_PPD@ \
	src
DIST_SUBDIRS = $(SUBDIRS)

ACLOCAL_AMFLAGS = -I m4

noinst_DATA = \
	debian/control \
	debian/rules

debian/control: $(srcdir)/Makefile $(srcdir)/debian/control.in
	rm -f $@ $@.tmp; \
	sed -e 's|[@]PACKAGE_BUGREPORT[@]|$(PACKAGE_BUGREPORT)|' \
	    -e 's|[@]PACKAGE_NAME[@]|$(PACKAGE_NAME)|' \
	    -e 's|[@]PACKAGE_TARNAME[@]|$(PACKAGE_TARNAME)|' \
	    '$(top_srcdir)/$@.in' > $@.tmp; \
	mv $@.tmp $@

debian/rules: $(srcdir)/Makefile $(srcdir)/debian/rules.in
	rm -f $@ $@.tmp; \
	sed -e 's|[@]CUPS_FILTER_DIR[@]|$(CUPS_FILTER_DIR)|' \
	    -e 's|[@]CUPS_PPD_DIR[@]|$(CUPS_PPD_DIR)|' \
	    -e 's|[@]PACKAGE_TARNAME[@]|$(PACKAGE_TARNAME)|' \
	    -e 's|[@]INTERNAL_PPD_OPTION[@]|$(INTERNAL_PPD_OPTION)|' \
	    '$(top_srcdir)/$@.in' > $@.tmp; \
	mv $@.tmp $@; \
	chmod +x $@

#  Automate maintenance of the list of shipped M4 macros.
#  Do not change the list unnecessarily.  As a side effect, this also
#  works around issues with a read-only $(srcdir) during a 'distcheck'.

$(top_builddir)/m4-macros: $(top_srcdir)/m4
	echo M4_MACROS = `ls $^ | sed 's,^,m4/,'` > m4-macros.tmp
	cmp -s m4-macros.tmp $@ || cp m4-macros.tmp $@
	rm m4-macros.tmp

include m4-macros


EXTRA_DIST = \
	$(M4_MACROS) \
	$(noinst_DATA) \
	AUTHORS \
	COPYING \
	NEWS \
	README \
	README.ja \
	bootstrap \
	debian/changelog \
	debian/compat \
	debian/control.in \
	debian/copyright \
	debian/docs \
	debian/postinst \
	debian/postrm \
	debian/rules.in \
	epson-inkjet-printer-escpr.spec \
	epson-inkjet-printer-escpr.spec.in \
	lsb/lsb-rpm.spec \
	lsb/lsb-rpm.spec.in \
	m4
