  #include <Wire.h>
#include <RDA5807.h>
#include <TM1637Display.h>
#include <IRremote.h>
#include <EEPROM.h> // Bug 6: Added EEPROM for saving frequency

// --- IR Codes ---
#define IR_1         0xBA45FF00
#define IR_2         0xB946FF00
#define IR_3         0xB847FF00
#define IR_4         0xBB44FF00
#define IR_5         0xBF40FF00
#define IR_6         0xBC43FF00
#define IR_7         0xF807FF00
#define IR_VOL_UP    0xE718FF00
#define IR_VOL_DOWN  0xAD52FF00
#define IR_FREQ_DOWN 0xF708FF00
#define IR_FREQ_UP   0xA55AFF00
#define IR_MUTE      0xE31CFF00

// --- Pins ---
const int IR_PIN = 10;
const int btnVolUp = 5;
const int btnVolDown = 6;
const int btnFreqUp = 7;
const int btnFreqDown = 8;

RDA5807 radio;
TM1637Display display(2, 3);

int presets[] = {9110, 9350, 9430, 9500, 9670, 9830, 10400};
int currentVolume = 7;
bool isMuted = false;
uint16_t currentFreq; // Stored frequency

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_PIN);
  
  pinMode(btnVolUp, INPUT_PULLUP);
  pinMode(btnVolDown, INPUT_PULLUP);
  pinMode(btnFreqUp, INPUT_PULLUP);
  pinMode(btnFreqDown, INPUT_PULLUP);
  
  radio.setup();
  
  // Bug 6: Read last saved frequency from EEPROM
  EEPROM.get(0, currentFreq);
  if (currentFreq < 8700 || currentFreq > 10800) {
    currentFreq = 9830; // Default fail-safe frequency
  }
  
  radio.setVolume(currentVolume);
  radio.setFrequency(currentFreq);
  
  // Bug 4: Allow Stereo mode
  radio.setMono(false); 
  
  display.setBrightness(0x0f);
  updateDisplay();
}

void loop() {
  // Handle IR Remote
  if (IrReceiver.decode()) {
    processIR(IrReceiver.decodedIRData.decodedRawData);
    IrReceiver.resume();
  }

  // Handle Physical Buttons
  if (digitalRead(btnFreqUp) == LOW) { 
    radio.seek(RDA_SEEK_WRAP, RDA_SEEK_UP); 
    saveAndDisplayFreq(); 
    delay(300); 
  }
  
  if (digitalRead(btnFreqDown) == LOW) { 
    radio.seek(RDA_SEEK_WRAP, RDA_SEEK_DOWN); 
    saveAndDisplayFreq(); 
    delay(300); 
  }
  
  if (digitalRead(btnVolUp) == LOW) { 
    unmuteIfNeeded(); // Bug 5
    if (currentVolume < 15) { // Bug 1
      radio.setVolume(++currentVolume);
    }
    showVolume(); // Bug 2 & 3
    delay(300); 
  }
  
  if (digitalRead(btnVolDown) == LOW) { 
    unmuteIfNeeded(); // Bug 5
    if (currentVolume > 0) { // Bug 1
      radio.setVolume(--currentVolume);
    }
    showVolume(); // Bug 2 & 3
    delay(300); 
  }

  // Continuously refresh stereo indicator and signal strength (Bug 7)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    updateDisplay();
    lastUpdate = millis();
  }
}

// Helper to save frequency to EEPROM to avoid losing station on power cut
void saveAndDisplayFreq() {
  currentFreq = radio.getFrequency();
  EEPROM.put(0, currentFreq);
  updateDisplay();
}

// Auto-unmute logic
void unmuteIfNeeded() {
  if (isMuted) {
    isMuted = false;
    radio.setMute(false);
  }
}

// Custom volume display logic
void showVolume() {
  display.clear();
  // Displays the volume on the right side of the screen
  display.showNumberDec(currentVolume, false, 2, 2);
  delay(800);      // Wait for a short moment to show user the volume
  updateDisplay(); // Revert back to frequency display
}

void processIR(unsigned long cmd) {
  if (cmd == 0) return;
  
  bool freqChanged = false;

  if (cmd == IR_1) { radio.setFrequency(presets[0]); freqChanged = true; }
  else if (cmd == IR_2) { radio.setFrequency(presets[1]); freqChanged = true; }
  else if (cmd == IR_3) { radio.setFrequency(presets[2]); freqChanged = true; }
  else if (cmd == IR_4) { radio.setFrequency(presets[3]); freqChanged = true; }
  else if (cmd == IR_5) { radio.setFrequency(presets[4]); freqChanged = true; }
  else if (cmd == IR_6) { radio.setFrequency(presets[5]); freqChanged = true; }
  else if (cmd == IR_7) { radio.setFrequency(presets[6]); freqChanged = true; }
  
  else if (cmd == IR_VOL_UP) {
    unmuteIfNeeded();
    if (currentVolume < 15) radio.setVolume(++currentVolume);
    showVolume();
  }
  else if (cmd == IR_VOL_DOWN) {
    unmuteIfNeeded();
    if (currentVolume > 0) radio.setVolume(--currentVolume);
    showVolume();
  }
  
  else if (cmd == IR_FREQ_UP) {
    radio.seek(RDA_SEEK_WRAP, RDA_SEEK_UP);
    freqChanged = true;
  }
  else if (cmd == IR_FREQ_DOWN) {
    radio.seek(RDA_SEEK_WRAP, RDA_SEEK_DOWN);
    freqChanged = true;
  }
  
  else if (cmd == IR_MUTE) {
    isMuted = !isMuted;
    radio.setMute(isMuted);
  }

  // If station changed via remote, save to EEPROM and update display
  if (freqChanged) {
    saveAndDisplayFreq();
  } else {
    updateDisplay(); // Ensure display updates for mute etc.
  }
}

void updateDisplay() {
  // Bug 7: Stereo indicator using the middle colon of TM1637
  uint8_t dots = radio.isStereo() ? 0b01000000 : 0x00;
  
  // Update display with frequency and stereo colon
  display.showNumberDecEx(radio.getFrequency() / 10, dots, false, 4, 0);
  
  // Bug 7: Log RSSI (Signal Strength) to Serial Monitor
  Serial.print("Station: ");
  Serial.print(radio.getFrequency() / 100.0);
  Serial.print(" MHz | RSSI: ");
  Serial.print(radio.getRssi());
  Serial.print(" | Stereo: ");
  Serial.println(radio.isStereo() ? "Yes" : "No");
}
