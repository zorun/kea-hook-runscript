# About kea-hook-runscript

This a hook for the Kea DHCP server that allows to run an external script
at various points in the processing of DHCP requests and responses.

The goal is to simplify integration with Kea: for many simple use-cases,
it is overkill to have to write a full-blown Kea hook, where a simple shell script
can do the job.

## What it can and can't do

Integration is mostly done one-way: thanks to this hook, Kea passes information
to the external script, but the script cannot easily modify Kea's behaviour.

The **external script** can be any kind of executable program, but often it will
be a simple script (shell, Perl, Python...).  Information about what Kea is doing
is provided to the external script through environment variables: MAC address of
the requesting DHCP client, IP address being handed out, etc.

Each time Kea encounters a hook point, it will call the script **synchronously**.
That is, Kea will do absolutely nothing else while the script is running.
Thus, it is a good idea to perform only lightweight processing in the script,
and absolutely avoid blocking operations.  Also, scripting languages that need
to initialise a huge interpreter (such as Python or Ruby) will cause a large
amount of CPU usage and a massive slowdown of Kea, because the script is run
multiple times for each DHCP transaction.

This hook works for both DHCPv4 and DHCPv6, on Kea 1.1 and above.

In the future, the hook will possibly feed the return code of the external script
back into Kea.  This would allow the external script to cancel part of Kea's normal processing
(for instance, it could be possible to easily implement a flexible host blacklist this way).

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

## How to build

You first need to compile the hook.  For this, you need Kea and Boost
development headers installed: on Debian, the packages are `kea-dev` and
`libboost-dev`.

To build, simply run:

    $ make

Some notes on Kea versions:

- Kea 1.1 does not install all required headers (most notably `dhcpsrv/`),
  so you may need to build against Kea's source tree.
- Kea 1.2 is missing a header file by mistake, so depending on your
  distribution, you may need to manually copy `option6_pdexclude.h` from
  the Kea git repository to `/usr/include/kea/dhcp/`.
- Kea 1.3 should work out-of-the-box.

To build against a local Kea source tree, assumed to be in `~/kea`:

- build Kea (`cd ~/kea && make -j`)
- install Kea to a local directory (`cd ~/kea && make install DESTDIR=/tmp/kea`)

Then build this hook with:

    $ export KEA_MSG_COMPILER=~/kea/src/lib/log/compiler/kea-msg-compiler
    $ export KEA_INCLUDE=~/kea/src/lib
    $ export KEA_LIB=/tmp/kea/usr/local/lib
    $ make

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
            "script": "/path/to/myscript.sh"
          }
        }
      ],
      ...
    }
    }

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

| Variable name              | Type     | Description                                                 | Reference                                                                                                                                                       |
|----------------------------|----------|-------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `KEA_QUERY4_TYPE`          | `string` | Type of DHCP message                                        | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#aa5bfdcc4861aa3dab5328dba89362016)                                  |
| `KEA_QUERY4_INTERFACE`     | `string` | Interface on which query was received                       |                                                                                                                                                                 |
| `KEA_QUERY4_RELAYED`       | `bool`   | Whether query was relayed                                   | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html#a8468401827b9bacdd3796bb4e20d8e5e)                               |
| `KEA_QUERY4_HWADDR`        | `string` | Hardware address of the client (often MAC address)          |                                                                                                                                                                 |
| `KEA_QUERY4_HWADDR_TYPE`   | `int`    | Type of hardware address                                    | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#addcff933049489d800f9869196c8e46fa96a62c59182d6e06780b0e1ef40da059) |
| `KEA_QUERY4_HWADDR_SOURCE` | `int`    | Currently always set to `0`                                 | [dhcp/hwaddr.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d2/db9/structisc_1_1dhcp_1_1HWAddr.html)                                                            |
| `KEA_RESPONSE4_TYPE`       | `string` | Type of DHCP message                                        | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#aa5bfdcc4861aa3dab5328dba89362016)                                  |
| `KEA_RESPONSE4_INTERFACE`  | `string` | Interface on which response is being sent                   |                                                                                                                                                                 |
| `KEA_SUBNET4_PREFIX`       | `IPv4`   | IP prefix of the subnet (without prefix length)             |                                                                                                                                                                 |
| `KEA_SUBNET4_PREFIXLEN`    | `int`    | Prefix length of the subnet (`0` to `32`)                   |                                                                                                                                                                 |
| `KEA_SUBNET4`              | `string` | `KEA_SUBNET4_PREFIX`/`KEA_SUBNET4_PREFIXLEN`                |                                                                                                                                                                 |
| `KEA_LEASE4_ADDRESS`       | `IPv4`   | IPv4 address leased to client                               |                                                                                                                                                                 |
| `KEA_REMOVE_LEASE`         | `bool`   | Whether the lease should be removed from the lease database | [DHCPv4 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Expire)                                                  |
| `KEA_FAKE_ALLOCATION`      | `bool`   | Whether the query is a DISCOVER or a REQUEST                | [DHCPv4 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLeaseSelect)                                                   |

## DHCPv4 hook points

For each Kea hook point, here are all variables usable in the external
script.

### [`pkt4_receive`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksPkt4Receive)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`

### [`pkt4_send`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksPkt4Send)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_RESPONSE4_TYPE`
- `KEA_RESPONSE4_INTERFACE`

### [`subnet4_select`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksSubnet4Select)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_SUBNET4_PREFIX`
- `KEA_SUBNET4_PREFIXLEN`
- `KEA_SUBNET4`

