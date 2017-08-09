#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/dhcp6.h>
#include <dhcp/pkt6.h>
#include <dhcpsrv/subnet.h>
#include <dhcpsrv/lease.h>

#include <string>
#include <vector>

#include "runscript.h"

using namespace isc::dhcp;
using namespace isc::hooks;

extern "C" {

/* These are helpers that extract relevant information from Kea data
 * structures and store them in environment variables. */
void extract_query4(std::vector<std::string>& env, const Pkt4Ptr query)
{
    /* General information */
    env.push_back("DHCP4_TYPE=" + std::to_string(query->getType()));
    /* Hardware address */
    HWAddrPtr hwaddr = query->getHWAddr();
    env.push_back("HWADDR_TYPE=" + std::to_string(hwaddr->htype_));
    env.push_back("HWADDR_SOURCE=" + std::to_string(hwaddr->source_));
    env.push_back("HWADDR=" + hwaddr->toText(false));
    /* Misc */
    env.push_back("DHCP4_RELAYED=" + std::to_string(query->isRelayed()));
}

/* IPv4 callouts */
int pkt4_receive(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt4Ptr query;
    handle.getArgument("query4", query);
    extract_query4(env, query);
    /* Run script */
    int ret;
    ret = run_script("pkt4_receive", env);
    return 0;
}

int lease4_select(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt4Ptr query;
    Subnet4Ptr subnet;
    bool fake_allocation;
    Lease4Ptr lease;
    handle.getArgument("query4", query);
    extract_query4(env, query);
    handle.getArgument("subnet4", subnet);
    handle.getArgument("fake_allocation", fake_allocation);
    handle.getArgument("lease4", lease);
    /* Run script */
    int ret;
    ret = run_script("lease4_select", env);
    return 0;
}

/* IPv6 callouts */

} // end extern "C"
