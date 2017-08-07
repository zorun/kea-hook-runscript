#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/dhcp6.h>
#include <dhcp/pkt6.h>

#include <string>
#include <vector>

#include "runscript.h"

using namespace isc::dhcp;
using namespace isc::hooks;

extern "C" {

int pkt4_receive(CalloutHandle& handle) {
    int ret;
    std::vector<std::string> env;
    env.push_back("DHCP4_RELAYED=1");
    env.push_back("HWADDR_TYPE=foo");
    ret = run_script("pkt4_receive", env);
    fprintf(stderr, "ret = %d\n", ret);
    return 0;
}

} // end extern "C"
