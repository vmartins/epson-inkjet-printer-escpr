
%define pkg     epson-inkjet-printer-escpr
%define ver     1.7.7
%define rel     1

%define cupsfilterdir   /opt/epson-inkjet-printer-escpr/lib/cups/filter
%define cupsppddir      /opt/epson-inkjet-printer-escpr/share/cups/model

Name: %{pkg}
Version: %{ver}
Release: %{rel}
Source0: %{name}-%{version}.tar.gz
License: GPL
Vendor: Seiko Epson Corporation
URL: http://download.ebz.epson.net/dsc/search/01/search/?OSC=LX
Packager: Seiko Epson Corporation <linux-printer@epson.jp>
BuildRoot: %{_tmppath}/%{name}-%{version}
Group: Applications/System
Summary: Epson Inkjet Printer Driver (ESC/P-R) for Linux

%description
This software is a filter program used with Common UNIX Printing
System (CUPS) from the Linux. This can supply the high quality print
with Seiko Epson Color Ink Jet Printers.

This product supports only EPSON ESC/P-R printers. This package can be
used for all EPSON ESC/P-R printers.

For detail list of supported printer, please refer to below site:
http://download.ebz.epson.net/dsc/search/01/search/?OSC=LX

%prep
%setup -q

%build
%configure \
	--host=%{_target} \
	--with-cupsfilterdir=%{cupsfilterdir} \
	--with-cupsppddir=%{cupsppddir}
make

%install
rm -rf ${RPM_BUILD_ROOT}
make install-strip DESTDIR=${RPM_BUILD_ROOT}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
make clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc AUTHORS COPYING NEWS README README.ja
%{cupsfilterdir}/epson-escpr
%{cupsfilterdir}/epson-escpr-wrapper
%{_libdir}/libescpr.*
%{cupsppddir}/epson-inkjet-printer-escpr/*.ppd
