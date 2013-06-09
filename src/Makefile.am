# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Authors:
# 2013-
#    Oscar Koeroo <okoeroo@gmail.nl>
#

lib_LTLIBRARIES = libhtaccess.la

libhtaccess_ladir = $(includedir)/htaccess/

libhtaccess_la_HEADERS = \
        htaccess/tree.h \
        htaccess/queue.h \
        htaccess/htaccess.h

libhtaccess_la_SOURCES = \
        config.h \
        htaccess.c

libhtaccess_la_LDFLAGS = -fstack-protector -avoid-version

bin_PROGRAMS = htaccess_main

htaccess_main_SOURCES = \
        main.c

htaccess_main_LDFLAGS = -fstack-protector -lhtaccess

