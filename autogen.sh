#! /bin/bash
#
# tsarray - type-safe dynamic array library
# Copyright 2012, 2015, 2016, 2017 Israel G. Lugo
#
# This file is part of tsarray.
#
# tsarray is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# tsarray is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with tsarray.  If not, see <http://www.gnu.org/licenses/>.
#
# For suggestions, feedback or bug reports: israel.lugo@lugosys.com

#
# Bootstrap the build system
#
# Should be used when first checking out the sources, to create
# the necessary generated files (configure, config.guess, etc)
#


# Try to use autoreconf if possible
if ( autoreconf --version > /dev/null 2>&1 ); then
  autoreconf -i
else
  # autoreconf not available, manually launch the tools
  aclocal && autoconf && libtoolize -c && automake -ac && autoheader
fi
