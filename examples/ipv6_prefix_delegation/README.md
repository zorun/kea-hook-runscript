# Route management for IPv6 delegated prefixes

The goal here is to add/remove static IPv6 routes in the kernel whenever Kea delegates
an IPv6 prefix through DHCPv6-PD.

This is achieved by running [ipv6routes.sh](ipv6-routes.sh) with kea-hook-runscript.

The routes added by the script can then be picked up by a routing daemon
(e.g. [Bird](http://bird.network.cz/)) and propagated in a IGP like OSPF.

**Note:** the script also inserts routes for `IA_NA` addresses, because it is necessary
in our setup.  If you only need routes for delegated prefixes, adapt the script accordingly.

## Limitations

There is a potential issue when the IPv6 prefix reserved to a client is changed (e.g. if it is
modified in the Postgresql data source).  In that case, `lease6_release` / `lease6_expire` is
never called with the old prefix, so the corresponding route is never removed from the kernel.
It is not clear whether it is a bug in Kea or if it is related to the specific setup of the author.
In any case, this is something to watch out for.
