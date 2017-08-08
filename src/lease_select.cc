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

int lease4_select(CalloutHandle& handle) {
    std::vector<std::string> env;
    Pkt4Ptr query;
    Subnet4Ptr subnet;
    bool fake_allocation;
    Lease4Ptr lease;
    handle.getArgument("query4", query);
    handle.getArgument("subnet4", subnet);
    handle.getArgument("fake_allocation", fake_allocation);
    handle.getArgument("lease4", lease);
    /* Run script */
    int ret;
    ret = run_script("lease4_select", env);
    fprintf(stderr, "ret = %d\n", ret);
    return 0;
}

} // end extern "C"
