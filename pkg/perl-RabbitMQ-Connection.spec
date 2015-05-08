Name:           perl-RabbitMQ-Connection
Version:        0.1
Release:        1%{?dist}
Summary:        RabbitMQ::Connection Perl module
License:        CHECK(Distributable)
Group:          Development/Libraries
Source0:        RabbitMQ-Connection-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      x86_64
BuildRequires:  perl(ExtUtils::MakeMaker) rabbitmq-c
Requires:       perl(:MODULE_COMPAT_%(eval "`%{__perl} -V:version`"; echo $version)) rabbitmq-c

%description
RabbitMQ::Connection Perl extension to make use of the rabbitmq-c library.

%prep
%setup -q -n RabbitMQ-Connection-%{version}

%build
%{__perl} Makefile.PL INSTALLDIRS=vendor
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make pure_install PERL_INSTALL_ROOT=$RPM_BUILD_ROOT
find $RPM_BUILD_ROOT -type f -name .packlist -exec rm -f {} \;
find $RPM_BUILD_ROOT -depth -type d -exec rmdir {} 2>/dev/null \;

%{_fixperms} $RPM_BUILD_ROOT/*

# %check
# rm -rf /tmp/dbng-test
# mkdir -p /tmp/dbng-test
# TEST_BASE="/tmp/dbng-test" make test

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
#%doc META.json
%{perl_vendorarch}/*
