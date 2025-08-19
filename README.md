# MMRE - an RSS to email daemon

MMRE is a daemon that periodically checks RSS (or Atom) feeds and sends new
posts to email using SMTP. It has support for multiple users.

## install instructions

You need to install `libcurl-dev` and `libxml2-dev`.

1. clone this repository with the `--recursive` flag.
2. run `make install` with the appropriate privileges.

## contributing

Please format C source using `clang-format`. For source code documentation, see
[internal.md](internal.md).

## configuration

See [`mmre(5)`](https://mmq.cz/mmre/man5.html)

## command line flags

See [`mmre(1)`](https://mmq.cz/mmre/man1.html)

## license

See `LICENSE` in the repo root.
