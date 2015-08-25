# All variables and this file are optional, if they are not present the PG and the
# makefiles will try to parse the correct values from the file system.
#
# Variables that specify exclusions can use % as a wildcard to specify that anything in
# that position will match. A partial path can also be specified to, for example, exclude
# a whole folder from the parsed paths from the file system
#
# Variables can be specified using = or +=
# = will clear the contents of that variable both specified from the file or the ones parsed
# from the file system
# += will add the values to the previous ones in the file or the ones parsed from the file 
# system
# 
# The PG can be used to detect errors in this file, just create a new project with this addon 
# and the PG will write to the console the kind of error and in which line it is

meta:
	ADDON_NAME = ofxXMPP
	ADDON_DESCRIPTION = Addon for using xmpp services like jabber, google talk and others
	ADDON_AUTHOR = Arturo Castro
	ADDON_TAGS = "chat" "google talk" "network"
	ADDON_URL = http://github.com/arturoc/ofxXMPP

common:
	# dependencies with other addons, a list of them separated by spaces 
	# or use += in several lines
	# ADDON_DEPENDENCIES =
	
	# include search paths, this will be usually parsed from the file system
	# but if the addon or addon libraries need special search paths they can be
	# specified here separated by spaces or one per line using +=
	# ADDON_INCLUDES =
	
	# any special flag that should be passed to the compiler when using this
	# addon
	# ADDON_CFLAGS =
	
	# any special flag that should be passed to the linker when using this
	# addon, also used for system libraries with -lname
	# ADDON_LDFLAGS =
	
	# linux only, any library that should be included in the project using
	# pkg-config
	# ADDON_PKG_CONFIG_LIBRARIES =
	
	# osx/iOS only, any framework that should be included in the project
	# ADDON_FRAMEWORKS =
	
	# source files, these will be usually parsed from the file system looking
	# in the src folders in libs and the root of the addon. if your addon needs
	# to include files in different places or a different set of files per platform
	# they can be specified here
	# ADDON_SOURCES =
	
	# some addons need resources to be copied to the bin/data folder of the project
	# specify here any files that need to be copied, you can use wildcards like * and ?
	# ADDON_DATA = 
	
	# when parsing the file system looking for libraries exclude this for all or
	# a specific platform
	# ADDON_LIBS_EXCLUDE =
	
	# when parsing the file system looking for sources exclude this for all or
	# a specific platform
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	
	# when parsing the file system looking for include paths exclude this for all or
	# a specific platform
    # ADDON_INCLUDES_EXCLUDE =
	
linux64:
	ADDON_LDFLAGS = -lresolv
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_schannel.c
	
linux:
	ADDON_LDFLAGS = -lresolv
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_schannel.c
	
win_cb:
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs\strophe\src\parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs\strophe\src\tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs\strophe\src\tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs\strophe\src\tls_schannel.c
	ADDON_INCLUDES += ../../../libs/openssl/include
	
vs:
	ADDON_LIBS += Iphlpapi.lib
	
	
linuxarmv6l:
	ADDON_LDFLAGS = -lresolv
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_schannel.c
	
	
linuxarmv7l:
	ADDON_LDFLAGS = -lresolv
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_schannel.c

android/armeabi:	
	ADDON_LDFLAGS = -lresolv
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_schannel.c
	
android/armeabi-v7a:	
	ADDON_LDFLAGS = -lresolv
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_schannel.c

osx:
	ADDON_LDFLAGS = -lresolv
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_schannel.c
    
ios:
	ADDON_LDFLAGS = -lresolv
	ADDON_CFLAGS = -DHAVE_MEMMOVE
	ADDON_SOURCES_EXCLUDE = libs/strophe/src/parser_libxml2.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_dummy.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_gnutls.c
	ADDON_SOURCES_EXCLUDE += libs/strophe/src/tls_schannel.c


