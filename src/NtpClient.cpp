#include "NtpClient.h"

#include "Debug.h"

// PUBLIC:
void NtpClient::setup() {
  mUdp.begin(123);
  mCurrentState = UdpSetup;
}

void NtpClient::update() {
  switch (mCurrentState) {
    case Uninitialized:
      return;
    case UdpSetup:
      return;
    case SendingTimeRequest:
      sendNtpRequest();
      break;
    case AwaitingTimeResponse:
      if (checkTimeout()) {
        waitForTimeResponse();
      }
      break;
    case TimeAvailable:
      return;
    case Timeout:
      requestTime();
      break;
  }
}

void NtpClient::requestTime() {
  if (mCurrentState == Uninitialized) {
    return;
  }
  mCurrentState = SendingTimeRequest;
}

u_long NtpClient::getUnixTimestamp() {
  if (mCurrentState != TimeAvailable) {
    return 0;
  }
  return mTimestamp - UNIX_TIMESTAMP_OFFSET;
}

// PRIVATE:

void NtpClient::waitForTimeResponse() {
  size_t packetSize = mUdp.parsePacket();

  if (packetSize == 0) {
    return;
  }

  D_Println(packetSize);

  memset(mBuffer, 0, NTP_PACKET_SIZE);

  mUdp.read(mBuffer, packetSize);

  // read first 32 bits = 4 bytes as the seconds in byte 40-48
  u_long high = word(mBuffer[40], mBuffer[41]);
  u_long low = word(mBuffer[42], mBuffer[43]);

  u_long timestamp = (high << 16 | low);

  mTimestamp = timestamp;

  mCurrentState = TimeAvailable;

  mOnTimeReceivedCb(mTimestamp);
}

bool NtpClient::checkTimeout() {
  if (millis() - mRequestTimestamp >= TIMEOUT_SEC) {
    D_Println("Timeout");
    mCurrentState = Timeout;
    return false;
  }
  return true;
}

void NtpClient::sendNtpRequest() {
  memset(mBuffer, 0, NTP_PACKET_SIZE);
  mRequestTimestamp = millis();
  mCurrentState = AwaitingTimeResponse;

  int r = mUdp.beginPacket("pool.ntp.org", 123);
  if (r == 0) {
    D_Println("Failed");
  }
  // Set server, version and mode
  mBuffer[0] = 0b11100011;

  mUdp.write(mBuffer, NTP_PACKET_SIZE);
  mUdp.endPacket();
}
