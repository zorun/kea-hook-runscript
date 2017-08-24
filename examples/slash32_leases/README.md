# Handing out IPv4 addresses in /32 subnets

The goal of this example is to lease IPv4 addresses individually (/32 design).
This is essentially a "out-of-subnet" reservation mode, because clients do not
need to be in the same IP subnet as the DHCP server.  This is mostly
useful to hand out public IPv4 addresses to customers.

This method is very flexible:

- each DHCP server can handle clients with very different IP addresses
  (there is no need to partition the network with static subnets)
- clients can "move" from one DHCP server to another while keeping
  their IP address (even when moving to a different layer 2 network),
  provided that a dynamic routing protocol is used.

The script adds a route in the Linux kernel whenever a DHCP client gets a lease,
so that the new client's IP address becomes reachable.  Similarly, when an IP address
is released or a lease expires, the route is removed from the kernel.  This assumes
that clients are directly connected to Kea (same layer 2 network), so it will not
work with DHCP relays.  In such cases, the DHCP relay itself should manage routes
towards clients, not Kea.

The routes added by the script can then be picked up by a routing daemon
(e.g. [Bird](http://bird.network.cz/)) and propagated in a IGP like OSPF.

On the client side, DHCP option 121 is used so that the client can use the
DHCP server as gateway without being in the same subnet.  Assuming the DHCP
server's IP is `10.250.250.1/32` and the client's leased IP is `192.0.2.201/32`,
the routing table of the client will look like this:

    10.250.250.1 dev eth0 proto kernel  scope link  src 192.0.2.201  metric 224 
    default via 10.250.250.1 dev eth0 src 192.0.2.201  metric 224 

