// File created from src/messages.mes on Sat Nov 02 2019 19:41

#ifndef MESSAGES_H
#define MESSAGES_H

#include <log/message_types.h>

extern const isc::log::MessageID RUNSCRIPT_EXEC_FAILED;
extern const isc::log::MessageID RUNSCRIPT_FORK_FAILED;
extern const isc::log::MessageID RUNSCRIPT_MISSING_PARAM;
extern const isc::log::MessageID RUNSCRIPT_MISTYPED_PARAM;
extern const isc::log::MessageID RUNSCRIPT_WAITING_SCRIPT;
extern const isc::log::MessageID RUNSCRIPT_WAITPID_FAILED;

#endif // MESSAGES_H
