Name:             metronome
Version:          @VERSION@
Release:          @RELEASE@
Summary:          A tiny graphite receiver with flat-file storage
Group:            System Environment/Daemons

License:          GPLv2 
URL:              https://github.com/ahupowerdns/metronome
Source0:          metronome-@VERSION@.tar.bz2

BuildRequires:    boost-devel
BuildRequires:    eigen3-devel
BuildRequires:    systemd-devel
Requires(pre):    shadow-utils
Requires(post):   systemd-units
Requires(preun):  systemd-units
Requires(postun): systemd-units

%description
Metronome is a small receiver for Carbon (graphite) data with a small http API to retreive this data.

%prep
%setup -q

%build
%configure \
  --disable-silent-rules \
  --enable-systemd --with-systemd=/lib/systemd/system

make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
%make_install
install -d -m 755 %{buildroot}/var/lib/metronome

%pre
getent group metronome > /dev/null || groupadd -r metronome
getent passwd metronome > /dev/null || \
    useradd -d /var/lib/metronome -r -g metronome -d / -s /sbin/nologin \
    -c "Metronome user" metronome
exit 0

%post
chown metronome:metronome /var/lib/metronome
%systemd_post metronome.service

%preun
%systemd_preun metronome.service

%postun
%systemd_postun metronome.service

%files
/usr/bin/*
/usr/share/metronome
/lib/systemd/system/metronome.service
%dir /var/lib/metronome
%doc %{_defaultdocdir}/%{name}/README.md
