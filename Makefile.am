
## Process this file with automake to produce Makefile.in 

## we need version 1.5 of automake for DIST_SUBDIRS to work and dist-bzip2.
AUTOMAKE_OPTIONS = 1.5 check-news dist-bzip2

## Note that the file dkms.conf is somewhat ephemeral.  It is included
## within the distribution tar file, but gets recreated from the
## dkms.conf.in file by ./configure, and gets removed by 'make distclean'.
## It would be nice if there was a simple way to create the files at
## "maintainer" level, rather than at the "configure" level, then they
## would stick around after a 'make distclean'.
EXTRA_DIST =  AUTHORS COPYING ChangeLog ChangeLog-1.xx README.md \
	      autogen.sh dkms.conf .gitignore

SUBDIRS = driver

## From automake documentation:
## Note that EXTRA_DIST can only handle files in the current
## directory; files in other directories will cause make dist runtime
## failures.

dist-hook:
	@if test -d "$(srcdir)/.git"; \
	then \
		echo Creating ChangeLog && \
		( cd "$(top_srcdir)" && \
		  echo '# Generated by Makefile.  Do not edit.'; echo; \
		  $(top_srcdir)/missing --run \
		  git log --name-only --date=iso --abbrev-commit --decorate ) \
		 > ChangeLog.tmp \
		&& mv -f ChangeLog.tmp $(distdir)/ChangeLog \
		|| ( rm -f ChangeLog.tmp ; \
		     echo Failed to generate ChangeLog >&2 ); \
	else \
		echo A git clone is required to generate a ChangeLog >&2; \
	fi
