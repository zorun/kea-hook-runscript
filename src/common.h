#include <string>

extern "C" {

/* From load.cc */

/* Path of the script to be run in hooks. */
extern std::string script_path;
/* Name of the script (without the leading directory). */
extern std::string script_name;

}
