# Define the shared library name and version
lib_LTLIBRARIES = libnebapi.la

ACLOCAL_AMFLAGS = -I m4

# Source files for the library
libnebapi_la_SOURCES =  src/nebapi.c \
                        src/nebstructs_json.c \
                        src/mongoose.c

# Include directories
AM_CPPFLAGS = -I$(top_srcdir)/include

# Header files for the library
nobase_include_HEADERS = include/nebapi/nebapi.h

# Install the library
#pkglib_LTLIBRARIES = $(lib_LTLIBRARIES)

# Tell Automake to install header files
include_HEADERS = include/nebapi/nebapi.h
