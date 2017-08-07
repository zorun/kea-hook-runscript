#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/dhcp6.h>
#include <dhcp/pkt6.h>

#include "runscript.h"

using namespace isc::dhcp;
using namespace isc::hooks;

extern "C" {

int pkt4_receive(CalloutHandle& handle) {
    int ret;
    char *env[] = { "HOME=/usr/home", "FOO=bar", (char *)NULL };
    ret = run_script("/tmp/test.sh", "pkt4_receive", env);
    fprintf(stderr, "ret = %d\n", ret);
    return 0;
}

} // end extern "C"
