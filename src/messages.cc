// File created from src/messages.mes on Sat Nov 02 2019 19:41

#include <cstddef>
#include <log/message_types.h>
#include <log/message_initializer.h>

extern const isc::log::MessageID RUNSCRIPT_EXEC_FAILED = "RUNSCRIPT_EXEC_FAILED";
extern const isc::log::MessageID RUNSCRIPT_FORK_FAILED = "RUNSCRIPT_FORK_FAILED";
extern const isc::log::MessageID RUNSCRIPT_MISSING_PARAM = "RUNSCRIPT_MISSING_PARAM";
extern const isc::log::MessageID RUNSCRIPT_MISTYPED_PARAM = "RUNSCRIPT_MISTYPED_PARAM";
extern const isc::log::MessageID RUNSCRIPT_WAITING_SCRIPT = "RUNSCRIPT_WAITING_SCRIPT";
extern const isc::log::MessageID RUNSCRIPT_WAITPID_FAILED = "RUNSCRIPT_WAITPID_FAILED";

namespace {

const char* values[] = {
    "RUNSCRIPT_EXEC_FAILED", "exec() failed, please check that the script exists and is executable. Error: %1",
    "RUNSCRIPT_FORK_FAILED", "fork() failed with error: %1",
    "RUNSCRIPT_MISSING_PARAM", "required parameter \"%1\" missing in configuration",
    "RUNSCRIPT_MISTYPED_PARAM", "parameter \"%1\" in configuration has wrong type",
    "RUNSCRIPT_WAITING_SCRIPT", "the user-defined script is running, and the main process is currently waiting",
    "RUNSCRIPT_WAITPID_FAILED", "waitpid() failed with error: %1",
    NULL
};

const isc::log::MessageInitializer initializer(values);

} // Anonymous namespace

