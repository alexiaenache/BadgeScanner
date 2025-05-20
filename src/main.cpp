#include <utils.hpp>

// Global variables for menu navigation and state
bool detailMode = false;   // Indicates if detailed view mode is active (currently unused)
bool adminFlag = false;    // Flag indicating if admin is logged in
bool updateDisplay = true; // Flag to indicate when to update the LCD display
int mainMenuIndex = 0;     // Tracks the current main menu selection index
int menuLevel = 0;         // Current menu level (0 = main menu, 1 = member list, 2 = member details)

// Static variables for internal state management
static int detailIndex = 0;                 // Index used to cycle through member detail pages
static unsigned long lastDebounceTime = 0;  // Timestamp for last joystick input processed
const unsigned long debounceDelay = 300;    // Debounce delay for joystick button press (milliseconds)
const unsigned long navigationDelay = 1000; // Delay for left/right navigation to prevent fast scrolling

// --- Helper functions for menu navigation ---

// Navigate one step left based on current menu level
void navigateLeft()
{
  if (menuLevel == 0)
  {
    // Navigate left in main menu, wrap around if at first item
    mainMenuIndex = (mainMenuIndex > 0) ? mainMenuIndex - 1 : 3;
  }
  else if (menuLevel == 1)
  {
    // Navigate left in member list, wrap around to last member if at first
    currentMemberIndex = (currentMemberIndex > 0) ? currentMemberIndex - 1 : uidCount - 1;
  }
  else if (menuLevel == 2)
  {
    // Cycle left through member detail pages, wrap around if at first detail page
    detailIndex = (detailIndex > 0) ? detailIndex - 1 : 4;
  }
  updateDisplay = true;        // Mark that LCD should be updated after navigation
  lastDebounceTime = millis(); // Update debounce timer to prevent rapid input repeats
}

// Navigate one step right based on current menu level
void navigateRight()
{
  if (menuLevel == 0)
  {
    // Navigate right in main menu, wrap around if at last item
    mainMenuIndex = (mainMenuIndex < 3) ? mainMenuIndex + 1 : 0;
  }
  else if (menuLevel == 1)
  {
    // Navigate right in member list, wrap around to first member if at last
    currentMemberIndex = (currentMemberIndex < uidCount - 1) ? currentMemberIndex + 1 : 0;
  }
  else if (menuLevel == 2)
  {
    // Cycle right through member detail pages, wrap around if at last detail page
    detailIndex = (detailIndex < 4) ? detailIndex + 1 : 0;
  }
  updateDisplay = true;        // Mark that LCD should be updated after navigation
  lastDebounceTime = millis(); // Update debounce timer
}

// Select or "enter" the current menu option or detail
void selectMenuOption()
{
  if (menuLevel == 0)
  {
    // Main menu selection handling
    switch (mainMenuIndex)
    {
    case 0:
      menuLevel = 1; // Enter member list view
      break;
    case 1:
      addCardAccess(); // Trigger adding card access procedure
      break;
    case 2:
      removeCardAccess(); // Trigger removing card access procedure
      break;
    case 3:
      showTotalNumber(); // Show total number of members
      break;
    }
  }
  else if (menuLevel == 1)
  {
    // From member list, enter detailed info view
    menuLevel = 2;
  }
  updateDisplay = true;        // Mark that LCD should be updated after selection
  lastDebounceTime = millis(); // Update debounce timer
}

// Go back or exit current menu level
void goBack()
{
  if (menuLevel == 2)
  {
    // From member details back to member list
    menuLevel = 1;
  }
  else if (menuLevel == 1)
  {
    // From member list back to main menu with a short message
    lcd.clear();
    lcd.print("Exiting...");
    delay(1000);
    menuLevel = 0;
  }
  updateDisplay = true;        // Mark that LCD should be updated after going back
  lastDebounceTime = millis(); // Update debounce timer
}

// --- Function to update the LCD display based on current menu state ---

