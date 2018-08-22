# Changelog for kea-hook-runscript

## unreleased

- allow to run the script asynchronously (wait=false)

## 1.1.0 (2017-10-22)

- Add lots of DHCPv6 variables: DHCPv6 should now be usable
- Add some more DHCPv4 variables, especially for leases
- Safety fixes: fix potential NULL pointer dereference, and unsafe string usage
- Makefile: use standard CXX variable instead of hard-coding g++

## 1.0.0 (2017-08-24)

- Initial release
