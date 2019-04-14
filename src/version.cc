/* Copyright (c) 2017-2019 by Baptiste Jonglez
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// version.cc
#include <hooks/hooks.h>
extern "C" {
int version() {
    return (KEA_HOOKS_VERSION);
}
}
