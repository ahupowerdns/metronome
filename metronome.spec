%if 0%{?rhel} < 6
exit 1
%endif

%define upstart_post() \
  if [ -x /sbin/initctl ]; then \
    /sbin/initctl start %1 \
  fi\
%{nil}

%define upstart_postun() \
  if [ -x /sbin/initctl ] && /sbin/initctl status %1 2>/dev/null | grep -q 'running' ; then \
    /sbin/initctl stop %1 >/dev/null 2>&1 \
    [ -f /etc/init/%1.conf ] && { echo -n "Re-"; /sbin/initctl start %1; }; \
  fi \
%{nil}

Name:             metronome
Version:          @VERSION@
Release:          @RELEASE@
Summary:          A tiny graphite receiver with flat-file storage
Group:            System Environment/Daemons

License:          GPLv2
URL:              https://github.com/ahupowerdns/metronome
Source0:          metronome-@VERSION@.tar.bz2

%if %{?rhel} >= 7
BuildRequires:    boost-devel
BuildRequires:    systemd-devel
Requires(post):   systemd-units
Requires(preun):  systemd-units
Requires(postun): systemd-units
%else
BuildRequires:    boost148-devel
BuildRequires:    devtoolset-4-gcc-c++
%endif

BuildRequires:    eigen3-devel
Requires(pre):    shadow-utils

%description
Metronome is a small receiver for Carbon (graphite) data with a small http API to retreive this data.

%prep
%if %{?rhel} < 7
source /opt/rh/devtoolset-4/enable
%endif
%setup -q

%build

%configure \
  --disable-silent-rules \
%if %{?rhel} >= 7
  --enable-systemd --with-systemd=/lib/systemd/system \
%else
  --disable-systemd \
  --with-boost=/usr/include/boost148 \
  LIBRARY_PATH=/usr/lib64/boost148
%endif

%if %{?rhel} >= 7
make %{?_smp_mflags}
%else
LIBRARY_PATH=/usr/lib64/boost148 make %{?_smp_mflags}
%endif

%install
rm -rf $RPM_BUILD_ROOT
%make_install
install -d -m 755 %{buildroot}/var/lib/%{name}
%if %{?rhel} < 7
install -d -m 755 %{buildroot}%{_sysconfdir}/init
install -m 644 %{name}-upstart.conf %{buildroot}%{_sysconfdir}/init/%{name}.conf
%endif

%pre
getent group %{name} > /dev/null || groupadd -r %{name}
getent passwd %{name} > /dev/null || \
    useradd -d /var/lib/%{name} -r -g %{name} -d / -s /sbin/nologin \
    -c "Metronome user" %{name}
exit 0

%post
chown %{name}:%{name} /var/lib/%{name}
%if %{?rhel} >= 7
%systemd_post %{name}.service
%else
%upstart_post %{name}
%endif

%preun
%if %{?rhel} >= 7
%systemd_preun %{name}.service
%endif

%postun
%if %{?rhel} >= 7
%systemd_postun %{name}.service
%else
%upstart_postun %{name}
%endif

%files
/usr/bin/*
/usr/share/%{name}
%if %{?rhel} >= 7
/lib/systemd/system/%{name}.service
%else
%{_sysconfdir}/init/%{name}.conf
%endif
%dir /var/lib/%{name}
%doc %{_defaultdocdir}/%{name}/README.md
