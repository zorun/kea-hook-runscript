#!/bin/sh
# This script adds and removes IPv6 routes in the Linux kernel whenever a DHCP client
# gets a lease or a lease expires.

# Protocol to use in "ip -6 route"
PROTO=static

add_ipv6_routes()
{
    if [ "$KEA_LEASE6_TYPE" = "IA_NA" ]; then
        # Add interface route towards client
        ip -6 route replace "${KEA_LEASE6_ADDRESS}"/64 dev "${KEA_QUERY6_INTERFACE}" proto "${PROTO}"
    fi
    if [ "$KEA_LEASE6_TYPE" = "IA_PD" ]; then
        # Add route for delegated prefix (next hop is the client)
        ip -6 route replace "${KEA_LEASE6_DELEGATED_PREFIX}" via "${KEA_QUERY6_REMOTE_ADDRESS}" dev "${KEA_QUERY6_INTERFACE}" proto "${PROTO}"
    fi
}

remove_ipv6_routes()
{
    if [ "$KEA_LEASE6_TYPE" = "IA_NA" ]; then
        ip -6 route delete "${KEA_LEASE6_ADDRESS}"/64 proto "${PROTO}"
    fi
    if [ "$KEA_LEASE6_TYPE" = "IA_PD" ]; then
        ip -6 route delete "${KEA_LEASE6_DELEGATED_PREFIX}" proto "${PROTO}"
    fi
}

case "$1" in
  "lease6_select")
    # Only add route if FAKE_ALLOCATION is set to 0
    [ "${KEA_FAKE_ALLOCATION}" = "0" ] || break
    add_ipv6_routes
    ;;
  "lease6_renew")
    add_ipv6_routes
    ;;
  "lease6_release"|"lease6_expire")
    remove_ipv6_routes
    ;;
esac
