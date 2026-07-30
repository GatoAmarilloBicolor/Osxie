# NOTE: this package MUST be built after building the main `osxie` package

#disable binary striping, in case this is a problem
%global __os_install_post %{nil}
#Disable debug packages, since these are emulated files mostly
%define debug_package %{nil}
%define commit_date %{getenv:OSXIE_COMMIT_DATE}
%if "%{commit_date}" == ""
	%define commit_date 0
%endif
%global _buildshell /bin/bash

Name:           osixie-cli-devenv
Version:        0.1.%{commit_date}
Release:        1%{?dist}
Summary:        Osxie developer environment

Group:          Utility
License:        GPLv3
URL:            https://www.osxiehq.org/
# Use this line for Source0 if there are ever official versions.
# Source0:        https://github.com/osxiehq/osixie/archive/%%{version}/%%{name}-%%{version}.tar.gz
Source0:        %{name}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  bash

Requires:       osixie-cli osixie-python2 osixie-ruby osixie-perl osixie-cli-devenv-gui-common osixie-cli-devenv-gui-stubs-common osixie-iosurface
Conflicts:      osixie-gui osixie-gui-stubs

%description
A Osxie environment for CLI-only programs for building and developing using the command line developer tools

%prep
%setup -q -n %{name}

%build
true

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
cp -rla . %{?buildroot}/

%files
%{_libexecdir}/osxie

%changelog
* Tue May 02 2023 Ariel Abreu <facekapow@outlook.com> - 0.1.20230502-1
- Initial version working for Fedora 37
- Split package off from `osxie` package
