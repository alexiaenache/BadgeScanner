#include <utils.hpp>

// Function to print a given message on the LCD screen.
// Supports printing messages longer than 16 characters by splitting across two rows.
void printStringOnLCD(const char *message)
{
    // Number of characters the LCD can display per row (16 columns)
    int lcdWidth = 16;

    // Clear the LCD before printing a new message
    lcd.clear();

    // Print the first row of the message, character by character,
    // stopping either at the end of the message or the LCD width
    for (int i = 0; i < lcdWidth && i < strlen(message); i++)
    {
        lcd.setCursor(i, 0); // Set cursor to position i on the first row (row 0)
        lcd.print(message[i]);
    }

    // Check if the message length exceeds one row (more than lcdWidth chars)
    // If yes, print the remaining characters on the second row
    if (strlen(message) > lcdWidth)
    {
        for (int i = 0; i < lcdWidth && (lcdWidth + i) < strlen(message); i++)
        {
            lcd.setCursor(i, 1); // Set cursor to position i on the second row (row 1)
            lcd.print(message[lcdWidth + i]);
        }
    }
}

// Overloaded version of printStringOnLCD that accepts Arduino String objects.
// Converts the String to a C-style string and calls the above function.
void printStringOnLCD(String message)
{
    printStringOnLCD(message.c_str());
}

// Function to display the total number of registered members on the LCD.
// The function then waits in a loop until the joystick is pushed up to exit this screen.
void showTotalNumber()
{
    lcd.clear();                  // Clear the LCD before printing
    lcd.print("Total Members: "); // Print label on first line
    lcd.setCursor(0, 1);          // Move cursor to the start of the second line
    lcd.print(uidCount);          // Print the total number of members registered

    // Wait here until user pushes joystick up to exit this screen
    while (true)
    {
        int joyY = analogRead(JOYSTICK_URX_PIN); // Read vertical joystick position

        // Check if joystick is pushed beyond upper threshold (upward direction)
        if (joyY > UPPER_JOYSTICK_THRESHOLD)
        {
            lcd.clear();             // Clear LCD before exiting
            lcd.print("Exiting..."); // Inform user we are exiting the screen
            delay(1000);             // Wait a moment to allow user to see the message
            return;                  // Exit the function and return to previous menu/state
        }
        delay(100); // Small delay to reduce CPU usage during wait loop
    }
}

// Function to display an idle message on the LCD when the system is waiting for user action.
// Alternates every 4 seconds between prompting to scan a card and showing the count of logged-in members.
void printIdle()
{
    // Static variables retain their values between calls
    static unsigned long lastToggleTime = 0; // Timestamp when the message last toggled
    static bool showScanMessage = true;      // Flag indicating which message to display

    // Check if 4 seconds have passed since last toggle
    if (millis() - lastToggleTime >= 4000)
    {
        lastToggleTime = millis();          // Update the last toggle time
        showScanMessage = !showScanMessage; // Switch message flag

        lcd.clear(); // Clear the LCD before printing new message

        if (showScanMessage)
        {
            lcd.print("Scan Card"); // Prompt user to scan a card
        }
        else
        {
            // Count the number of members currently logged in
            int loggedMembersCount = 0;
            for (int i = 0; i < uidCount; i++)
            {
                if (users_db[i].logged)
                {
                    loggedMembersCount++;
                }
            }

            // Display the number of logged-in members
            lcd.print("Members in: ");
            lcd.print(loggedMembersCount);
        }
    }
}

// Function to display temperature and humidity readings on the LCD.
// Shows temperature in degrees Celsius on the first row and humidity percentage on the second row.
void print_temperature_humidity(float temperature, float humidity)
{
    lcd.clear(); // Clear the display before printing new values

    // Print temperature label and value on the first line
    lcd.print("Temperature: ");
    lcd.print(temperature);
    lcd.print("C");

    // Move to second line to print humidity label and value
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");
}
