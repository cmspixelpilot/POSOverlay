%define _package DiagSystem
%define _packagename DiagSystem
%define _version __version__
%define _release __build__
%define _prefix  /opt/xdaq
%define _tmppath /tmp
%define _packagedir $BUILD_HOME/DiagSystem
%define _packagedirbase $BUILD_HOME
%define _os linux
%define _platform x86
%define _project pixel
%define _author aryd
%define _summary dummy
%define _url dummy
%define _buildarch x86

%define _unpackaged_files_terminate_build 0

#
# Binary RPM specified attributed (lib and bin)
#
Summary: %{_summary}
Name: %{_project}-%{_packagename}
Version: %{_version}
Release: %{_release}
Packager: %{_author}
#BuildArch: %{_buildarch}
License: BSD
Group: Applications/XDAQ
URL: %{_url}
BuildRoot: %{_tmppath}/%{_packagename}-%{_version}-%{_release}-buildroot
Prefix: %{_prefix}

%description


#
# Devel RPM specified attributed (extension to binary rpm with include files)
#
%package -n %{_project}-%{_packagename}-devel
Summary:  Development package for %{_summary}
Group:    Applications/XDAQ

%description -n %{_project}-%{_packagename}-devel


#%prep


#%setup 

#%build

#
# Prepare the list of files that are the input to the binary and devel RPMs
#
%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_prefix}/{bin,lib,include,etc}
mkdir -p $RPM_BUILD_ROOT/%{_prefix}/include/%{_package}/linux
mkdir -p $RPM_BUILD_ROOT/%{_prefix}/htdocs/%{_package}/{images,xml,html}
mkdir -p $RPM_BUILD_ROOT/%{_prefix}/dat/%{_package}

if [ -d %{_packagedir}/bin/%{_os}/%{_platform} ]; then 
	install -m 655 %{_packagedir}/bin/%{_os}/%{_platform}/* $RPM_BUILD_ROOT/%{_prefix}/bin/ 
fi
if [ -d %{_packagedir}/tools/lib ]; then
	install -m 655 %{_packagedir}/tools/lib/lib*.so $RPM_BUILD_ROOT/%{_prefix}/lib/
fi
if [ -d %{_packagedirbase}/pixel/XDAQConfiguration ]; then
	install -m 655 %{_packagedirbase}/pixel/XDAQConfiguration/GEDConfig.set $RPM_BUILD_ROOT/%{_prefix}/dat/%{_package}/
	install -m 655 %{_packagedirbase}/pixel/XDAQConfiguration/LogViewer.set $RPM_BUILD_ROOT/%{_prefix}/dat/%{_package}/
	install -m 655 %{_packagedirbase}/pixel/XDAQConfiguration/SupervisorConfig.set $RPM_BUILD_ROOT/%{_prefix}/dat/%{_package}/
fi
if [ -d %{_packagedirbase}/pixel/PixelMonitor/libcsv-3.0.0 ]; then
	install -m 655 %{_packagedirbase}/pixel/PixelMonitor/libcsv-3.0.0/libcsv.so $RPM_BUILD_ROOT/%{_prefix}/lib/
fi
if [ -d %{_packagedir}/../pixel/PixelMonitor/curlpp-0.6.1/./curlpp/.libs ]; then
	install -m 655 %{_packagedir}/../pixel/PixelMonitor/curlpp-0.6.1/./curlpp/.libs/libcurlpp.so.0 $RPM_BUILD_ROOT/%{_prefix}/lib/
fi
if [ -d %{_packagedir}/include ]; then
	cd %{_packagedir}/include; \
	find ./ \( -name "*.[hi]" -o -name "*.hpp" -o -name "*.hh" -o -name "*.hxx" \) -exec install -m 655 -D {} $RPM_BUILD_ROOT/%{_prefix}/include/{} \;
fi
if [ -d %{_packagedir}/scripts ]; then
	install -m 755 %{_packagedir}/scripts/*.* $RPM_BUILD_ROOT/%{_prefix}/bin
fi
if [ -d %{_packagedir}/etc ]; then
#install -m 644 %{_packagedir}/etc/*.* $RPM_BUILD_ROOT/%{_prefix}/etc
	cd %{_packagedir}/etc; \
	find ./ -name "*.*" -exec install -m 655 -D {} $RPM_BUILD_ROOT/%{_prefix}/etc/{} \;
fi
if [ -d %{_packagedir}/xml ]; then
	cd %{_packagedir}/xml; \
	find ./ -name "*.*" -exec install -m 655 -D {} $RPM_BUILD_ROOT/%{_prefix}/htdocs/%{_package}/xml/{} \;
fi
if [ -d %{_packagedir}/images ]; then
	cd %{_packagedir}/images; \
	find ./ -name "*.*" -exec install -m 655 -D {} $RPM_BUILD_ROOT/%{_prefix}/htdocs/%{_package}/images/{} \;
fi
if [ -d %{_packagedir}/html ]; then
	cd %{_packagedir}/html; \
	find ./ -name "*.*" -exec install -m 655 -D {} $RPM_BUILD_ROOT/%{_prefix}/htdocs/%{_package}/html/{} \;
fi

if [ -e %{_packagedir}/ChangeLog ]; then
	install -m 655 %{_packagedir}/ChangeLog %{_packagedir}/rpm/RPMBUILD/BUILD/ChangeLog
else
	touch %{_packagedir}/rpm/RPMBUILD/BUILD/ChangeLog
fi

if [ -e %{_packagedir}/README ]; then
	install -m 655 %{_packagedir}/README %{_packagedir}/rpm/RPMBUILD/BUILD/README
else
	touch %{_packagedir}/rpm/RPMBUILD/BUILD/README
fi

if [ -e %{_packagedir}/MAINTAINER ]; then
	install -m 655 %{_packagedir}/MAINTAINER %{_packagedir}/rpm/RPMBUILD/BUILD/MAINTAINER
else
	touch %{_packagedir}/rpm/RPMBUILD/BUILD/MAINTAINER
fi

%clean
rm -rf $RPM_BUILD_ROOT

#
# Files that go in the binary RPM
#
%files
%defattr(-,root,root,-)

# 
# Files required by Quattor
#
%doc MAINTAINER ChangeLog README

%dir
%{_prefix}/bin
%{_prefix}/lib
%{_prefix}/etc
%{_prefix}/htdocs/%{_package}/xml
%{_prefix}/htdocs/%{_package}/images
%{_prefix}/htdocs/%{_package}/html
%{_prefix}/dat/%{_package}

#
# Files that go in the devel RPM
#
%files -n %{_project}-%{_packagename}-devel
%defattr(-,root,root,-)
%{_prefix}/include

#%changelog

# 
# Files required by Quattor
#
# %doc MAINTAINER ChangeLog README
