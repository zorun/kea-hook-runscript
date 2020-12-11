/* Copyright (c) 2017-2019 by Baptiste Jonglez
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

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

/* Extract information from a DHCPv4 packet (query received, or response
 * about to be sent) */
void extract_pkt4(std::vector<std::string>& env, const std::string envprefix, const Pkt4Ptr pkt4)
{
    /* General information */
    env.push_back(envprefix + "TYPE=" + std::string(pkt4->getName()));
    env.push_back(envprefix + "INTERFACE=" + pkt4->getIface());
    env.push_back(envprefix + "IFINDEX=" + std::to_string(pkt4->getIndex()));
    /* Hardware address */
    HWAddrPtr hwaddr = pkt4->getHWAddr();
    if (hwaddr) {
        env.push_back(envprefix + "HWADDR=" + hwaddr->toText(false));
        env.push_back(envprefix + "HWADDR_TYPE=" + std::to_string(hwaddr->htype_));
        env.push_back(envprefix + "HWADDR_SOURCE=" + std::to_string(hwaddr->source_));
    } else {
        env.push_back(envprefix + "HWADDR=");
        env.push_back(envprefix + "HWADDR_TYPE=");
        env.push_back(envprefix + "HWADDR_SOURCE=");
    }
    /* Misc */
    env.push_back(envprefix + "CIADDR=" + pkt4->getCiaddr().toText());
    env.push_back(envprefix + "SIADDR=" + pkt4->getSiaddr().toText());
    env.push_back(envprefix + "YIADDR=" + pkt4->getYiaddr().toText());
    env.push_back(envprefix + "GIADDR=" + pkt4->getGiaddr().toText());
    env.push_back(envprefix + "RELAYED=" + std::to_string(pkt4->isRelayed()));
    env.push_back(envprefix + "RELAY_HOPS=" + std::to_string(pkt4->getHops()));

    /* Specific Options */
    OptionPtr option60 = pkt4->getOption(60);
    if (option60) {
        env.push_back(envprefix + "OPTION60=" + option60->toString());
    }

    OptionPtr rai = pkt4->getOption(82);
    if (rai) {
        env.push_back(envprefix + "RAI=" + rai->toHexString());

        OptionPtr circuit_id = rai->getOption(RAI_OPTION_AGENT_CIRCUIT_ID);
        if (circuit_id) {
            env.push_back(envprefix + "RAI_CIRCUIT_ID=" + circuit_id->toHexString());
        }

        OptionPtr remote_id = rai->getOption(RAI_OPTION_REMOTE_ID);
        if (remote_id) {
            env.push_back(envprefix + "RAI_REMOTE_ID=" + remote_id->toHexString());
        }
    }
}

void extract_query4(std::vector<std::string>& env, const Pkt4Ptr query)
{
    extract_pkt4(env, "KEA_QUERY4_", query);
}

void extract_response4(std::vector<std::string>& env, const Pkt4Ptr response)
{
    extract_pkt4(env, "KEA_RESPONSE4_", response);
}

/* Extract information from a DHCPv6 packet (query received, or response
 * about to be sent) */
void extract_pkt6(std::vector<std::string>& env, const std::string envprefix, const Pkt6Ptr pkt6)
{
    /* General information */
    env.push_back(envprefix + "TYPE=" + std::string(pkt6->getName()));
    env.push_back(envprefix + "INTERFACE=" + pkt6->getIface());
    env.push_back(envprefix + "IFINDEX=" + std::to_string(pkt6->getIndex()));
    HWAddrPtr hwaddr = pkt6->getMAC(HWAddr::HWADDR_SOURCE_ANY);
    if (hwaddr) {
        env.push_back(envprefix + "HWADDR=" + hwaddr->toText(false));
        env.push_back(envprefix + "HWADDR_TYPE=" + std::to_string(hwaddr->htype_));
        env.push_back(envprefix + "HWADDR_SOURCE=" + std::to_string(hwaddr->source_));
    } else {
        env.push_back(envprefix + "HWADDR=");
        env.push_back(envprefix + "HWADDR_TYPE=");
        env.push_back(envprefix + "HWADDR_SOURCE=");
    }
    env.push_back(envprefix + "LOCAL_ADDRESS=" + pkt6->getLocalAddr().toText());
    env.push_back(envprefix + "LOCAL_PORT=" + std::to_string(pkt6->getLocalPort()));
    env.push_back(envprefix + "REMOTE_ADDRESS=" + pkt6->getRemoteAddr().toText());
    env.push_back(envprefix + "REMOTE_PORT=" + std::to_string(pkt6->getRemotePort()));
    env.push_back(envprefix + "LABEL=" + pkt6->getLabel());
    env.push_back(envprefix + "TRANSACTION_ID=" + std::to_string(pkt6->getTransid()));
    /* TODO */
    env.push_back(envprefix + "DUID=");
    /* TODO: all options?  Only common ones?  Which format? */
    /* TODO */
    env.push_back(envprefix + "DEBUG=" + pkt6->toText());
}

void extract_query6(std::vector<std::string>& env, const Pkt6Ptr query)
{
    extract_pkt6(env, "KEA_QUERY6_", query);
}


void extract_response6(std::vector<std::string>& env, const Pkt6Ptr response)
{
    extract_pkt6(env, "KEA_RESPONSE6_", response);
}

