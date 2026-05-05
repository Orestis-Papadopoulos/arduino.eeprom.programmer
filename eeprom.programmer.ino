
// "Build an Arduino EEPROM programmer" https://www.youtube.com/watch?v=K88pgWhEb1M&t=1196s

// I 've used a different EEPROM (AT28C64B) than the video
// I 've wired the EEPROM IO pins differently
// The clocks are wired opposite in the video because he uses the Philips shift register; I have the Texas Instruments one (starts with "SN")

#define SERIAL_INPUT 2         // SHIFT_DATA
#define SHIFT_REGISTER_CLK 4   // SHIFT_CLK
#define STORAGE_REGISTER_CLK 3 // SHIFT_LATCH
#define EEPROM_IO1 12
#define EEPROM_IO8 5
#define WRITE_EN 13
#define MAX_ADDRESS 2048       // 11 address lines implemented

// 4-bit hex decoder for common anode 7-segment display
byte data[] = { 0x81, 0xcf, 0x92, 0x86, 0xcc, 0xa4, 0xa0, 0x8f, 0x80, 0x84, 0x88, 0xe0, 0xb1, 0xc2, 0xb0, 0xb8 };

void setAddress(int address, bool output_enable) {
  // "address" bytes: 2 (two shift outs)
  // address lines used on eeprom chip: 11
  // output enable pin: 1
  // spare pins: 4
  shiftOut(SERIAL_INPUT, SHIFT_REGISTER_CLK, MSBFIRST, (address >> 8) | (output_enable ? 0x00 : 0x80));
  shiftOut(SERIAL_INPUT, SHIFT_REGISTER_CLK, MSBFIRST, address);
  digitalWrite(STORAGE_REGISTER_CLK, LOW);
  digitalWrite(STORAGE_REGISTER_CLK, HIGH);
  digitalWrite(STORAGE_REGISTER_CLK, LOW);
}

byte readEEPROM (int address) {
  for (int pin = EEPROM_IO8; pin <= EEPROM_IO1; pin++) {
    pinMode(pin, INPUT);
  }

  setAddress(address, true);
  byte data = 0;
  for (int pin = EEPROM_IO8; pin <= EEPROM_IO1; pin++) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

void writeEEPROM (int address, byte data) {
  setAddress(address, false); // must go before loop (check first comment of video)
  for (int pin = EEPROM_IO8; pin <= EEPROM_IO1; pin++) {
    pinMode(pin, OUTPUT);
  }

  for (int pin = EEPROM_IO1; pin >= EEPROM_IO8; pin--) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1); // minimum 100 ns (0.1 μs) is required according to datasheet
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

void eraseEEPROM() {
  Serial.print("\nErasing EEPROM");
  for (int address = 0; address < MAX_ADDRESS; address++) {
    writeEEPROM(address, 0xff);
    if (address % 64 == 0) Serial.print(".");
  }
  Serial.println("EEPROM erased\n");
}

void printContentsUpTo(int address) { // 2047 max (11 address lines)
  for (int base = 0; base <= (address + 1) / 2; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset++) {
      data[offset] = readEEPROM(base + offset);
    }

    char buffer[80];
    sprintf(buffer, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x", base,
    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buffer);
  }
}

void setup() {
  pinMode(SERIAL_INPUT, OUTPUT);
  pinMode(SHIFT_REGISTER_CLK, OUTPUT);
  pinMode(STORAGE_REGISTER_CLK, OUTPUT);
  
  // in that order
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(57600);

  eraseEEPROM();

  // program data bytes
  Serial.print("Programming EEPROM");
  for (int address = 0; address < sizeof(data); address++) {
    writeEEPROM(address, data[address]);
    if (address % 64 == 0) Serial.print(".");
  }
  Serial.println(" Done\n");

  printContentsUpTo(2047);
}

void loop() {
}