void updateLCDMenu()
{
  if (menuLevel == 0)
  {
    // Display main menu options
    lcd.clear();
    switch (mainMenuIndex)
    {
    case 0:
      lcd.print("See Members");
      lcd.setCursor(3, 1);
      lcd.print("--page 1--");
      break;
    case 1:
      lcd.print("Add Access");
      lcd.setCursor(3, 1);
      lcd.print("--page 2--");
      break;
    case 2:
      lcd.print("Remove Access");
      lcd.setCursor(3, 1);
      lcd.print("--page 3--");
      break;
    case 3:
      lcd.print("Total Number");
      lcd.setCursor(3, 1);
      lcd.print("--page 4--");
      break;
    default:
      lcd.print("Default");
      break;
    }
  }
  else if (uidCount == 0)
  {
    // No members registered
    lcd.clear();
    lcd.print("No Members");
    menuLevel = 1;
  }
  else if (menuLevel == 1)
  {
    // Show selected member's name in member list
    lcd.clear();
    lcd.print(users_db[currentMemberIndex].name);
    lcd.setCursor(3, 1);
    lcd.print("--page ");
    lcd.print(currentMemberIndex + 1);
    lcd.print("--");
  }
  else if (menuLevel == 2)
  {
    // Show detailed info for selected member based on detailIndex page
    lcd.clear();
    switch (detailIndex)
    {
    case 0:
      lcd.print("UID: ");
      lcd.print(users_db[currentMemberIndex].uid);
      break;
    case 1:
      lcd.print("Logged: ");
      lcd.print(users_db[currentMemberIndex].logged ? "Yes" : "No");
      break;
    case 2:
      lcd.print("Last access:");
      lcd.setCursor(0, 1);
      lcd.print(users_db[currentMemberIndex].lastLogTime);
      break;
    case 3:
    {
      lcd.print("Last time spent:");
      lcd.setCursor(0, 1);
      lcd.print(formatSpentTime(users_db[currentMemberIndex].lastTimeSpent));
      break;
    }
    default:
      lcd.print("Default");
      break;
    }
  }
  updateDisplay = false; // Reset flag after updating display
}

// --- Main function that handles admin menu interaction using joystick input ---

void adminLogged()
{
  int joyX = analogRead(JOYSTICK_URY_PIN); // Read joystick X-axis (left/right)
  int joyY = analogRead(JOYSTICK_URX_PIN); // Read joystick Y-axis (up/down)
  unsigned long currentTime = millis();    // Current system time in ms

  // Left navigation (with delay to prevent rapid changes)
  if (joyX < LOWER_JOYSTICK_THRESHOLD && (currentTime - lastDebounceTime > navigationDelay))
  {
    navigateLeft();
  }
  // Right navigation (with delay)
  else if (joyX > UPPER_JOYSTICK_THRESHOLD && (currentTime - lastDebounceTime > navigationDelay))
  {
    navigateRight();
  }

  // Select button pressed (joystick down)
  if (joyY < LOWER_JOYSTICK_THRESHOLD && (currentTime - lastDebounceTime > debounceDelay))
  {
    selectMenuOption();
  }
  // Back button pressed (joystick up)
  else if (joyY > UPPER_JOYSTICK_THRESHOLD && (currentTime - lastDebounceTime > debounceDelay))
  {
    goBack();
  }

  // Update LCD if flagged
  if (updateDisplay)
  {
    updateLCDMenu();
  }
}

// --- Setup function to initialize hardware and peripherals ---

void setup()
{
  Serial.begin(115200); // Start serial communication for debugging

  // Initialize communication buses and devices
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN); // I2C for LCD
  SPI.begin();                          // SPI for RFID reader
  mfrc522.PCD_Init();                   // Initialize RFID reader
  Rtc.Begin();                          // Initialize RTC
  lcd.init();                           // Initialize LCD
  lcd.backlight();                      // Turn on LCD backlight

  // Set pins as outputs for LEDs and buzzer
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setupLEDControl(); // Setup PWM channels for LEDs

  digitalWrite(BUZZER_PIN, HIGH); // Turn buzzer off initially (assuming active low)
  turnOffLEDs();                  // Turn off all LEDs initially

  Serial.println("Approximate your card to the reader...");

  setDateTime(); // Set RTC date/time based on compile time

  dht.begin(); // Initialize temperature and humidity sensor

  // Initialize joystick pins as inputs
  pinMode(JOYSTICK_URX_PIN, INPUT);
  pinMode(JOYSTICK_URY_PIN, INPUT);
  pinMode(JOYSTICK_SW_PIN, INPUT);

  Serial.println("Setting up members...");
}

