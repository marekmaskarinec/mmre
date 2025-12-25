# internal documentation

## file structure

- `include/` - C header files
  - `mmre.h` - header file used by all MMRE sources
- `src/` - C source files
  - `conf.c` - configuration loading
  - `main.c` - application main loop, argument parsing
  - `net.c` - networking functions (HTTP GET, SMTP send)
  - `rss.c` - RSS parsing, checking for new posts
  - `util.c` - miscellaneous functions

## feed checking behavior

MMRE saves some data to `/var/lib/mmre`. In this folder, each user has a folder
named the same as their email. In this folder, each of the user's feeds has its
file. The file is named as a hex dump of a djb2 hash of the feed's URL. The file
contains a sorted list of the hashes of processed posts.

When a feed is checked, first we check if it already has a file. If it doesn't,
it is created and automatically filled with all the hashes in the feed (this is
so the user doesn't get bombarded with emails after adding a new feed). If a file
already exists, each of the post's hashes is searched for in the file using `bsearch`.

If the post isn't found in the file, an attempt to send it is made. If the send
succeeds, the hash is added to the file. Otherwise, it is not added to the file, so
it can be sent the other time feeds are processed.
