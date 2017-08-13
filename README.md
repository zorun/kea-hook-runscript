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

This hook works for both DHCPv4 and DHCPv6.

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

## How to use this hook

Here is how to tell Kea to use this hook, for DHCPv4:

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

## Information passed to the external script

TODO: table with environment variables passed to the script at each hook point.

For now, just use the debug script or read the code.

# TODO

- handle DHCPv6 hook points better
- also call the script at load/unload
- document environment variables
- allow to configure which hook points will trigger the script
- take into account the return code of the script to set the status
  of the callout (this should be configurable to avoid surprises...).
