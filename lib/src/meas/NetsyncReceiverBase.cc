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
