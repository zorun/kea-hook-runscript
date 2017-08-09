
## TODO

- handle DHCPv6 hook points
- ask for a new `lease_expired` hook point
- document environment variables
- change some variables from integer to their textual representation
  (e.g. `QUERY4_TYPE=3` → `QUERY4_TYPE=DHCPREQUEST`)
- allow to configure which hook points will trigger the script
- take into account the return code of the script to set the status
  of the callout.
