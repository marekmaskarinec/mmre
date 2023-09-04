# MMRE - a RSS to email daemon

MMRE is a daemon which periodically checks RSS (or Atom) feeds and sends new
posts to email using SMTP.  It has support for multiple users.

## install instructions

You need to install `libcurl-dev` and `libxml2-dev`.

1. clone this repository with the `--recursive` flag.
2. run `make install` with the appropriate privileges.

## configuration

See [`mmre(5)`](https://mrms.cz/mmre/man5.html)

## command line flags

See [`mmre(1)`](https://mrms.cz/mmre/man1.html)

## license

See `LICENSE` in the repo root.
