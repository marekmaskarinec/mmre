MMRE(5) - File Formats Manual

# NAME

**mmre** - mmre configuration format

# DESCRIPTION

mmre uses a simple INI based configuration format. By default it is
/etc/mmre.ini. Example:

	[meta]
	interval = 20
	smtp_login = rss
	smtp_pwd = pa$$word1
	smtp_from = <rss@example.com>
	smtp_url = smtps://smtp.example.com
	smtp_domain = example.com
	
	[user]
	email = <example@example.com>
	url = https://mrms.cz/articles/rss.xml
	url = https://sr.ht/~mrms/mmre/feed.rss
	
	[user]
	email = <example2@example.com>
	url = https://mrms.cz/articles/rss.xml
	url = https://sr.ht/~mrms/mmre/feed.rss

# meta section

The meta section specifies non-user-specific configuration.

## interval

The interval in minutes at which feeds shall be checked.  It must be a non-zero
integer number.

## smtp\_login

This specifies the login of the outgoing address.

## smtp\_pwd

Password to the outgoing address.

## smtp\_from

The outgoing address, must be inside &lt;&gt;.

## smtp\_url

Specify the adress of the outgoing smtp server.

## smtp\_domain

Specify the domain, where the smtp server runs.  Should be same as in
smtp\_from.

# user section

The user section specifies per-user configuration.

## email

This specifies the email to which posts will be sent.  The email property
creates a new user.  The address must be inside &lt;&gt;.

## url

Add a feed by it's url.  This option can be used multiple times.

# SEE ALSO

mmre(1)

# AUTHOR

Marek Ma&#353;karinec,
[marek@mrms.cz](mailto:marek@mrms.cz).

Debian - August 27, 2023
