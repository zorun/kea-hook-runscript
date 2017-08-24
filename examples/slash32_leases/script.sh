#!/bin/sh
# This script adds and removes routes in the Linux kernel whenever a DHCP client
# gets a lease or a lease expires.

# Protocol to use in "ip route"
PROTO=static

case "$1" in
  "lease4_select")
    # Only add route if FAKE_ALLOCATION is set to 0
    [ "${KEA_FAKE_ALLOCATION}" = "0" ] || break
    ip route replace "${KEA_LEASE4_ADDRESS}"/32 dev "${KEA_QUERY4_INTERFACE}" proto "${PROTO}"
    ;;
  "lease4_renew")
    ip route replace "${KEA_LEASE4_ADDRESS}"/32 dev "${KEA_QUERY4_INTERFACE}" proto "${PROTO}"
    ;;
  "lease4_release"|"lease4_expire")
    ip route del "${KEA_LEASE4_ADDRESS}"/32 proto "${PROTO}"
    ;;
  "lease4_decline")
    echo "$(date -R): received DHCPDECLINE on ${KEA_QUERY4_INTERFACE} from ${KEA_QUERY4_HWADDR} about ${KEA_LEASE4_ADDRESS}." >&2
    echo "Probably a duplicate IP assignment, not removing any route." >&2
    echo "Existing route for this IP: $(ip route show ${KEA_LEASE4_ADDRESS})" >&2
    echo "Existing ARP entry for this IP: $(ip neigh show ${KEA_LEASE4_ADDRESS})" >&2
    ;;
esac
