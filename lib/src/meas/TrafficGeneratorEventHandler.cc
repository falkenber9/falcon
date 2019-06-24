/*
 * Copyright (c) 2019 Robert Falkenberg.
 *
 * This file is part of FALCON 
 * (see https://github.com/falkenber9/falcon).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */
#include "falcon/meas/TrafficGeneratorEventHandler.h"

using namespace std;

void traffic_generator_action_before_transfer(void* obj) {
  if(obj) static_cast<TrafficGeneratorEventHandler*>(obj)->actionBeforeTransfer();
}

void traffic_generator_action_during_transfer(void* obj) {
  if(obj) static_cast<TrafficGeneratorEventHandler*>(obj)->actionDuringTransfer();
}

void traffic_generator_action_after_transfer(void* obj) {
  if(obj) static_cast<TrafficGeneratorEventHandler*>(obj)->actionAfterTransfer();
}

const probe_event_handlers_t ev_funcs_default {
  traffic_generator_action_before_transfer,
  traffic_generator_action_during_transfer,
  traffic_generator_action_after_transfer
};