// --- Helper functions for handling card processing ---

// Called when admin card is scanned and admin is not logged in
void processAdminCard()
{
  adminAccessMelody(); // Play admin access melody and turn on LEDs
  Serial.println("Admin Access Granted");
  lcd.clear();
  lcd.print("Admin Access");
  lcd.setCursor(0, 1);
  lcd.print("Granted");
  adminFlag = true; // Set admin flag to true
  delay(2000);      // Pause so user can read the message
}

// Called when admin card is scanned and admin is currently logged in (exit admin mode)
void processAdminExit()
{
  adminGoodbyeMelody(); // Play admin exit melody
  Serial.println("Admin Exited");
  lcd.clear();
  lcd.print("Admin Exited");
  adminFlag = false; // Reset admin flag
  delay(2000);
}

// Called when a member enters (logs in)
void processMemberEntry(int index)
{
  accessGrantedMelody(); // Play access granted melody
  RtcDateTime now = Rtc.GetDateTime();
  String nowString = timeToString(now);

  float temperature = 0, humidity = 0;
  get_temperature_humidity(temperature, humidity);

  Serial.println(users_db[index].lastLogTime);
  users_db[index].logged = true;                   // Mark member as logged in
  users_db[index].lastLogTime = nowString;         // Record last access time as string
  users_db[index].lastLogTimeInt = dateToInt(now); // Record last access time as int (seconds)

  Serial.println("Access Granted");
  lcd.clear();
  lcd.print("Access Granted");
  lcd.setCursor(0, 1);
  lcd.print(nowString);
  delay(1000);

  lcd.clear();
  lcd.print("Welcome");
  Serial.println("Welcome");
  lcd.setCursor(0, 1);
  lcd.print(users_db[index].name);
  Serial.println(users_db[index].name);
  delay(1500);

  lcd.clear();
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
  delay(1500);
}

// Called when a member exits (logs out)
void processMemberExit(int index)
{
  goodbyeMelody(); // Play goodbye melody
  RtcDateTime now = Rtc.GetDateTime();
  String nowString = timeToString(now);
  Serial.println(nowString);

  users_db[index].logged = false; // Mark member as logged out

  float temperature = 0, humidity = 0;
  get_temperature_humidity(temperature, humidity);

  Serial.println("Logging out...");
  lcd.clear();
  lcd.print("Left at ");
  lcd.setCursor(0, 1);
  lcd.print(nowString);
  delay(2000);
  displayExitTime(now, index); // Display the time spent information
}

// --- Main program loop ---

void loop()
{
  // Read card regardless of admin mode to allow toggling admin mode
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    String readUID = convertUID(mfrc522);
    Serial.print("Card UID:");
    Serial.println(readUID);

    if (readUID == ADMIN_UID)
    {
      // Toggle admin mode on/off when admin card is scanned
      if (!adminFlag)
      {
        processAdminCard();
      }
      else
      {
        processAdminExit();
      }
      turnOffLEDs(); // Ensure LEDs off after admin card scan
      // Do NOT return here to allow processing below
    }
    else if (adminFlag)
    {
      // If in admin mode, ignore member card scans but keep menu active
      turnOffLEDs();
      // No return, so menu keeps updating below
    }
    else
    {
      // If not in admin mode, check member access and log entry/exit
      if (isAuthorizedUID(readUID))
      {
        int index = uidToIndex(readUID);
        if (!users_db[index].logged)
        {
          processMemberEntry(index);
        }
        else
        {
          processMemberExit(index);
        }
      }
      else
      {
        // Unauthorized access attempt feedback
        accessDeniedMelody();
        Serial.println("Access Denied");
        lcd.clear();
        lcd.print("Access Denied");
      }
      turnOffLEDs();
      // No return here to allow idle display below if needed
    }
  }

  // If admin mode is active, handle admin menu interaction
  if (adminFlag)
  {
    adminLogged();
    return; // Skip idle display when in admin mode
  }

  // If not admin, show idle message on LCD
  printIdle();
}
