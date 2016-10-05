#include <EEPROM.h>

boolean eeprom_read_string(int addr, char* buffer, int bufSize);
boolean eeprom_write_string(int addr, const char* string);
boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes);
boolean eeprom_is_addr_ok(int addr);