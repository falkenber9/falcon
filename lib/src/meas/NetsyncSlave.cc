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
#include "falcon/meas/NetsyncSlave.h"
#include <sstream>

using namespace std;

NetsyncSlave::NetsyncSlave(const uint16_t port) :
  NetsyncReceiverBase(),
  SignalHandler(),
  broadcastSlave(port),
  modem(nullptr),
  startFlag(false),
  stopFlag(false),
  stopObject(nullptr),
  remoteParams(),
  textstream()
{

}

NetsyncSlave::~NetsyncSlave() {

}

void NetsyncSlave::pollReply() {
  // build text message (payload)
  ostringstream msgText;
  if(modem == nullptr) {
    msgText << "Poll reply, no modem";
  }
  else {
    msgText << modem->getOperatorName() << " ";
    msgText << modem->getNetworkInfo();
  }

  replyText(msgText.str());
}

void NetsyncSlave::startReply() {
  replyText("Received 'start' command.");
}

void NetsyncSlave::stopReply() {
  replyText("Received 'stop' command.");
}

void NetsyncSlave::notifyFinished() {
  replyText("Task finished.");
}

void NetsyncSlave::notifyError(const string text) {
  replyText("Error: " + text);
}

void NetsyncSlave::init(AuxModem* modem) {
  this->modem = modem;
}

bool NetsyncSlave::waitStart() {
  cout << "Waiting for start" << endl;
  unique_lock<mutex> lck(startMutex);
  startConditionVar.wait(lck, [this](){
    bool result = startFlag;
    startFlag = false;  //one shot, only
    return result;
  });
  return !stopFlag;
}

void NetsyncSlave::signalStart() {
  signal(false);
}

void NetsyncSlave::signalAbort() {
  signal(true);
}

bool NetsyncSlave::stopReceived() {
  return stopFlag;
}

void NetsyncSlave::attachCancelable(Cancelable* obj) {
  stopObject = obj;
}

const NetsyncMessageStart&NetsyncSlave::getRemoteParams() const {
  return remoteParams;
}

void NetsyncSlave::handle(NetsyncMessagePoll msg) {
  (void)msg;  //unused
  cout << "Received poll message." << endl;
  pollReply();
}

void NetsyncSlave::handle(NetsyncMessageStart msg) {
  remoteParams = msg; // store a copy as params

  cout << "Start:" << endl;
  cout << " id: " << remoteParams.getId() << endl;
  cout << " nofSubframes: " << remoteParams.getNofSubframes() << endl;
  cout << " offsetSubframes: " << remoteParams.getOffsetSubframes() << endl;
  cout << " direction: " << remoteParams.getDirection() << endl;
  cout << " payloadSize: " << remoteParams.getPayloadSize() << endl;
  cout << " url: " << remoteParams.getUrl() << endl;
  cout << " latitude: " << remoteParams.getLatitude() << endl;
  cout << " longitude: " << remoteParams.getLongitude() << endl;

  startReply();
  signalStart();
}

void NetsyncSlave::handle(NetsyncMessageStop msg) {
  (void)msg;  //unused
  stopFlag = true;
  if(stopObject != nullptr) stopObject->cancel();
  stopReply();
  signalAbort();
}

void NetsyncSlave::replyText(const string& text) {
  NetsyncMessageText msg;
  msg.setText(text);
  stringstream ss(stringstream::out | stringstream::binary);
  ss << msg;
  string str = ss.str();
  broadcastSlave.replyBytes(str.c_str(), str.length());
}

void NetsyncSlave::receive() {
  while(!cancelThread) {
    size_t size;
    size = broadcastSlave.receiveBytes(recvThreadBuf, recvThreadBufSize);
    parse(recvThreadBuf, recvThreadBufSize);
  }
}

void NetsyncSlave::signal(bool abort) {
  {
    lock_guard<mutex> lck(startMutex);
    startFlag = true;
    stopFlag = abort;
  }
  cout << "Received start/stop - unlocked" << endl;
  startConditionVar.notify_all();
}

void NetsyncSlave::handleSignal() {
  signalAbort();
}

NetsyncSlave& operator<<(NetsyncSlave &s, NetsyncSlave& (*f)(NetsyncSlave&)) {
  return f(s);
}

//NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ostream&)) {
//  f(std::cout);
//  return s;
//}

//NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ios&)) {
//  f(std::cout);
//  return s;
//}

//NetsyncSlave& operator<<(NetsyncSlave &s, std::ostream& (*f)(std::ios_base&)) {
//  f(std::cout);
//  return s;
//}

NetsyncSlave& endl(NetsyncSlave& s) {
  std::cout << endl;
  s.replyText((s.modem == nullptr ? "No modem" : s.modem->getOperatorNameCached()) + ": " + s.textstream.str());
  //erase textstream
  s.textstream.str(string()); // put empty string
  s.textstream.clear(); // affects error state, only
  return s;
  //return std::flush( os.put( os.widen('\n') ) );
}
