JS/SCS: Secure Connection Services for Javascript

JS/SCS provides TLS/SSL-based services to Javacript.  Currently I'm
working on the 'file' service: essentially a way to mkdir, rmdir, mv,
cp, ls, and cat from within Javascript.

JS/SCS is an NPAPI plugin based on the FireBreath project. To build
JS/SCS, get the FireBreath source, and put this source tree in the
projects directory.  Follow the FireBreath build directions.  I'm
working on Ubuntu and have yet to test on Windows (where it
undoubtably does not work) and OS/X (where it also probably does not
work).
