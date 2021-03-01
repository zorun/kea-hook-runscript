[![CI status](https://code.ffdn.org/zorun/kea-hook-runscript/badges/master/pipeline.svg)](https://code.ffdn.org/zorun/kea-hook-runscript/-/pipelines)

# About kea-hook-runscript

This a hook for the Kea DHCP server that allows to run an external script
at various points in the processing of DHCP requests and responses.

The goal is to simplify integration with Kea: for many simple use-cases,
it is overkill to have to write a full-blown Kea hook, where a simple shell script
can do the job.

This hook is licensed under the Mozilla Public License version 2 (MPL2).

## What it can and can't do

Integration is mostly done one-way: thanks to this hook, Kea passes information
to the external script, but the script cannot easily modify Kea's behaviour.

The **external script** can be any kind of executable program, but often it will
be a simple script (shell, Perl, Python...).  Information about what Kea is doing
is provided to the external script through environment variables: MAC address of
the requesting DHCP client, IP address being handed out, etc.

Each time Kea encounters a hook point, it will (by default) call the script
**synchronously**.
That is, Kea will do absolutely nothing else while the script is running.
Thus, it is a good idea to perform only lightweight processing in the script,
and absolutely avoid blocking operations.  Also, scripting languages that need
to initialise a huge interpreter (such as Python or Ruby) will cause a large
amount of CPU usage and a massive slowdown of Kea, because the script is run
multiple times for each DHCP transaction.

If you know what you are doing, you can optionally call the script **asynchronously**
by setting `wait` to `false` (see below).

This hook works for both DHCPv4 and DHCPv6, on Kea 1.1 and above.

In the future, the hook will possibly feed the return code of the external script
back into Kea.  This would allow the external script to cancel part of Kea's normal processing
(for instance, it could be possible to easily implement a flexible host blacklist this way).

## Alternative

Since Kea 1.9.5, a similar hook is provided by ISC: <https://kea.readthedocs.io/en/latest/arm/hooks.html#run-script-support>

It has similar functionalities: information is passed to the script through environment
variables.  However, only **asynchronous** execution is supported in ISC's hook, at
least as of Kea 1.9.5.

Which hook to use is up to you: the ISC one will probably be better maintained when
new versions of Kea come out, while this one supports synchronous execution which is safer.
Also, environment variables are different between the two hooks, which can be an important
factor if you need to process specific sub-options in your script.

## Use-cases

Given the limitations exposed above, here are some example use-cases for which
this hook is well-suited:

- add/remove routing entries when DHCP clients arrive or leave.  This can be useful
  when handing out IPv4 addressing in /32 subnets, or IPv6 Prefix Delegation
  with DHCPv6-PD.  An example is included in `examples/slash32_leases/`;
- update firewall rules to allow/refuse access to new DHCP clients;
- log information about successful leases.

For more complex use-cases, including non-trivial changes to Kea's behaviour,
it may be easier to just write a Kea hook yourself.

## Examples

If you have more examples of usage, feel free to contribute your Kea
config and your scripts!

### Managing routes for IPv6 delegated prefixes

When delegating IPv6 prefixes with DHCPv6-PD, it is necessary to add the corresponding routes
in the kernel.

This example script adds/removes static IPv6 routes whenever Kea delegates an IPv6 prefix
through DHCPv6-PD or when the lease expires.

See the included [README](examples/ipv6_prefix_delegation/README.md) for more
explanations and the [source](examples/ipv6_prefix_delegation) with the script and
an example Kea configuration.

### Handing out IPv4 addresses in /32 subnets

This example allows to lease IPv4 addresses individually (/32 subnets), by
inserting routes in the kernel each time a DHCP client connects, and
sending custom routes to clients using DHCP option 121.  This is mostly
useful to hand out public IPv4 addresses to customers.

See the included [README](examples/slash32_leases/README.md) for more
explanations and the [source](examples/slash32_leases) with the script and
an example Kea configuration.

### Debug script

To experiment, a simple debug script is provided: `examples/debug.sh`.  It
simply prints the name of the hook point and all environment variables
passed to it.

The output of the script is at `/tmp/kea-hook-runscript-debug.log`.  A nice way to debug
is to continously display the content of this file:

    tail -F /tmp/kea-hook-runscript-debug.log

## Pre-built binaries

Since version 1.4.0, we have a CI system to build the hook on various OS and for various
versions of Kea. It's new, so there might be bugs.

The binaries are available from the [release page](https://github.com/zorun/kea-hook-runscript/releases)
or you can directly
[browse through the pipeline results](https://code.ffdn.org/zorun/kea-hook-runscript/-/pipelines).

## How to build

If you want to build the hook yourself, you need the Kea libraries as well
as the Kea and Boost development headers.

### Using a packaged version of Kea

If you use a Kea package, you need the appropriate development packages:

- boost development files: `libboost-dev` or equivalent
- kea development files: `isc-kea-dev` from cloudsmith (official Kea package)

If you prefer using the Kea package from Debian, install `kea-dev` instead.
However, it is currently unsupported and is only available in sid.

Then, to build the hook, simply run:

    $ make -j4

### Using Kea source

To build against a local Kea source tree, assumed to be in `~/kea`:

- build Kea (`cd ~/kea && make -j`)
- install Kea to a local directory (`cd ~/kea && make install DESTDIR=/tmp/kea`)

Then build this hook with:

    $ export KEA_INCLUDE=$HOME/kea/src/lib
    $ export KEA_LIB=/tmp/kea/usr/local/lib
    $ make

### Supported Kea versions

Some notes on Kea versions:

- Kea 1.1 does not install all required headers (most notably `dhcpsrv/`),
  so you may need to build against Kea's source tree.
- Kea 1.2 is missing a header file by mistake, so depending on your
  distribution, you may need to manually copy `option6_pdexclude.h` from
  the Kea git repository to `/usr/include/kea/dhcp/`.
- Kea 1.3 to 1.7 should work out-of-the-box.
- Kea 1.8 needs to run without [multi-threading](https://kea.readthedocs.io/en/kea-1.8.0/arm/dhcp4-srv.html#multi-threading-settings).
  Open a ticket if you need multi-threading support.

## How to use this hook

If all goes well, you should obtain a `kea-hook-runscript.so` file.
Then, here is how to tell Kea to use this hook, for DHCPv4:

    {
    "Dhcp4":
    {
      "hooks-libraries": [
        {
          "library": "/path/to/hea-hook-runscript/kea-hook-runscript.so",
          "parameters": {
            "script": "/path/to/myscript.sh",
            "wait": true
          }
        }
      ],
      ...
    }
    }

The `wait` parameter indicates whether Kea waits for the script to exit.  That is,
if set to `true`, Kea will block while the script is running.
If you need high-performance DHCP, you can set it to `false`, but you must be prepared
to handle several instances of the script running in parallel.

You can use the same script for both DHCPv4 and DHCPv6, or use two different scripts.

The script will receive the name of the hook point as first argument, and all
relevant information available at the current hook point will be passed as
environment variables, documented below.

To debug, see the `examples/debug.sh` script described above.

Refer to the Kea documentation for more information about each hook point:

- DHCPv4 hooks reference: <https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html>
- DHCPv6 hooks reference: <https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html>

# Reference of variables passed to the external script

## DHCPv4 variables

Here are all possible variables for DHCPv4, with their type, description
and reference of the possible values.  Booleans are simply expressed with
`0` and `1`.

| Variable name                         | Type     | Description                                                 | Reference                                                                                                                                                       |
|---------------------------------------|----------|-------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `KEA_QUERY4_TYPE`                     | `string` | Type of DHCP message                                        | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#aa5bfdcc4861aa3dab5328dba89362016)                                  |
| `KEA_QUERY4_INTERFACE`                | `string` | Interface on which query was received                       |                                                                                                                                                                 |
| `KEA_QUERY4_IFINDEX`                  | `int`    | Index of the interface on which query was received          |                                                                                                                                                                 |
| `KEA_QUERY4_HWADDR`                   | `string` | Hardware address of the client (its MAC address)            |                                                                                                                                                                 |
| `KEA_QUERY4_HWADDR_TYPE`              | `int`    | Type of hardware address                                    | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#addcff933049489d800f9869196c8e46fa96a62c59182d6e06780b0e1ef40da059) |
| `KEA_QUERY4_HWADDR_SOURCE`            | `int`    | How this MAC address was obtained                           | [dhcp/hwaddr.h](https://jenkins.isc.org/job/Kea_doc/doxygen/da/dae/group__hw__sources.html)                                                                     |
| `KEA_QUERY4_RELAYED`                  | `bool`   | Whether query was relayed                                   | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html#a8468401827b9bacdd3796bb4e20d8e5e)                               |
| `KEA_QUERY4_RELAY_HOPS`               | `int`    | Number of relay agents traversed                            |                                                                                                                                                                 |
| `KEA_QUERY4_OPTION60`                 | `string` | Option 60 - vendor id                                       |                                                                                                                                                                 |
| `KEA_QUERY4_CIADDR`                   | `string` | Client IP address                                           | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html)                                                                 |
| `KEA_QUERY4_SIADDR`                   | `string` | Server IP address                                           | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html)                                                                 |
| `KEA_QUERY4_YIADDR`                   | `string` | Your IP address                                             | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html)                                                                 |
| `KEA_QUERY4_GIADDR`                   | `string` | Gateway IP address (inserted by DHCP relay)                 | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html)                                                                 |
| `KEA_QUERY4_RAI`                      | `string` | Relay Agent Information (RFC 3046) as hex string            |                                                                                                                                                                 |
| `KEA_QUERY4_RAI_CIRCUIT_ID`           | `string` | RAI sub-option 1 Circuit id (RFC 3046) as hex string        |                                                                                                                                                                 |
| `KEA_QUERY4_RAI_REMOTE_ID`            | `string` | RAI sub-option 2 Remote id (RFC 3046) as hex string         |                                                                                                                                                                 |
| `KEA_RESPONSE4_TYPE`                  | `string` | Type of DHCP message                                        | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#aa5bfdcc4861aa3dab5328dba89362016)                                  |
| `KEA_RESPONSE4_INTERFACE`             | `string` | Interface on which response is being sent                   |                                                                                                                                                                 |
| `KEA_RESPONSE4_IFINDEX`               | `int`    | Index of the interface on which response is being sent      |                                                                                                                                                                 |
| `KEA_RESPONSE4_HWADDR`                | `string` | Hardware address of the client (its MAC address)            |                                                                                                                                                                 |
| `KEA_RESPONSE4_HWADDR_TYPE`           | `int`    | Type of hardware address                                    | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#addcff933049489d800f9869196c8e46fa96a62c59182d6e06780b0e1ef40da059) |
| `KEA_RESPONSE4_HWADDR_SOURCE`         | `int`    | How this MAC address was obtained                           | [dhcp/hwaddr.h](https://jenkins.isc.org/job/Kea_doc/doxygen/da/dae/group__hw__sources.html)                                                                     |
| `KEA_RESPONSE4_RELAYED`               | `bool`   | Whether response is relayed                                 | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html#a8468401827b9bacdd3796bb4e20d8e5e)                               |
| `KEA_RESPONSE4_RELAY_HOPS`            | `int`    | Number of relay agents traversed                            |                                                                                                                                                                 |
| `KEA_RESPONSE4_CIADDR`                | `string` | Client IP address                                           | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html)                                                                 |
| `KEA_RESPONSE4_SIADDR`                | `string` | Server IP address                                           | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html)                                                                 |
| `KEA_RESPONSE4_YIADDR`                | `string` | Your IP address                                             | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html)                                                                 |
| `KEA_RESPONSE4_GIADDR`                | `string` | Gateway IP address                                          | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html)                                                                 |
| `KEA_SUBNET4_PREFIX`                  | `IPv4`   | IP prefix of the subnet (without prefix length)             |                                                                                                                                                                 |
| `KEA_SUBNET4_PREFIXLEN`               | `int`    | Prefix length of the subnet (`0` to `32`)                   |                                                                                                                                                                 |
| `KEA_SUBNET4`                         | `string` | `KEA_SUBNET4_PREFIX`/`KEA_SUBNET4_PREFIXLEN`                |                                                                                                                                                                 |
| `KEA_LEASE4_ADDRESS`                  | `IPv4`   | IPv4 address leased to client                               |                                                                                                                                                                 |
| `KEA_LEASE4_TYPE`                     | `string` | Type of lease, always equal to "V4"                         |                                                                                                                                                                 |
| `KEA_LEASE4_HWADDR`                   | `string` | Hardware address of the client                              |                                                                                                                                                                 |
| `KEA_LEASE4_HOSTNAME`                 | `string` | Hostname associated to the client                           |                                                                                                                                                                 |
| `KEA_LEASE4_STATE`                    | `string` | Current state of the lease                                  | [dhcpsrv/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d0/dee/structisc_1_1dhcp_1_1Lease.html#a7075e6229e9eadedf27fc9ff49ece3c1)                         |
| `KEA_LEASE4_IS_EXPIRED`               | `bool`   | Whether the lease is expired                                |                                                                                                                                                                 |
| `KEA_LEASE4_CLIENT_LAST_TRANSMISSION` | `int`    | Unix timestamp of the last message received from the client | [dhcpsrv/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d0/dee/structisc_1_1dhcp_1_1Lease.html#ac71dc7f97dd753096a0f448c6649cdcf)                         |
| `KEA_LEASE4_VALID_LIFETIME`           | `int`    | Valid lifetime of the lease, in seconds                     | [dhcpsrv/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d0/dee/structisc_1_1dhcp_1_1Lease.html#a615302a9140991942225b9809ddd50fb)                         |
| `KEA_REMOVE_LEASE`                    | `bool`   | Whether the lease should be removed from the lease database | [DHCPv4 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Expire)                                                  |
| `KEA_FAKE_ALLOCATION`                 | `bool`   | Whether the query is a DISCOVER or a REQUEST                | [DHCPv4 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLeaseSelect)                                                   |

## DHCPv4 hook points

For each Kea hook point, here are all variables usable in the external
script.

### [`pkt4_receive`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksPkt4Receive)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_IFINDEX`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_QUERY4_RELAY_HOPS`
- `KEA_QUERY4_OPTION60`
- `KEA_QUERY4_CIADDR`
- `KEA_QUERY4_SIADDR`
- `KEA_QUERY4_YIADDR`
- `KEA_QUERY4_GIADDR`
- `KEA_QUERY4_RAI`
- `KEA_QUERY4_RAI_CIRCUIT_ID`
- `KEA_QUERY4_RAI_REMOTE_ID`

### [`pkt4_send`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksPkt4Send)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_IFINDEX`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_QUERY4_RELAY_HOPS`
- `KEA_QUERY4_OPTION60`
- `KEA_QUERY4_CIADDR`
- `KEA_QUERY4_SIADDR`
- `KEA_QUERY4_YIADDR`
- `KEA_QUERY4_GIADDR`
- `KEA_QUERY4_RAI`
- `KEA_QUERY4_RAI_CIRCUIT_ID`
- `KEA_QUERY4_RAI_REMOTE_ID`
- `KEA_RESPONSE4_TYPE`
- `KEA_RESPONSE4_INTERFACE`
- `KEA_RESPONSE4_IFINDEX`
- `KEA_RESPONSE4_HWADDR`
- `KEA_RESPONSE4_HWADDR_SOURCE`
- `KEA_RESPONSE4_HWADDR_TYPE`
- `KEA_RESPONSE4_RELAYED`
- `KEA_RESPONSE4_RELAY_HOPS`
- `KEA_RESPONSE4_CIADDR`
- `KEA_RESPONSE4_SIADDR`
- `KEA_RESPONSE4_YIADDR`
- `KEA_RESPONSE4_GIADDR`

### [`subnet4_select`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksSubnet4Select)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_IFINDEX`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_QUERY4_RELAY_HOPS`
- `KEA_QUERY4_CIADDR`
- `KEA_QUERY4_SIADDR`
- `KEA_QUERY4_YIADDR`
- `KEA_QUERY4_GIADDR`
- `KEA_QUERY4_RAI`
- `KEA_QUERY4_RAI_CIRCUIT_ID`
- `KEA_QUERY4_RAI_REMOTE_ID`
- `KEA_SUBNET4_PREFIX`
- `KEA_SUBNET4_PREFIXLEN`
- `KEA_SUBNET4`

### [`lease4_select`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Select)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_IFINDEX`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_QUERY4_RELAY_HOPS`
- `KEA_QUERY4_CIADDR`
- `KEA_QUERY4_SIADDR`
- `KEA_QUERY4_YIADDR`
- `KEA_QUERY4_GIADDR`
- `KEA_QUERY4_RAI`
- `KEA_QUERY4_RAI_CIRCUIT_ID`
- `KEA_QUERY4_RAI_REMOTE_ID`
- `KEA_SUBNET4_PREFIX`
- `KEA_SUBNET4_PREFIXLEN`
- `KEA_SUBNET4`
- `KEA_FAKE_ALLOCATION`
- `KEA_LEASE4_ADDRESS`
- `KEA_LEASE4_TYPE`
- `KEA_LEASE4_STATE`
- `KEA_LEASE4_IS_EXPIRED`
- `KEA_LEASE4_HWADDR`
- `KEA_LEASE4_HOSTNAME`
- `KEA_LEASE4_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE4_VALID_LIFETIME`

### [`lease4_renew`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Renew)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_IFINDEX`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_QUERY4_RELAY_HOPS`
- `KEA_QUERY4_CIADDR`
- `KEA_QUERY4_SIADDR`
- `KEA_QUERY4_YIADDR`
- `KEA_QUERY4_GIADDR`
- `KEA_QUERY4_RAI`
- `KEA_QUERY4_RAI_CIRCUIT_ID`
- `KEA_QUERY4_RAI_REMOTE_ID`
- `KEA_SUBNET4_PREFIX`
- `KEA_SUBNET4_PREFIXLEN`
- `KEA_SUBNET4`
- `KEA_LEASE4_ADDRESS`
- `KEA_LEASE4_TYPE`
- `KEA_LEASE4_STATE`
- `KEA_LEASE4_IS_EXPIRED`
- `KEA_LEASE4_HWADDR`
- `KEA_LEASE4_HOSTNAME`
- `KEA_LEASE4_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE4_VALID_LIFETIME`

### [`lease4_release`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Release)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_IFINDEX`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_QUERY4_RELAY_HOPS`
- `KEA_QUERY4_CIADDR`
- `KEA_QUERY4_SIADDR`
- `KEA_QUERY4_YIADDR`
- `KEA_QUERY4_GIADDR`
- `KEA_QUERY4_RAI`
- `KEA_QUERY4_RAI_CIRCUIT_ID`
- `KEA_QUERY4_RAI_REMOTE_ID`
- `KEA_LEASE4_ADDRESS`
- `KEA_LEASE4_TYPE`
- `KEA_LEASE4_STATE`
- `KEA_LEASE4_IS_EXPIRED`
- `KEA_LEASE4_HWADDR`
- `KEA_LEASE4_HOSTNAME`
- `KEA_LEASE4_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE4_VALID_LIFETIME`

### [`lease4_decline`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Decline)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_IFINDEX`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_QUERY4_RELAY_HOPS`
- `KEA_QUERY4_CIADDR`
- `KEA_QUERY4_SIADDR`
- `KEA_QUERY4_YIADDR`
- `KEA_QUERY4_GIADDR`
- `KEA_QUERY4_RAI`
- `KEA_QUERY4_RAI_CIRCUIT_ID`
- `KEA_QUERY4_RAI_REMOTE_ID`
- `KEA_LEASE4_ADDRESS`
- `KEA_LEASE4_TYPE`
- `KEA_LEASE4_STATE`
- `KEA_LEASE4_IS_EXPIRED`
- `KEA_LEASE4_HWADDR`
- `KEA_LEASE4_HOSTNAME`
- `KEA_LEASE4_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE4_VALID_LIFETIME`

### [`lease4_expire`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Expire)

- `KEA_LEASE4_ADDRESS`
- `KEA_LEASE4_TYPE`
- `KEA_LEASE4_STATE`
- `KEA_LEASE4_IS_EXPIRED`
- `KEA_LEASE4_HWADDR`
- `KEA_LEASE4_HOSTNAME`
- `KEA_LEASE4_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE4_VALID_LIFETIME`
- `KEA_REMOVE_LEASE`

### [`lease4_recover`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Recover)

- `KEA_LEASE4_ADDRESS`
- `KEA_LEASE4_TYPE`
- `KEA_LEASE4_STATE`
- `KEA_LEASE4_IS_EXPIRED`
- `KEA_LEASE4_HWADDR`
- `KEA_LEASE4_HOSTNAME`
- `KEA_LEASE4_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE4_VALID_LIFETIME`


## DHCPv6 variables

Here are all possible variables for DHCPv6, with their type, description
and reference of the possible values.  Booleans are simply expressed with
`0` and `1`.

| Variable name                         | Type     | Description                                                                         | Reference                                                                                                                                |
|---------------------------------------|----------|-------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------|
| `KEA_QUERY6_TYPE`                     | `string` | Type of DHCPv6 message                                                              | [dhcp/dhcp6.h](https://jenkins.isc.org/job/Kea_doc/doxygen/db/d87/dhcp6_8h_source.html)                                                  |
| `KEA_QUERY6_INTERFACE`                | `string` | Interface on which query was received                                               |                                                                                                                                          |
| `KEA_QUERY6_IFINDEX`                  | `int`    | Index of the interface on which query was received                                  |                                                                                                                                          |
| `KEA_QUERY6_DUID`                     | `string` | TODO                                                                                |                                                                                                                                          |
| `KEA_QUERY6_HWADDR`                   | `string` | Hardware address of the client (its MAC address)                                    |                                                                                                                                          |
| `KEA_QUERY6_HWADDR_TYPE`              | `int`    | Type of hardware address                                                            | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#addcff933049489d800f9869196c8e46f)           |
| `KEA_QUERY6_HWADDR_SOURCE`            | `int`    | How this MAC address was obtained                                                   | [dhcp/hwaddr.h](https://jenkins.isc.org/job/Kea_doc/doxygen/da/dae/group__hw__sources.html)                                              |
| `KEA_QUERY6_LOCAL_ADDRESS`            | `string` | Local IPv6 address on which the query was received (link-local or multicast)        | [dhcp/pkt.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d71/classisc_1_1dhcp_1_1Pkt.html#a55b5c3f4cbab0f60968b0498d8543c65)          |
| `KEA_QUERY6_LOCAL_PORT`               | `int`    | Local UDP or TCP port                                                               |                                                                                                                                          |
| `KEA_QUERY6_REMOTE_ADDRESS`           | `string` | Remote IPv6 address, from which the query was received (link-local)                 | [dhcp/pkt.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d71/classisc_1_1dhcp_1_1Pkt.html#a1e20bcdc69d5f97ed8cc48290017b8d9)          |
| `KEA_QUERY6_REMOTE_PORT`              | `int`    | Remote UDP or TCP port                                                              |                                                                                                                                          |
| `KEA_QUERY6_LABEL`                    | `string` | Unique identifier of the query, to be used e.g. in log messages                     | [dhcp/pkt.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d71/classisc_1_1dhcp_1_1Pkt.html#af9888e61c5304f4bac1983a93ac6a473)          |
| `KEA_QUERY6_TRANSACTION_ID`           | `int`    | Transaction ID of the query                                                         | [dhcp/pkt.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d71/classisc_1_1dhcp_1_1Pkt.html#a8cd6c6ab6c434b1bf6949bb1cc4102b1)          |
| `KEA_RESPONSE6_TYPE`                  | `string` | Type of DHCPv6 message                                                              | [dhcp/dhcp6.h](https://jenkins.isc.org/job/Kea_doc/doxygen/db/d87/dhcp6_8h_source.html)                                                  |
| `KEA_RESPONSE6_INTERFACE`             | `string` | Interface on which response is being sent                                           |                                                                                                                                          |
| `KEA_RESPONSE6_IFINDEX`               | `int`    | Index of the interface on which response is being sent                              |                                                                                                                                          |
| `KEA_RESPONSE6_DUID`                  | `string` | TODO                                                                                |                                                                                                                                          |
| `KEA_RESPONSE6_HWADDR`                | `string` | Hardware address of the client (its MAC address)                                    |                                                                                                                                          |
| `KEA_RESPONSE6_HWADDR_TYPE`           | `int`    | Type of hardware address                                                            | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#addcff933049489d800f9869196c8e46f)           |
| `KEA_RESPONSE6_HWADDR_SOURCE`         | `int`    | How this MAC address was obtained                                                   | [dhcp/hwaddr.h](https://jenkins.isc.org/job/Kea_doc/doxygen/da/dae/group__hw__sources.html)                                              |
| `KEA_RESPONSE6_LOCAL_ADDRESS`         | `string` | Local IPv6 address, from which the response is being sent (link-local or multicast) | [dhcp/pkt.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d71/classisc_1_1dhcp_1_1Pkt.html#a55b5c3f4cbab0f60968b0498d8543c65)          |
| `KEA_RESPONSE6_LOCAL_PORT`            | `int`    | Local UDP or TCP port                                                               |                                                                                                                                          |
| `KEA_RESPONSE6_REMOTE_ADDRESS`        | `string` | Remote IPv6 address, to which the response is being sent (link-local)               | [dhcp/pkt.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d71/classisc_1_1dhcp_1_1Pkt.html#a1e20bcdc69d5f97ed8cc48290017b8d9)          |
| `KEA_RESPONSE6_REMOTE_PORT`           | `int`    | Remote UDP or TCP port                                                              |                                                                                                                                          |
| `KEA_RESPONSE6_LABEL`                 | `string` | Unique identifier of the response, to be used e.g. in log messages                  | [dhcp/pkt.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d71/classisc_1_1dhcp_1_1Pkt.html#af9888e61c5304f4bac1983a93ac6a473)          |
| `KEA_RESPONSE6_TRANSACTION_ID`        | `int`    | Transaction ID of the response                                                      | [dhcp/pkt.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d71/classisc_1_1dhcp_1_1Pkt.html#a8cd6c6ab6c434b1bf6949bb1cc4102b1)          |
| `KEA_SUBNET6_PREFIX`                  | `IPv6`   | IP prefix of the subnet (without prefix length)                                     |                                                                                                                                          |
| `KEA_SUBNET6_PREFIXLEN`               | `int`    | Prefix length of the subnet (`0` to `128`)                                          |                                                                                                                                          |
| `KEA_SUBNET6`                         | `string` | `KEA_SUBNET6_PREFIX`/`KEA_SUBNET6_PREFIXLEN`                                        |                                                                                                                                          |
| `KEA_LEASE6_TYPE`                     | `string` | Type of lease, either "NA", "TA", or "PD"                                           | [dhcp/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d0/dee/structisc_1_1dhcp_1_1Lease.html#a9257a6a410119ea79f29b9d2756c8769)     |
| `KEA_LEASE6_ADDRESS`                  | `IPv6`   | IPv6 address leased to client                                                       |                                                                                                                                          |
| `KEA_LEASE6_DELEGATED_PREFIX`         | `string` | For TYPE="PD", prefix delegated to client (in `prefix/prefixlen` form)              |                                                                                                                                          |
| `KEA_LEASE6_DELEGATED_PREFIXLEN`      | `int`    | For TYPE="PD", length of the prefix delegated to client                             |                                                                                                                                          |
| `KEA_LEASE6_CLIENT_DUID`              | `string` | DUID of the client                                                                  |                                                                                                                                          |
| `KEA_LEASE6_HWADDR`                   | `string` | Hardware address of the client                                                      |                                                                                                                                          |
| `KEA_LEASE6_HOSTNAME`                 | `string` | Hostname associated to the client                                                   |                                                                                                                                          |
| `KEA_LEASE6_STATE`                    | `string` | Current state of the lease                                                          | [dhcpsrv/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d0/dee/structisc_1_1dhcp_1_1Lease.html#a7075e6229e9eadedf27fc9ff49ece3c1)  |
| `KEA_LEASE6_IS_EXPIRED`               | `bool`   | Whether the lease is expired                                                        |                                                                                                                                          |
| `KEA_LEASE6_CLIENT_LAST_TRANSMISSION` | `int`    | Unix timestamp of the last message received from the client                         | [dhcpsrv/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d0/dee/structisc_1_1dhcp_1_1Lease.html#ac71dc7f97dd753096a0f448c6649cdcf)  |
| `KEA_LEASE6_VALID_LIFETIME`           | `int`    | Valid lifetime of the lease, in seconds                                             | [dhcpsrv/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d0/dee/structisc_1_1dhcp_1_1Lease.html#a615302a9140991942225b9809ddd50fb)  |
| `KEA_LEASE6_PREFERRED_LIFETIME`       | `int`    | Preferred lifetime of the lease, in seconds                                         | [dhcpsrv/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/da/ddc/structisc_1_1dhcp_1_1Lease6.html#acece7ab17d67a657637cf16a9a2f1f6e) |
| `KEA_LEASE6_IAID`                     | `string` | Identity Association Identifier, to differentiate between IA containers             | [dhcpsrv/lease.h](https://jenkins.isc.org/job/Kea_doc/doxygen/da/ddc/structisc_1_1dhcp_1_1Lease6.html#acc2e175c33e09dbdc8c93b943488431e) |
| `KEA_REMOVE_LEASE`                    | `bool`   | Whether the lease should be removed from the lease database                         | [DHCPv6 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Expire)                           |
| `KEA_FAKE_ALLOCATION`                 | `bool`   | Whether the query is a SOLICIT or a REQUEST                                         | [DHCPv6 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Select)                           |

## DHCPv6 hook points

For each Kea hook point, here are all variables usable in the external
script.

### [`pkt6_receive`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksPkt6Receive)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_IFINDEX`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_QUERY6_HWADDR_TYPE`
- `KEA_QUERY6_HWADDR_SOURCE`
- `KEA_QUERY6_LOCAL_ADDRESS`
- `KEA_QUERY6_LOCAL_PORT`
- `KEA_QUERY6_REMOTE_ADDRESS`
- `KEA_QUERY6_REMOTE_PORT`
- `KEA_QUERY6_LABEL`
- `KEA_QUERY6_TRANSACTION_ID`

### [`pkt6_send`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksPkt6Send)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_IFINDEX`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_QUERY6_HWADDR_TYPE`
- `KEA_QUERY6_HWADDR_SOURCE`
- `KEA_QUERY6_LOCAL_ADDRESS`
- `KEA_QUERY6_LOCAL_PORT`
- `KEA_QUERY6_REMOTE_ADDRESS`
- `KEA_QUERY6_REMOTE_PORT`
- `KEA_QUERY6_LABEL`
- `KEA_QUERY6_TRANSACTION_ID`
- `KEA_RESPONSE6_TYPE`
- `KEA_RESPONSE6_INTERFACE`
- `KEA_RESPONSE6_IFINDEX`
- `KEA_RESPONSE6_DUID`
- `KEA_RESPONSE6_HWADDR`
- `KEA_RESPONSE6_HWADDR_TYPE`
- `KEA_RESPONSE6_HWADDR_SOURCE`
- `KEA_RESPONSE6_LOCAL_ADDRESS`
- `KEA_RESPONSE6_LOCAL_PORT`
- `KEA_RESPONSE6_REMOTE_ADDRESS`
- `KEA_RESPONSE6_REMOTE_PORT`
- `KEA_RESPONSE6_LABEL`
- `KEA_RESPONSE6_TRANSACTION_ID`

### [`subnet6_select`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksSubnet6Select)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_IFINDEX`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_QUERY6_HWADDR_TYPE`
- `KEA_QUERY6_HWADDR_SOURCE`
- `KEA_QUERY6_LOCAL_ADDRESS`
- `KEA_QUERY6_LOCAL_PORT`
- `KEA_QUERY6_REMOTE_ADDRESS`
- `KEA_QUERY6_REMOTE_PORT`
- `KEA_QUERY6_LABEL`
- `KEA_QUERY6_TRANSACTION_ID`
- `KEA_SUBNET6_PREFIX`
- `KEA_SUBNET6_PREFIXLEN`
- `KEA_SUBNET6`

### [`lease6_select`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Select)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_IFINDEX`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_QUERY6_HWADDR_TYPE`
- `KEA_QUERY6_HWADDR_SOURCE`
- `KEA_QUERY6_LOCAL_ADDRESS`
- `KEA_QUERY6_LOCAL_PORT`
- `KEA_QUERY6_REMOTE_ADDRESS`
- `KEA_QUERY6_REMOTE_PORT`
- `KEA_QUERY6_LABEL`
- `KEA_QUERY6_TRANSACTION_ID`
- `KEA_SUBNET6_PREFIX`
- `KEA_SUBNET6_PREFIXLEN`
- `KEA_SUBNET6`
- `KEA_FAKE_ALLOCATION`
- `KEA_LEASE6_TYPE`
- `KEA_LEASE6_ADDRESS`
- `KEA_LEASE6_DELEGATED_PREFIX`
- `KEA_LEASE6_DELEGATED_PREFIXLEN`
- `KEA_LEASE6_CLIENT_DUID`
- `KEA_LEASE6_HWADDR`
- `KEA_LEASE6_HOSTNAME`
- `KEA_LEASE6_STATE`
- `KEA_LEASE6_IS_EXPIRED`
- `KEA_LEASE6_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE6_VALID_LIFETIME`
- `KEA_LEASE6_PREFERRED_LIFETIME`
- `KEA_LEASE6_IAID`

### [`lease6_renew`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Renew)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_IFINDEX`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_QUERY6_HWADDR_TYPE`
- `KEA_QUERY6_HWADDR_SOURCE`
- `KEA_QUERY6_LOCAL_ADDRESS`
- `KEA_QUERY6_LOCAL_PORT`
- `KEA_QUERY6_REMOTE_ADDRESS`
- `KEA_QUERY6_REMOTE_PORT`
- `KEA_QUERY6_LABEL`
- `KEA_QUERY6_TRANSACTION_ID`
- `KEA_LEASE6_TYPE`
- `KEA_LEASE6_ADDRESS`
- `KEA_LEASE6_DELEGATED_PREFIX`
- `KEA_LEASE6_DELEGATED_PREFIXLEN`
- `KEA_LEASE6_CLIENT_DUID`
- `KEA_LEASE6_HWADDR`
- `KEA_LEASE6_HOSTNAME`
- `KEA_LEASE6_STATE`
- `KEA_LEASE6_IS_EXPIRED`
- `KEA_LEASE6_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE6_VALID_LIFETIME`
- `KEA_LEASE6_PREFERRED_LIFETIME`
- `KEA_LEASE6_IAID`

### [`lease6_rebind`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Rebind)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_IFINDEX`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_QUERY6_HWADDR_TYPE`
- `KEA_QUERY6_HWADDR_SOURCE`
- `KEA_QUERY6_LOCAL_ADDRESS`
- `KEA_QUERY6_LOCAL_PORT`
- `KEA_QUERY6_REMOTE_ADDRESS`
- `KEA_QUERY6_REMOTE_PORT`
- `KEA_QUERY6_LABEL`
- `KEA_QUERY6_TRANSACTION_ID`
- `KEA_LEASE6_TYPE`
- `KEA_LEASE6_ADDRESS`
- `KEA_LEASE6_DELEGATED_PREFIX`
- `KEA_LEASE6_DELEGATED_PREFIXLEN`
- `KEA_LEASE6_CLIENT_DUID`
- `KEA_LEASE6_HWADDR`
- `KEA_LEASE6_HOSTNAME`
- `KEA_LEASE6_STATE`
- `KEA_LEASE6_IS_EXPIRED`
- `KEA_LEASE6_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE6_VALID_LIFETIME`
- `KEA_LEASE6_PREFERRED_LIFETIME`
- `KEA_LEASE6_IAID`

### [`lease6_decline`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Decline)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_QUERY6_HWADDR_TYPE`
- `KEA_QUERY6_HWADDR_SOURCE`
- `KEA_QUERY6_LOCAL_ADDRESS`
- `KEA_QUERY6_LOCAL_PORT`
- `KEA_QUERY6_REMOTE_ADDRESS`
- `KEA_QUERY6_REMOTE_PORT`
- `KEA_QUERY6_LABEL`
- `KEA_QUERY6_TRANSACTION_ID`
- `KEA_LEASE6_TYPE`
- `KEA_LEASE6_ADDRESS`
- `KEA_LEASE6_DELEGATED_PREFIX`
- `KEA_LEASE6_DELEGATED_PREFIXLEN`
- `KEA_LEASE6_CLIENT_DUID`
- `KEA_LEASE6_HWADDR`
- `KEA_LEASE6_HOSTNAME`
- `KEA_LEASE6_STATE`
- `KEA_LEASE6_IS_EXPIRED`
- `KEA_LEASE6_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE6_VALID_LIFETIME`
- `KEA_LEASE6_PREFERRED_LIFETIME`
- `KEA_LEASE6_IAID`

### [`lease6_release`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Release)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_IFINDEX`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_QUERY6_HWADDR_TYPE`
- `KEA_QUERY6_HWADDR_SOURCE`
- `KEA_QUERY6_LOCAL_ADDRESS`
- `KEA_QUERY6_LOCAL_PORT`
- `KEA_QUERY6_REMOTE_ADDRESS`
- `KEA_QUERY6_REMOTE_PORT`
- `KEA_QUERY6_LABEL`
- `KEA_QUERY6_TRANSACTION_ID`
- `KEA_LEASE6_TYPE`
- `KEA_LEASE6_ADDRESS`
- `KEA_LEASE6_DELEGATED_PREFIX`
- `KEA_LEASE6_DELEGATED_PREFIXLEN`
- `KEA_LEASE6_CLIENT_DUID`
- `KEA_LEASE6_HWADDR`
- `KEA_LEASE6_HOSTNAME`
- `KEA_LEASE6_STATE`
- `KEA_LEASE6_IS_EXPIRED`
- `KEA_LEASE6_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE6_VALID_LIFETIME`
- `KEA_LEASE6_PREFERRED_LIFETIME`
- `KEA_LEASE6_IAID`

### [`lease6_expire`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Expire)

- `KEA_LEASE6_TYPE`
- `KEA_LEASE6_ADDRESS`
- `KEA_LEASE6_DELEGATED_PREFIX`
- `KEA_LEASE6_DELEGATED_PREFIXLEN`
- `KEA_LEASE6_CLIENT_DUID`
- `KEA_LEASE6_HWADDR`
- `KEA_LEASE6_HOSTNAME`
- `KEA_LEASE6_STATE`
- `KEA_LEASE6_IS_EXPIRED`
- `KEA_LEASE6_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE6_VALID_LIFETIME`
- `KEA_LEASE6_PREFERRED_LIFETIME`
- `KEA_LEASE6_IAID`
- `KEA_REMOVE_LEASE`

### [`lease6_recover`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Recover)

- `KEA_LEASE6_TYPE`
- `KEA_LEASE6_ADDRESS`
- `KEA_LEASE6_DELEGATED_PREFIX`
- `KEA_LEASE6_DELEGATED_PREFIXLEN`
- `KEA_LEASE6_CLIENT_DUID`
- `KEA_LEASE6_HWADDR`
- `KEA_LEASE6_HOSTNAME`
- `KEA_LEASE6_STATE`
- `KEA_LEASE6_IS_EXPIRED`
- `KEA_LEASE6_CLIENT_LAST_TRANSMISSION`
- `KEA_LEASE6_VALID_LIFETIME`
- `KEA_LEASE6_PREFERRED_LIFETIME`
- `KEA_LEASE6_IAID`


# TODO

- take stdout/stderr of script and turn it into proper Kea logs
- agree on a consistent terminology:
  - should a "prefix" variable contain the prefixlen (2001:db8::/48) or just the base address (2001:db8::)?
- also call the script at load/unload
- figure out how to call several scripts (loading the hook multiple times doesn't seem to work)
- allow to configure which hook points will trigger the script
- take into account the return code of the script to set the status
  of the callout (this should be configurable to avoid surprises...).

Some bugs to investigate/fix in Kea:

- `lease6_select` is called twice (once with `IA_NA` and once with `IA_PD`), but
  other functions (`lease6_renew`, `lease6_release`, `lease6_expire`) are only
  called with `IA_PD`.
- when an address reservation is changed for a given client, `lease6_expire` is never
  called for the old address.
