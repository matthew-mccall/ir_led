#include <Adafruit_NeoPixel.h>

#define DECODE_NEC          // Includes Apple and Onkyo
#include <IRremote.hpp> // include the library

#define NUM_PIXELS 150
Adafruit_NeoPixel strip(NUM_PIXELS, 2, NEO_GRB + NEO_KHZ800);

unsigned long lastMillis;

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

  lastMillis = millis();
}

enum Mode {
  MONOCOLOR = 0, 
  GRADIENT,
};

const int sampleWindow = 33; // Sample window width in mS (33 mS = 30Hz)
uint16_t huePerSecond = 6552;
unsigned int sample;

uint16_t hue = 65536 * 0.66;
uint16_t hue2 = 0;
uint8_t sat = 255;
uint8_t val = 255;
bool soundTrigger = false;
bool movingHues = false;
int mode = Mode::MONOCOLOR;

void loop() {
  unsigned long deltaMillis = millis() - lastMillis;
  lastMillis = millis();
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
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
        // We have an unknown protocol here, print more info
        IrReceiver.printIRResultRawFormatted(&Serial, true);
    }
    Serial.println();

    /*
      * !!!Important!!! Enable receiving of the next value,
      * since receiving has stopped after the end of the current received data packet.
      */
    IrReceiver.resume(); // Enable receiving of the next value

    switch(IrReceiver.decodedIRData.command) {
      case 0x0: // volume down decrease brightness
      if (val > 0) {
        val -= 15;
      }
      break;
      case 0x1: // pause/play toggle sound mode
      soundTrigger = !soundTrigger;
      if (!soundTrigger) val = 255;
      break;
      case 0x2: // volume up increase brightness
      if (val < 255) {
        val += 15;
      }
      break;
      case 0x4: // setup moving hues
      movingHues = !movingHues;
      break;
      case 0x5: // prev decrease hue
        hue -= 771;
      break;
      case 0x6: // mode decrease hue2
        hue2 -= 771;
      break;
      case 0x8: // ch- saturation down
      if (sat > 0) {
        sat -= 15;
      }
      break; 
      case 0x9: // cycle gradient modes
      ++mode %= 2
      // Serial.println(mode);
      break;
      case 0xA: // ch+ saturation up
      if (sat < 255) {
        sat += 15;
      }
      break;
      case 0xC: // 0 10+ cycle moving hue speeds
      huePerSecond += 1091;
      break;
      case 0xD: // next increase hue
        hue += 771;
      break;
      case 0xE: // return increase hue2
        hue2 += 771;
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
      default:
      return;
    }
  }

  if (soundTrigger && IrReceiver.isIdle()) { // https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels
    unsigned long startMillis= millis();  // Start of sample window
    unsigned int peakToPeak = 0;   // peak-to-peak level

    unsigned int signalMax = 100;
    unsigned int signalMin = 800;

    // collect data for 50 mS
    while (millis() - startMillis < sampleWindow)
    {
        sample = analogRead(0);
        if (sample < 1024)  // toss out spurious readings
        {
          if (sample > signalMax)
          {
              signalMax = sample;  // save just the max levels
          }
          else if (sample < signalMin)
          {
              signalMin = sample;  // save just the min levels
          }
        }
    }
    peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    double volts = (peakToPeak * 255.0) / 1024;  // convert to volts
    val = volts;
    // Serial.println(val);
  }

  if (movingHues) {
    float deltaHue = huePerSecond * deltaMillis / 1000.f;
    hue += deltaHue;
    hue2 += deltaHue;
  }

  switch (mode) {
    case Mode::MONOCOLOR:
    strip.fill(strip.gamma32(strip.ColorHSV(hue, sat, val)));
    break;
    case Mode::GRADIENT:
    {
      float step = (hue - hue2) / 150.f;
      // Serial.println("gradient ");

      for (int i = 0; i < NUM_PIXELS; i++) {
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue + (step * i), sat, val)));
      }
    }
    break;
  }

  if (IrReceiver.isIdle()) { strip.show();}    
}

void handleReceivedTinyIRData(uint8_t aAddress, uint8_t aCommand, uint8_t aFlags) {
  printTinyReceiverResultMinimal(&Serial, aAddress, aCommand, aFlags);
}