void extract_subnet4(std::vector<std::string>& env, const Subnet4Ptr subnet)
{
    /* The subnet given by Kea might be NULL, this seems to happen when
     * Kea fails to find a matching subnet for a client request. */
    if (subnet != NULL) {
        env.push_back("KEA_SUBNET4=" + subnet->toText());
        std::pair<isc::asiolink::IOAddress, uint8_t> prefix = subnet->get();
        env.push_back("KEA_SUBNET4_PREFIX=" + prefix.first.toText());
        env.push_back("KEA_SUBNET4_PREFIXLEN=" + std::to_string(prefix.second));
    } else {
        env.push_back("KEA_SUBNET4=");
        env.push_back("KEA_SUBNET4_PREFIX=");
        env.push_back("KEA_SUBNET4_PREFIXLEN=");
    }
}

void extract_subnet6(std::vector<std::string>& env, const Subnet6Ptr subnet)
{
    if (subnet != NULL) {
        env.push_back("KEA_SUBNET6=" + subnet->toText());
        std::pair<isc::asiolink::IOAddress, uint8_t> prefix = subnet->get();
        env.push_back("KEA_SUBNET6_PREFIX=" + prefix.first.toText());
        env.push_back("KEA_SUBNET6_PREFIXLEN=" + std::to_string(prefix.second));
    } else {
        env.push_back("KEA_SUBNET6=");
        env.push_back("KEA_SUBNET6_PREFIX=");
        env.push_back("KEA_SUBNET6_PREFIXLEN=");
    }
}

void extract_lease4(std::vector<std::string>& env, const Lease4Ptr lease)
{
    env.push_back("KEA_LEASE4_TYPE=V4");
    env.push_back("KEA_LEASE4_STATE=" + lease->basicStatesToText(lease->state_));
    extract_bool(env, "KEA_LEASE4_IS_EXPIRED", lease->expired());
    env.push_back("KEA_LEASE4_ADDRESS=" + lease->addr_.toText());
    if (lease->hwaddr_) {
        env.push_back("KEA_LEASE4_HWADDR=" + lease->hwaddr_->toText(false));
    } else {
        env.push_back("KEA_LEASE4_HWADDR=");
    }
    env.push_back("KEA_LEASE4_HOSTNAME=" + lease->hostname_);
    env.push_back("KEA_LEASE4_CLIENT_LAST_TRANSMISSION=" + std::to_string(lease->cltt_));
    env.push_back("KEA_LEASE4_VALID_LIFETIME=" + std::to_string(lease->valid_lft_));
    env.push_back("KEA_LEASE4_DEBUG=" + lease->toText());
}

void extract_lease6(std::vector<std::string>& env, const Lease6Ptr lease)
{
    env.push_back("KEA_LEASE6_TYPE=" + lease->typeToText(lease->type_));
    env.push_back("KEA_LEASE6_STATE=" + lease->basicStatesToText(lease->state_));
    extract_bool(env, "KEA_LEASE6_IS_EXPIRED", lease->expired());
    env.push_back("KEA_LEASE6_ADDRESS=" + lease->addr_.toText());
    if (lease->type_ == Lease::TYPE_PD) {
        env.push_back("KEA_LEASE6_DELEGATED_PREFIX=" + lease->addr_.toText() + "/" + std::to_string(lease->prefixlen_));
        env.push_back("KEA_LEASE6_DELEGATED_PREFIXLEN=" + std::to_string(lease->prefixlen_));
    }
    if (lease->hwaddr_) {
        env.push_back("KEA_LEASE6_HWADDR=" + lease->hwaddr_->toText(false));
    } else {
        env.push_back("KEA_LEASE6_HWADDR=");
    }
    env.push_back("KEA_LEASE6_HOSTNAME=" + lease->hostname_);
    env.push_back("KEA_LEASE6_CLIENT_DUID=" + lease->duid_->toText());
    env.push_back("KEA_LEASE6_CLIENT_LAST_TRANSMISSION=" + std::to_string(lease->cltt_));
    env.push_back("KEA_LEASE6_VALID_LIFETIME=" + std::to_string(lease->valid_lft_));
    env.push_back("KEA_LEASE6_PREFERRED_LIFETIME=" + std::to_string(lease->preferred_lft_));
    env.push_back("KEA_LEASE6_IAID=" + std::to_string(lease->iaid_));
    env.push_back("KEA_LEASE6_DEBUG=" + lease->toText());
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
    boost::shared_ptr<Option6IA> ia_pd;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    try {
        handle.getArgument("ia_na", ia_na);
        /* TODO: use ia_na */
    } catch (const NoSuchArgument&) { }
    try {
        handle.getArgument("ia_pd", ia_pd);
        /* TODO: use ia_pd */
    } catch (const NoSuchArgument&) { }
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
    boost::shared_ptr<Option6IA> ia_pd;
    handle.getArgument("query6", query);
    extract_query6(env, query);
    handle.getArgument("lease6", lease);
    extract_lease6(env, lease);
    try {
        handle.getArgument("ia_na", ia_na);
        /* TODO: use ia_na */
    } catch (const NoSuchArgument&) { }
    try {
        handle.getArgument("ia_pd", ia_pd);
        /* TODO: use ia_pd */
    } catch (const NoSuchArgument&) { }
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
