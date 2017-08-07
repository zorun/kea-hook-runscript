#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/dhcp6.h>
#include <dhcp/pkt6.h>
#include <dhcpsrv/lease.h>

#include <string>
#include <vector>

#include "runscript.h"

using namespace isc::dhcp;
using namespace isc::hooks;

extern "C" {

int lease4_select(CalloutHandle& handle) {
    int ret;
    std::vector<std::string> env;
    env.push_back("FOO=bar");
    ret = run_script("lease4_select", env);
    fprintf(stderr, "ret = %d\n", ret);
    return 0;
}

} // end extern "C"
