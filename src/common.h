/* Copyright (c) 2017-2019 by Baptiste Jonglez
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <string>

extern "C" {

/* From load.cc */

/* Path of the script to be run in hooks. */
extern std::string script_path;
/* Name of the script (without the leading directory). */
extern std::string script_name;
/* Wait for script to finish executing */
extern bool script_wait;

}
