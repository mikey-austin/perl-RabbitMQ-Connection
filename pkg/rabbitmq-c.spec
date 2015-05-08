Name:      rabbitmq-c
Summary:   RabbitMQ C client
Version:   0.6.0
Release:   1
License:   MIT
Group:     Applications/Internet
URL:       https://github.com/alanxz/rabbitmq-c
Source0:   %{name}-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root

Requires: openssl

BuildRequires: openssl-devel

%description
RabbitMQ C client.

%prep
%setup -q

%build
./configure --prefix=%{_prefix} \
            --sysconfdir=%{_sysconfdir} \
            --localstatedir=%{_var}/lib \
            --sbindir=%{_sbindir} \
            --libdir=%{_libdir} \
            --includedir=%{_includedir} \
            --with-ssl=openssl \
            --disable-examples \
            --disable-docs \
            --disable-tools
make

%install
rm -fr %{buildroot}
make install DESTDIR=%{buildroot}

%clean
#rm -fr %{buildroot}

%files
%defattr(-,root,root)
%{_libdir}/librabbit*
%{_libdir}/pkgconfig*
%{_includedir}/amqp*
