#include <SoftwareSerial.h>

#define CALLER_ID_RX 10
#define CaLLER_ID_TX 11

#define RING_DET_PIN 8
#define CLI_PWDN_PIN 9

#define MDMF_HEADER 0x80
#define MDMF_PARAM_TIME 0x01
#define MDMF_PARAM_CID 0x02
#define MDMF_PARAM_NAME 0x07

enum CID_STATE { CID_IDLE,
                 CID_SYNC,
                 CID_PACKET,
                 CID_MESSAGE,
                 CID_END };
enum CID_MSG_STATE { CIDMSG_HEADER,
                     CIDMSG_LEN,
                     CIDMSG_DATA };

SoftwareSerial callerID(CALLER_ID_RX, CaLLER_ID_TX);

enum CID_STATE cidState;
enum CID_MSG_STATE cidMsg;

unsigned char tempCount, cidData, packetLen, currLen;
unsigned char msgType, msgLen, msgCurrPos;
unsigned long delayStart, watchdogDelay;
unsigned char lineIdleCounter, watchdogSec;
unsigned char tempInfoPos;

char msgData[16];

void setup() {
  Serial.begin(115200);
  callerID.begin(1200);

  pinMode(RING_DET_PIN, INPUT);
  pinMode(CLI_PWDN_PIN, OUTPUT);
  pinMode(CALLER_ID_RX, INPUT);
  pinMode(CaLLER_ID_TX, OUTPUT);

  digitalWrite(CLI_PWDN_PIN, HIGH);

  cidState = CID_IDLE;
  cidMsg = CIDMSG_HEADER;

  resetWatchdog();
}

void loop() {

  if ((cidState == CID_SYNC) || (cidState == CID_PACKET) || (cidState == CID_MESSAGE)) {
    // Watchdog expire-counter increments on every one second.
    if ((micros() - watchdogDelay) >= 1000000) {
      watchdogSec++;
    }

    if (watchdogSec > 10) {
      resetWatchdog();

      cidState = CID_IDLE;
      cidMsg = CIDMSG_HEADER;
      tempCount = 0;
      lineIdleCounter = 0;
      msgCurrPos = 0;

      return;
    }
  }

  if ((cidState == CID_IDLE) && digitalRead(RING_DET_PIN)) {
    Serial.println("INCOMING CALL");

    pinMode(CLI_PWDN_PIN, LOW);
    tempCount = 0;

    resetWatchdog();

    cidState = CID_SYNC;
  } else if (cidState == CID_END) {
    // Caller ID decoding is finished, lets wait until the ringing is finished.
    if ((micros() - delayStart) >= 100000) {
      delayStart = micros();
      lineIdleCounter++;

      resetWatchdog();

      if (digitalRead(RING_DET_PIN)) {
        // Ring signal/session is still active.
        lineIdleCounter = 0;
      }
    }

    // Wait for 8 seconds to start new line sensing session.
    if (lineIdleCounter > 80) {
      // Switch system to the idle state.
      lineIdleCounter = 0;
      cidState = CID_IDLE;
    }
  } else if ((cidState == CID_SYNC) && (callerID.available())) {
    // Process sync data received from callerID and waiting for packet ID.
    cidData = callerID.read();

    resetWatchdog();

    if ((tempCount >= 25) && (cidData == MDMF_HEADER)) {
      Serial.println("MDMF PACKET");
      cidState = CID_PACKET;
      resetWatchdog();
    }

    if (cidData == 0x55) {
      // Sync byte detected, increment the sync data counter.
      tempCount++;
    }
  } else if ((cidState == CID_PACKET) && (callerID.available())) {
    packetLen = callerID.read();
    currLen = 0;
    cidState = CID_MESSAGE;
    cidMsg = CIDMSG_HEADER;

    resetWatchdog();
  } else if ((cidState == CID_MESSAGE) && (callerID.available())) {
    if (currLen >= packetLen) {
      Serial.println("MDMF PACKET END");

      cidState = CID_END;
      cidMsg = CIDMSG_HEADER;

      // Initialize idle state monitoring variables.
      lineIdleCounter = 0;
      delayStart = micros();

      resetWatchdog();

      // Read checksum byte.
      callerID.read();

      // Shutdown caller id decoder.
      pinMode(CLI_PWDN_PIN, HIGH);
      return;
    }

    // Process messages in MDMF packet.
    if (cidMsg == CIDMSG_HEADER) {
      msgType = callerID.read();

      Serial.print(" MSG HEADER: ");
      Serial.println(msgType, HEX);

      cidMsg = CIDMSG_LEN;
      currLen++;

      resetWatchdog();
    } else if (cidMsg == CIDMSG_LEN) {
      msgLen = callerID.read();

      Serial.print(" MSG LEN: ");
      Serial.println(msgLen, HEX);

      cidMsg = CIDMSG_DATA;
      msgCurrPos = 0;
      currLen++;

      resetWatchdog();
    } else if (cidMsg == CIDMSG_DATA) {
      if (msgCurrPos < 15) {
        msgData[msgCurrPos] = callerID.read();
        msgData[msgCurrPos + 1] = 0;
      } else {
        callerID.read();
      }

      msgCurrPos++;
      currLen++;

      if (msgCurrPos >= msgLen) {
        Serial.print(" MSG END ");
        Serial.println(msgData);

        switch (msgType) {
          case MDMF_PARAM_TIME:
            printCallerIDTime();
            break;
          case MDMF_PARAM_CID:
            Serial.println(msgData);
            break;
          case MDMF_PARAM_NAME:
            Serial.println(msgData);
            break;
        }

        cidMsg = CIDMSG_HEADER;

        resetWatchdog();
      }
    }
  } else if (cidState == CID_IDLE) {
    // System and phone line are on idle state.
  }
}

void resetWatchdog() {
  watchdogDelay = micros();
  watchdogSec = 0;
}

void printCallerIDTime() {
  unsigned char month = ((msgData[0] - 0x30) * 10) + (msgData[1] - 0x30);
  char valStr[3];
  String monthStr = "Jan";

  switch (month) {
    case 2:
      monthStr = "Feb";
      break;
    case 3:
      monthStr = "Mar";
      break;
    case 4:
      monthStr = "Apr";
      break;
    case 5:
      monthStr = "May";
      break;
    case 6:
      monthStr = "Jun";
      break;
    case 7:
      monthStr = "Jul";
      break;
    case 8:
      monthStr = "Aug";
      break;
    case 9:
      monthStr = "Sep";
      break;
    case 10:
      monthStr = "Oct";
      break;
    case 11:
      monthStr = "Nov";
      break;
    case 12:
      monthStr = "Dec";
      break;
  }

  Serial.print(monthStr);

  // date
  valStr[0] = msgData[2];
  valStr[1] = msgData[3];
  valStr[2] = 0;

  Serial.print(valStr);

  // hour
  valStr[0] = msgData[4];
  valStr[1] = msgData[5];
  valStr[2] = 0;

  Serial.print(valStr);

  // Print time seperator.
  Serial.print(':');

  //Print minutes
  valStr[0] = msgData[6];
  valStr[1] = msgData[7];
  valStr[2] = 0;

  Serial.println(valStr);
}