### [`lease4_select`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Select)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_SUBNET4_PREFIX`
- `KEA_SUBNET4_PREFIXLEN`
- `KEA_SUBNET4`
- `KEA_FAKE_ALLOCATION`
- `KEA_LEASE4_ADDRESS`

### [`lease4_renew`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Renew)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_SUBNET4_PREFIX`
- `KEA_SUBNET4_PREFIXLEN`
- `KEA_SUBNET4`
- `KEA_LEASE4_ADDRESS`

### [`lease4_release`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Release)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_LEASE4_ADDRESS`

### [`lease4_decline`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Decline)

- `KEA_QUERY4_TYPE`
- `KEA_QUERY4_INTERFACE`
- `KEA_QUERY4_HWADDR`
- `KEA_QUERY4_HWADDR_SOURCE`
- `KEA_QUERY4_HWADDR_TYPE`
- `KEA_QUERY4_RELAYED`
- `KEA_LEASE4_ADDRESS`

### [`lease4_expire`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Expire)

- `KEA_LEASE4_ADDRESS`
- `KEA_REMOVE_LEASE`

### [`lease4_recover`](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Recover)

- `KEA_LEASE4_ADDRESS`


## DHCPv6 variables

Here are all possible variables for DHCPv6, with their type, description
and reference of the possible values.  Booleans are simply expressed with
`0` and `1`.

| Variable name                      | Type     | Description                                                 | Reference                                                                                                      |
|------------------------------------|----------|-------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------|
| `KEA_QUERY6_TYPE`                  | `string` | Type of DHCPv6 message                                      | [dhcp/dhcp6.h](https://jenkins.isc.org/job/Kea_doc/doxygen/db/d87/dhcp6_8h_source.html)                        |
| `KEA_QUERY6_INTERFACE`             | `string` | Interface on which query was received                       |                                                                                                                |
| `KEA_QUERY6_DUID`                  | `string` | TODO                                                        |                                                                                                                |
| `KEA_QUERY6_HWADDR`                | `string` | TODO                                                        |                                                                                                                |
| `KEA_RESPONSE6_TYPE`               | `string` | Type of DHCPv6 message                                      | [dhcp/dhcp6.h](https://jenkins.isc.org/job/Kea_doc/doxygen/db/d87/dhcp6_8h_source.html)                        |
| `KEA_RESPONSE6_INTERFACE`          | `string` | Interface on which response is being sent                   |                                                                                                                |
| `KEA_RESPONSE6_ADDRESS`            | `string` | TODO                                                        |                                                                                                                |
| `KEA_RESPONSE6_PREFERRED_LIFETIME` | `int`    | TODO                                                        |                                                                                                                |
| `KEA_RESPONSE6_VALID_LIFETIME`     | `int`    | TODO                                                        |                                                                                                                |
| `KEA_SUBNET6_PREFIX`               | `IPv6`   | IP prefix of the subnet (without prefix length)             |                                                                                                                |
| `KEA_SUBNET6_PREFIXLEN`            | `int`    | Prefix length of the subnet (`0` to `128`)                  |                                                                                                                |
| `KEA_SUBNET6`                      | `string` | `KEA_SUBNET6_PREFIX`/`KEA_SUBNET6_PREFIXLEN`                |                                                                                                                |
| `KEA_LEASE6_ADDRESS`               | `IPv6`   | IPv6 address leased to client                               |                                                                                                                |
| `KEA_REMOVE_LEASE`                 | `bool`   | Whether the lease should be removed from the lease database | [DHCPv6 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Expire) |
| `KEA_FAKE_ALLOCATION`              | `bool`   | Whether the query is a SOLICIT or a REQUEST                 | [DHCPv6 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Select) |

## DHCPv6 hook points

For each Kea hook point, here are all variables usable in the external
script.

### [`pkt6_receive`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksPkt6Receive)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`

### [`pkt6_send`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksPkt6Send)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_RESPONSE6_TYPE`
- `KEA_RESPONSE6_INTERFACE`
- `KEA_RESPONSE6_ADDRESS`
- `KEA_RESPONSE6_PREFERRED_LIFETIME`
- `KEA_RESPONSE6_VALID_LIFETIME`

### [`subnet6_select`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksSubnet6Select)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_SUBNET6_PREFIX`
- `KEA_SUBNET6_PREFIXLEN`
- `KEA_SUBNET6`

### [`lease6_select`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Select)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_SUBNET6_PREFIX`
- `KEA_SUBNET6_PREFIXLEN`
- `KEA_SUBNET6`
- `KEA_FAKE_ALLOCATION`
- `KEA_LEASE6_ADDRESS`

### [`lease6_renew`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Renew)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_LEASE6_ADDRESS`

### [`lease6_rebind`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Rebind)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_LEASE6_ADDRESS`

### [`lease6_decline`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Decline)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_LEASE6_ADDRESS`

### [`lease6_release`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Release)

- `KEA_QUERY6_TYPE`
- `KEA_QUERY6_INTERFACE`
- `KEA_QUERY6_DUID`
- `KEA_QUERY6_HWADDR`
- `KEA_LEASE6_ADDRESS`

### [`lease6_expire`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Expire)

- `KEA_LEASE6_ADDRESS`
- `KEA_REMOVE_LEASE`

### [`lease6_recover`](https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html#dhcpv6HooksLease6Recover)

- `KEA_LEASE6_ADDRESS`


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
