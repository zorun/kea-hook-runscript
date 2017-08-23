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
  with DHCPv6-PD;
- update firewall rules to allow/refuse access to new DHCP clients;
- log information about successful leases.

For more complex use-cases, including non-trivial changes to Kea's behaviour,
it may be easier to just write a Kea hook yourself.

## How to build

You first need to compile the hook.  For this, you need Kea and Boost
development headers installed: on Debian, the packages are `kea-dev` and
`libboost-dev`.

To build, simply run:

    $ make -j

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
    $ make -j

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
environment variables.

Refer to the Kea documentation for more information about each hook point:

- DHCPv4 hooks reference: <https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html>
- DHCPv6 hooks reference: <https://jenkins.isc.org/job/Kea_doc/doxygen/d1/d02/dhcpv6Hooks.html>

## Debug script

To experiment, an example script is provided: `examples/debug.sh`.  It simply prints
the name of the hook point and all environment variables passed to it.

The output of the script is at `/tmp/kea-hook-runscript-debug.log`.  A nice way to debug
is to continously display the content of this file:

    tail -F /tmp/kea-hook-runscript-debug.log

# Reference of variables passed to the external script

## DHCPv4 variables

Here are all possible variables, with their type, description and
reference of the possible values.  Booleans are simply expressed with `0`
and `1`.

| Variable name          | Type     | Description                                                 | Reference                                                                                                                                                       |
|------------------------|----------|-------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `QUERY4_TYPE`          | `string` | Type of DHCP message                                        | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#aa5bfdcc4861aa3dab5328dba89362016)                                  |
| `QUERY4_INTERFACE`     | `string` | Interface on which query was received                       |                                                                                                                                                                 |
| `QUERY4_RELAYED`       | `bool`   | Whether query was relayed                                   | [dhcp/pkt4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/de/d13/classisc_1_1dhcp_1_1Pkt4.html#a8468401827b9bacdd3796bb4e20d8e5e)                               |
| `QUERY4_HWADDR`        | `string` | Hardware address of the client (often MAC address)          |                                                                                                                                                                 |
| `QUERY4_HWADDR_TYPE`   | `int`    | Type of hardware address                                    | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#addcff933049489d800f9869196c8e46fa96a62c59182d6e06780b0e1ef40da059) |
| `QUERY4_HWADDR_SOURCE` | `int`    | Currently always set to `0`                                 | [dhcp/hwaddr.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d2/db9/structisc_1_1dhcp_1_1HWAddr.html)                                                            |
| `RESPONSE4_TYPE`       | `string` | Type of DHCP message                                        | [dhcp/dhcp4.h](https://jenkins.isc.org/job/Kea_doc/doxygen/d5/d8c/namespaceisc_1_1dhcp.html#aa5bfdcc4861aa3dab5328dba89362016)                                  |
| `RESPONSE4_INTERFACE`  | `string` | Interface on which response is being sent                   |                                                                                                                                                                 |
| `SUBNET4_PREFIX`       | `IPv4`   | Address of the IP prefix (without prefix length)            |                                                                                                                                                                 |
| `SUBNET4_PREFIXLEN`    | `int`    | Prefix length (`0` to `32`)                                 |                                                                                                                                                                 |
| `SUBNET4`              | `string` | `SUBNET4_PREFIX`/`SUBNET4_PREFIXLEN`                        |                                                                                                                                                                 |
| `LEASE4_ADDRESS`       | `IPv4`   | IPv4 address leased to client                               |                                                                                                                                                                 |
| `REMOVE_LEASE`         | `bool`   | Whether the lease should be removed from the lease database | [DHCPv4 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLease4Expire)                                                  |
| `FAKE_ALLOCATION`      | `bool`   | Whether the query is a DISCOVER or a REQUEST                | [DHCPv4 hook API](https://jenkins.isc.org/job/Kea_doc/doxygen/de/df3/dhcpv4Hooks.html#dhcpv4HooksLeaseSelect)                                                   |

## DHCPv4 hook points

For each Kea hook point, here are all variables usable in the external
script.

### `pkt4_receive`

- `QUERY4_TYPE`
- `QUERY4_INTERFACE`
- `QUERY4_HWADDR`
- `QUERY4_HWADDR_SOURCE`
- `QUERY4_HWADDR_TYPE`
- `QUERY4_RELAYED`

### `pkt4_send`

- `QUERY4_TYPE`
- `QUERY4_INTERFACE`
- `QUERY4_HWADDR`
- `QUERY4_HWADDR_SOURCE`
- `QUERY4_HWADDR_TYPE`
- `QUERY4_RELAYED`
- `RESPONSE4_TYPE`
- `RESPONSE4_INTERFACE`

### `subnet4_select`

- `QUERY4_TYPE`
- `QUERY4_INTERFACE`
- `QUERY4_HWADDR`
- `QUERY4_HWADDR_SOURCE`
- `QUERY4_HWADDR_TYPE`
- `QUERY4_RELAYED`
- `SUBNET4_PREFIX`
- `SUBNET4_PREFIXLEN`
- `SUBNET4`

### `lease4_select`

- `QUERY4_TYPE`
- `QUERY4_INTERFACE`
- `QUERY4_HWADDR`
- `QUERY4_HWADDR_SOURCE`
- `QUERY4_HWADDR_TYPE`
- `QUERY4_RELAYED`
- `SUBNET4_PREFIX`
- `SUBNET4_PREFIXLEN`
- `SUBNET4`
- `FAKE_ALLOCATION`
- `LEASE4_ADDRESS`

### `lease4_renew`

- `QUERY4_TYPE`
- `QUERY4_INTERFACE`
- `QUERY4_HWADDR`
- `QUERY4_HWADDR_SOURCE`
- `QUERY4_HWADDR_TYPE`
- `QUERY4_RELAYED`
- `SUBNET4_PREFIX`
- `SUBNET4_PREFIXLEN`
- `SUBNET4`
- `LEASE4_ADDRESS`

### `lease4_release`

- `QUERY4_TYPE`
- `QUERY4_INTERFACE`
- `QUERY4_HWADDR`
- `QUERY4_HWADDR_SOURCE`
- `QUERY4_HWADDR_TYPE`
- `QUERY4_RELAYED`
- `LEASE4_ADDRESS`

### `lease4_decline`

- `QUERY4_TYPE`
- `QUERY4_INTERFACE`
- `QUERY4_HWADDR`
- `QUERY4_HWADDR_SOURCE`
- `QUERY4_HWADDR_TYPE`
- `QUERY4_RELAYED`
- `LEASE4_ADDRESS`

### `lease4_expire`

- `LEASE4_ADDRESS`
- `REMOVE_LEASE`

### `lease4_recover`

- `LEASE4_ADDRESS`


# TODO

- handle DHCPv6 hook points better
- also call the script at load/unload
- allow to configure which hook points will trigger the script
- take into account the return code of the script to set the status
  of the callout (this should be configurable to avoid surprises...).
