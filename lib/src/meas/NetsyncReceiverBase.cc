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
#include "falcon/meas/NetsyncReceiverBase.h"

#include <iostream>

#define RECEIVE_BUFFER_SIZE 4000

NetsyncReceiverBase::NetsyncReceiverBase() :
  cancelThread(false)
{
  recvThreadBuf = new char[RECEIVE_BUFFER_SIZE];
  recvThreadBufSize = RECEIVE_BUFFER_SIZE;
}

NetsyncReceiverBase::~NetsyncReceiverBase() {
  cancelThread = true;
  if(!pthread_kill(recvThread, 0)) {
    pthread_cancel(recvThread);
    pthread_join(recvThread, nullptr);
  }
  delete[] recvThreadBuf;
  recvThreadBuf = nullptr;
  recvThreadBufSize = 0;
}

void NetsyncReceiverBase::launchReceiver() {
  if(pthread_create(&recvThread, nullptr, receiveStart, this)) {
    cerr << "could not create recvThread" << endl;
  }
}

void NetsyncReceiverBase::parse(const char *msg, size_t len) {
  const RawMessage& raw = *reinterpret_cast<const RawMessage*>(msg);
  size_t payloadSize = len - sizeof (raw.type);
  switch (raw.type) {
    case NMIStart:
      handle(NetsyncMessageStart(&raw.firstPayloadByte, payloadSize));
      break;
    case NMIStop:
      handle(NetsyncMessageStop(&raw.firstPayloadByte, payloadSize));
      break;
    case NMIPoll:
      handle(NetsyncMessagePoll(&raw.firstPayloadByte, payloadSize));
      break;
    case NMIText:
      handle(NetsyncMessageText(&raw.firstPayloadByte, payloadSize));
      break;
    default:
      cerr << "received unknown message type: " << raw.type << endl;
      break;
  }
}

void NetsyncReceiverBase::handle(NetsyncMessageStart msg) {
  cout << "Unhandled message 'start'" << endl;
}

void NetsyncReceiverBase::handle(NetsyncMessageStop msg) {
  cout << "Unhandled message 'stop'" << endl;
}

void NetsyncReceiverBase::handle(NetsyncMessagePoll msg) {
  cout << "Unhandled message 'poll'" << endl;
}

void NetsyncReceiverBase::handle(NetsyncMessageText msg) {
  cout << "Unhandled message 'text': " << msg.getText() << endl;
}

void *NetsyncReceiverBase::receiveStart(void *obj) {
  static_cast<NetsyncReceiverBase*>(obj)->receive();
  return nullptr;
}
