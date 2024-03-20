# Define the shared library name and version
lib_LTLIBRARIES = libnebapi.la

# Source files for the library
libnebapi_la_SOURCES = src/nebapi.c

# Include directories
INCLUDES = -I$(top_srcdir)/include

# Header files for the library
nobase_include_HEADERS = include/nebapi/nebapi.h

# Install the library
pkglib_LTLIBRARIES = $(lib_LTLIBRARIES)

# Tell Automake to install header files
include_HEADERS = include/nebapi/nebapi.h
