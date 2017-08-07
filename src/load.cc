#include <hooks/hooks.h>

using namespace isc::hooks;
using namespace isc::data;

/* Path of the script to run in hooks, accessed by the other files of this
 * library. */
std::string script_path;

extern "C" {

int load(LibraryHandle& handle) {
    ConstElementPtr script = handle.getParameter("script");
    if (!script) {
        return 1;
    }
    if (script->getType() != Element::string) {
        return 1;
    }
    script_path = script->stringValue();
    
    return 0;
}

} // end extern "C"
