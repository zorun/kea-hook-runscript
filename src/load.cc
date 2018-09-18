#include <signal.h>
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
/* Wait for script to finish executing */
bool script_wait;

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

    ConstElementPtr wait = handle.getParameter("wait");
    if (!wait) {
       script_wait = true;
    } else if (wait->getType() != Element::boolean) {
       LOG_ERROR(runscript_logger, RUNSCRIPT_MISTYPED_PARAM).arg("wait");
       return 1;
    } else {
       script_wait = wait->boolValue();
    }

     /* Install signal handler for non-wait case to avoid leaving  zombie processes around */
     if (!script_wait) {
        signal(SIGCHLD, SIG_IGN);
     }

    return 0;
}

} // end extern "C"
