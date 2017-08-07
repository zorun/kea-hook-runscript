#include <hooks/hooks.h>

#include "logger.h"
#include "common.h"

using namespace isc::hooks;
using namespace isc::data;

/* Path of the script to be run in hooks, accessed by the other files of
 * this library. */
std::string script_path;
/* Name of the script (without the leading directory). */
std::string script_name;

extern "C" {

int load(LibraryHandle& handle) {
    ConstElementPtr script = handle.getParameter("script");
    if (!script) {
        LOG_ERROR(runscript_logger, RUNSCRIPT_MISSING_PARAM).arg("script");
        return 1;
    }
    if (script->getType() != Element::string) {
        LOG_ERROR(runscript_logger, RUNSCRIPT_MISTYPED_PARAM).arg("script");
        return 1;
    }
    script_path = script->stringValue();
    script_name = script_path.substr(script_path.find_last_of('/') + 1);

    return 0;
}

} // end extern "C"
