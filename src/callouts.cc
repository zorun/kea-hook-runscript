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
    env.push_back("QUERY4_TYPE=" + std::string(query->getName()));
    /* Hardware address */
    HWAddrPtr hwaddr = query->getHWAddr();
    env.push_back("QUERY4_HWADDR_TYPE=" + std::to_string(hwaddr->htype_));
    env.push_back("QUERY4_HWADDR_SOURCE=" + std::to_string(hwaddr->source_));
    env.push_back("QUERY4_HWADDR=" + hwaddr->toText(false));
    /* Misc */
    env.push_back("QUERY4_RELAYED=" + std::to_string(query->isRelayed()));
}

void extract_response4(std::vector<std::string>& env, const Pkt4Ptr response)
{
    /* General information */
    env.push_back("RESPONSE4_TYPE=" + std::string(response->getName()));
}

void extract_subnet4(std::vector<std::string>& env, const Subnet4Ptr subnet)
{
    env.push_back("SUBNET4=" + subnet->toText());
    std::pair<isc::asiolink::IOAddress, uint8_t> prefix = subnet->get();
    env.push_back("SUBNET4_PREFIX=" + prefix.first.toText());
    env.push_back("SUBNET4_PREFIXLEN=" + std::to_string(prefix.second));
}

void extract_lease4(std::vector<std::string>& env, const Lease4Ptr lease)
{
    env.push_back("LEASE4_ADDRESS=" + lease->addr_.toText());
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

int subnet4_select(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt4Ptr query;
    Subnet4Ptr subnet;
    handle.getArgument("query4", query);
    extract_query4(env, query);
    handle.getArgument("subnet4", subnet);
    extract_subnet4(env, subnet);
    /* Run script */
    int ret;
    ret = run_script("subnet4_select", env);
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
    extract_subnet4(env, subnet);
    handle.getArgument("fake_allocation", fake_allocation);
    env.push_back("FAKE_ALLOCATION=" + fake_allocation ? "1" : "0");
    handle.getArgument("lease4", lease);
    extract_lease4(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease4_select", env);
    return 0;
}

int lease4_renew(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt4Ptr query;
    Subnet4Ptr subnet;
    Lease4Ptr lease;
    handle.getArgument("query4", query);
    extract_query4(env, query);
    handle.getArgument("subnet4", subnet);
    extract_subnet4(env, subnet);
    /* TODO: what is this?  Is it different from what is in the query? */
    //handle.getArgument("clientid", XX);
    //handle.getArgument("hwaddr", XX);
    handle.getArgument("lease4", lease);
    extract_lease4(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease4_renew", env);
    return 0;
}

int lease4_release(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt4Ptr query;
    Lease4Ptr lease;
    handle.getArgument("query4", query);
    extract_query4(env, query);
    handle.getArgument("lease4", lease);
    extract_lease4(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease4_release", env);
    return 0;
}

int lease4_decline(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt4Ptr query;
    Lease4Ptr lease;
    handle.getArgument("query4", query);
    extract_query4(env, query);
    handle.getArgument("lease4", lease);
    extract_lease4(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease4_decline", env);
    return 0;
}

int pkt4_send(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt4Ptr response;
    Pkt4Ptr query;
    handle.getArgument("response4", response);
    extract_response4(env, response);
    handle.getArgument("query4", query);
    extract_query4(env, query);
    /* Run script */
    int ret;
    ret = run_script("pkt4_send", env);
    return 0;
}

/* IPv6 callouts */

} // end extern "C"
