mmre(1) - General Commands Manual

# NAME

**mmre** - an RSS to email daemon

# SYNOPSIS

**mmre**
\[*-c&nbsp;config*]
\[*-l&nbsp;log*]
\[*-v*]

# DESCRIPTION

A daemon which periodically checks RSS feeds and sends new posts to an email
using SMTP.  It supports multiple users and feeds.

The options are as follows:

*-c*

> Specify the configuration file.  By default: /etc/mmre.ini

*-l*

> Specify the log file. By default: /var/log/mmre.log

*-v*

> Show verbose output

*-h*

> Show help text

# SEE ALSO

mmre(5)

# AUTHORS

Made by
Marek Ma&#353;karinec,
[marek@mrms.cz](mailto:marek@mrms.cz).

Debian - August 27, 2023
