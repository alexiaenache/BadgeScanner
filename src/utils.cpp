#include <utils.hpp>

// Define global variables
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
int uidCount = 3;
int currentMemberIndex = 0;
ThreeWire myWire(RTC_DAT_PIN, RTC_CLK_PIN, RTC_RST_PIN);
RtcDS1302<ThreeWire> Rtc(myWire);
DHT dht(DHTPIN, DHTTYPE);
// Initialize the members database
user users_db[MAX_UIDS] = {
    {"E37A082F", "John Doe", "", 0, false, 0, false},
    {"E3E40B2F", "Jane Smith", "", 0, true, 0, false},
    {"50E5BF14", "Mary Johnson", "", 0, true, 0, false}};
