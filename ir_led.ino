#include <Adafruit_NeoPixel.h>

#define DECODE_NEC          // Includes Apple and Onkyo
#include <IRremote.hpp> // include the library

Adafruit_NeoPixel strip(150, 2, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  // Just to know which program is running on my Arduino
  // Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
  IrReceiver.begin(3, ENABLE_LED_FEEDBACK);

  // Serial.print(F("Ready to receive IR signals of protocols: "));
  // printActiveIRProtocols(&Serial);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

uint16_t hue = 65536 * 0.66;
uint8_t sat = 255;
uint8_t val = 255;

void loop() {
  /*
    * Check if received data is available and if yes, try to decode it.
    * Decoded result is in the IrReceiver.decodedIRData structure.
    *
    * E.g. command is in IrReceiver.decodedIRData.command
    * address is in command is in IrReceiver.decodedIRData.address
    * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
    */
  if (IrReceiver.decode()) {
    /*
      * Print a short summary of received data
      */
    // IrReceiver.printIRResultShort(&Serial);
    // IrReceiver.printIRSendUsage(&Serial);
    // if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
    //     Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
    //     // We have an unknown protocol here, print more info
    //     IrReceiver.printIRResultRawFormatted(&Serial, true);
    // }
    // Serial.println();

    /*
      * !!!Important!!! Enable receiving of the next value,
      * since receiving has stopped after the end of the current received data packet.
      */
    IrReceiver.resume(); // Enable receiving of the next value

    switch(IrReceiver.decodedIRData.command) {
      case 0: // volume down decrease brightness
      if (val > 0) {
        val -= 15;
      }
      break;
      case 2: // volume up increase brightness
      if (val < 255) {
        val += 15;
      }
      break;
      case 0x8: // ch- saturation down
      if (sat > 0) {
        sat -= 15;
      }
      break;
      case 0xA: // ch+ saturation up
      if (sat < 255) {
        sat += 15;
      }
      break;
      case 0x10:
      hue = 65536 * 0.0;
      break;
      case 0x11:
      hue = 65536 * 0.11;
      break;
      case 0x12:
      hue = 65536 * 0.22;
      break;
      case 0x14:
      hue = 65536 * 0.33;
      break;
      case 0x15:
      hue = 65536 * 0.44;
      break;
      case 0x16:
      hue = 65536 * 0.55;
      break;
      case 0x18:
      hue = 65536 * 0.66;
      break;
      case 0x19:
      hue = 65536 * 0.77;
      break;
      case 0x1A:
      hue = 65536 * 0.88;
      break;
    }
    strip.fill(strip.gamma32(strip.ColorHSV(hue, sat, val)));
    strip.show();
  }
}

void handleReceivedTinyIRData(uint8_t aAddress, uint8_t aCommand, uint8_t aFlags) {
  printTinyReceiverResultMinimal(&Serial, aAddress, aCommand, aFlags);
}