#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/dhcp6.h>
#include <dhcp/pkt6.h>
#include <dhcp/option6_ia.h>
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
void extract_bool(std::vector<std::string>& env, const std::string variable, bool value)
{
    env.push_back(variable + "=" + std::string(value ? "1" : "0"));
}

void extract_query4(std::vector<std::string>& env, const Pkt4Ptr query)
{
    /* General information */
    env.push_back("KEA_QUERY4_TYPE=" + std::string(query->getName()));
    env.push_back("KEA_QUERY4_INTERFACE=" + query->getIface());
    /* Hardware address */
    HWAddrPtr hwaddr = query->getHWAddr();
    env.push_back("KEA_QUERY4_HWADDR_TYPE=" + std::to_string(hwaddr->htype_));
    env.push_back("KEA_QUERY4_HWADDR_SOURCE=" + std::to_string(hwaddr->source_));
    env.push_back("KEA_QUERY4_HWADDR=" + hwaddr->toText(false));
    /* Misc */
    env.push_back("KEA_QUERY4_RELAYED=" + std::to_string(query->isRelayed()));
}

void extract_query6(std::vector<std::string>& env, const Pkt6Ptr query)
{
    /* General information */
    env.push_back("KEA_QUERY6_TYPE=" + std::string(query->getName()));
    env.push_back("KEA_QUERY6_INTERFACE=" + query->getIface());
    /* TODO */
    env.push_back("KEA_QUERY6_DUID=");
    env.push_back("KEA_QUERY6_HWADDR=");
    /* TODO: all options?  Only common ones?  Which format? */
    /* TODO */
    env.push_back("KEA_QUERY6_TEXT=" + query->toText());
}

void extract_response4(std::vector<std::string>& env, const Pkt4Ptr response)
{
    /* General information */
    env.push_back("KEA_RESPONSE4_TYPE=" + std::string(response->getName()));
    env.push_back("KEA_RESPONSE4_INTERFACE=" + response->getIface());
}

void extract_response6(std::vector<std::string>& env, const Pkt6Ptr response)
{
    /* General information */
    env.push_back("KEA_RESPONSE6_TYPE=" + std::string(response->getName()));
    env.push_back("KEA_RESPONSE6_INTERFACE=" + response->getIface());
    /* TODO, this may not always exist in the response */
    env.push_back("KEA_RESPONSE6_ADDRESS=");
    env.push_back("KEA_RESPONSE6_PREFERRED_LIFETIME=");
    env.push_back("KEA_RESPONSE6_VALID_LIFETIME=");
    /* TODO */
    env.push_back("KEA_RESPONSE6_TEXT=" + response->toText());
}

void extract_subnet4(std::vector<std::string>& env, const Subnet4Ptr subnet)
{
    env.push_back("KEA_SUBNET4=" + subnet->toText());
    std::pair<isc::asiolink::IOAddress, uint8_t> prefix = subnet->get();
    env.push_back("KEA_SUBNET4_PREFIX=" + prefix.first.toText());
    env.push_back("KEA_SUBNET4_PREFIXLEN=" + std::to_string(prefix.second));
}

void extract_subnet6(std::vector<std::string>& env, const Subnet6Ptr subnet)
{
    env.push_back("KEA_SUBNET6=" + subnet->toText());
    std::pair<isc::asiolink::IOAddress, uint8_t> prefix = subnet->get();
    env.push_back("KEA_SUBNET6_PREFIX=" + prefix.first.toText());
    env.push_back("KEA_SUBNET6_PREFIXLEN=" + std::to_string(prefix.second));
}

void extract_lease4(std::vector<std::string>& env, const Lease4Ptr lease)
{
    env.push_back("KEA_LEASE4_ADDRESS=" + lease->addr_.toText());
}

void extract_lease6(std::vector<std::string>& env, const Lease6Ptr lease)
{
    env.push_back("KEA_LEASE6_ADDRESS=" + lease->addr_.toText());
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
    extract_bool(env, "KEA_FAKE_ALLOCATION", fake_allocation);
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

int lease4_expire(CalloutHandle& handle) {
    std::vector<std::string> env;
    Lease4Ptr lease;
    bool remove_lease;
    handle.getArgument("lease4", lease);
    extract_lease4(env, lease);
    handle.getArgument("remove_lease", remove_lease);
    extract_bool(env, "KEA_REMOVE_LEASE", remove_lease);
    /* Run script */
    int ret;
    ret = run_script("lease4_expire", env);
    return 0;
}

int lease4_recover(CalloutHandle& handle) {
    std::vector<std::string> env;
    Lease4Ptr lease;
    handle.getArgument("lease4", lease);
    extract_lease4(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease4_recover", env);
    return 0;
}

/* IPv6 callouts */
int pkt6_receive(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt6Ptr query;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    /* Run script */
    int ret;
    ret = run_script("pkt6_receive", env);
    return 0;
}

int pkt6_send(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt6Ptr query, response;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("response6", response);
    extract_response6(env, response);
    /* Run script */
    int ret;
    ret = run_script("pkt6_send", env);
    return 0;
}

int subnet6_select(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt6Ptr query;
    Subnet6Ptr subnet;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("subnet6", subnet);
    extract_subnet6(env, subnet);
    /* Run script */
    int ret;
    ret = run_script("subnet6_select", env);
    return 0;
}

int lease6_select(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt6Ptr query;
    Subnet6Ptr subnet;
    bool fake_allocation;
    Lease6Ptr lease;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("subnet6", subnet);
    extract_subnet6(env, subnet);
    handle.getArgument("fake_allocation", fake_allocation);
    extract_bool(env, "KEA_FAKE_ALLOCATION", fake_allocation);
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease6_select", env);
    return 0;
}

int lease6_renew(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt6Ptr query;
    Lease6Ptr lease;
    boost::shared_ptr<Option6IA> ia_na;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    handle.getArgument("ia_na", ia_na);
    /* TODO */
    /* Run script */
    int ret;
    ret = run_script("lease6_renew", env);
    return 0;
}

int lease6_rebind(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt6Ptr query;
    Lease6Ptr lease;
    boost::shared_ptr<Option6IA> ia_na;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    handle.getArgument("ia_na", ia_na);
    /* TODO */
    /* Run script */
    int ret;
    ret = run_script("lease6_rebind", env);
    return 0;
}

int lease6_decline(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt6Ptr query;
    Lease6Ptr lease;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease6_decline", env);
    return 0;
}

int lease6_release(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt6Ptr query;
    Lease6Ptr lease;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease6_release", env);
    return 0;
}

int lease6_expire(CalloutHandle& handle) {
    std::vector<std::string> env;
    Lease6Ptr lease;
    bool remove_lease;
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    handle.getArgument("remove_lease", remove_lease);
    extract_bool(env, "KEA_REMOVE_LEASE", remove_lease);
    /* Run script */
    int ret;
    ret = run_script("lease6_expire", env);
    return 0;
}

int lease6_recover(CalloutHandle& handle) {
    std::vector<std::string> env;
    Lease6Ptr lease;
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    /* Run script */
    int ret;
    ret = run_script("lease6_recover", env);
    return 0;
}

} // end extern "C"
