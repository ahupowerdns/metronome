#!/bin/sh

aclocal -Im4
autoheader
autoconf
automake -ac
