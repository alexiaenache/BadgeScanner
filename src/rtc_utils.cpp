#include <utils.hpp>

// Function to calculate the number of days in a month
// Takes the month (1-12) and year as input and returns the number of days in that month.
// Note: Leap years are not considered; February always returns 28 days.
int daysInMonth(int month, int year)
{
    // Check the month and return the corresponding number of days
    switch (month)
    {
    case 1:
        return 31;
    case 2:
        return 28; // February fixed to 28 days (no leap year handling)
    case 3:
        return 31;
    case 4:
        return 30;
    case 5:
        return 31;
    case 6:
        return 30;
    case 7:
        return 31;
    case 8:
        return 31;
    case 9:
        return 30;
    case 10:
        return 31;
    case 11:
        return 30;
    case 12:
        return 31;
    default:
        return 0; // Invalid month input returns 0
    }
}

// Function to convert a RtcDateTime object into an integer representing the total seconds elapsed
// from the start of the year until the given date and time.
// Note: This function does not account for leap years.
int dateToInt(const RtcDateTime &dt)
{
    // Initialize seconds accumulator
    int seconds = 0;

    // Add seconds for all complete months before the current month
    for (int month = 1; month < dt.Month(); month++)
    {
        seconds += daysInMonth(month, dt.Year()) * 24 * 3600; // days * hours * seconds
    }

    // Add seconds for complete days before the current day in the current month
    seconds += (dt.Day() - 1) * 24 * 3600;

    // Add seconds for current hour, minutes, and seconds
    seconds += dt.Hour() * 3600;
    seconds += dt.Minute() * 60;
    seconds += dt.Second();

    return seconds; // Return total seconds elapsed
}

// Function to calculate the percentage change in time spent between last and current values.
// Returns 0.0 if both last and current are zero to avoid division by zero.
// Returns 100.0 if last is zero and current is non-zero (indicating a 100% increase).
float calculateTimeSpentPercentage(int lastTimeSpent, int currentTimeSpent)
{
    // Handle division by zero case when lastTimeSpent is zero
    if (lastTimeSpent == 0)
    {
        if (currentTimeSpent == 0)
        {
            return 0.0; // No change if both are zero
        }
        else
        {
            return 100.0; // Treated as 100% increase
        }
    }

    // Calculate percentage difference normally
    float percentageChange = ((float)(currentTimeSpent - lastTimeSpent) / lastTimeSpent) * 100.0;
    return percentageChange;
}

// Function to format a total number of seconds into a human-readable string in the format:
// "HHh:MMm:SSs", where HH = hours, MM = minutes, SS = seconds.
String formatSpentTime(int totalSeconds)
{
    // Compute hours, minutes, and seconds components
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    // Buffer for formatted time string
    char timeString[20];

    // Format the time string with zero-padding
    snprintf(timeString, sizeof(timeString), "%02dh:%02dm:%02ds", hours, minutes, seconds);
    return String(timeString);
}

// Function to convert an integer number of seconds into a formatted string showing days, hours,
// minutes, and seconds, in the format "DDd:HHh:MMm:SSs".
String intToDate(int seconds)
{
    // Calculate number of days
    int days = seconds / (24 * 3600);
    seconds = seconds % (24 * 3600); // Remaining seconds after removing days

    // Calculate hours
    int hours = seconds / 3600;
    seconds = seconds % 3600; // Remaining seconds after removing hours

    // Calculate minutes
    int minutes = seconds / 60;
    seconds = seconds % 60; // Remaining seconds

    // Buffer for formatted string
    char timeString[20];

    // Format the string with zero-padded day, hour, minute, second components
    snprintf(timeString, sizeof(timeString), "%02dd:%02dh:%02dm:%02ds", days, hours, minutes, seconds);
    return String(timeString);
}

// Function to convert a date string formatted as "YYYY-MM-DD HH:MM:SS" into an integer
// representing total seconds elapsed since the start of the year.
// This assumes valid date string input.
int stringDateToInt(String date)
{
    // Extract components from the date string by substring positions
    int year = date.substring(0, 4).toInt();
    int month = date.substring(5, 7).toInt();
    int day = date.substring(8, 10).toInt();
    int hour = date.substring(11, 13).toInt();
    int minute = date.substring(14, 16).toInt();
    int second = date.substring(17, 19).toInt();

    int seconds = 0; // Accumulate total seconds

    // Add seconds for all complete months before the current month
    for (int i = 1; i < month; i++)
    {
        seconds += daysInMonth(i, year) * 24 * 3600;
    }

    // Add seconds for complete days before current day in the month
    seconds += (day - 1) * 24 * 3600;

    // Add seconds for hour, minute, second
    seconds += hour * 3600;
    seconds += minute * 60;
    seconds += second;

    return seconds; // Return total seconds elapsed
}

// Function to read the current date and time from the RTC module.
// Returns the current RtcDateTime object and prints an error message if the RTC reports invalid time.
RtcDateTime readQuartzTime()
{
    RtcDateTime now = Rtc.GetDateTime();

    // Check if the RTC time is valid
    if (!now.IsValid())
    {
        Serial.println("RTC lost confidence in the DateTime!");
    }

    return now;
}

// Function to set the RTC date and time based on the time the program was compiled.
// Uses compile-time macros __DATE__ and __TIME__.
void setDateTime()
{
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Rtc.SetDateTime(compiled);
}

// Function to display the amount of time spent by a member since last login and the
// percentage change compared to their previous session.
// It shows "Spent time" followed by the formatted spent time and then the percent difference.
void displayExitTime(RtcDateTime now, int index)
{
    // Convert current RTC time to seconds
    int seconds = dateToInt(now);

    // Calculate time spent since last login
    int spentTime = seconds - users_db[index].lastLogTimeInt;

    // Format spent time into a readable string
    String spentTimeString = formatSpentTime(spentTime);

    // Calculate percentage difference from last session's time spent
    float difference = calculateTimeSpentPercentage(users_db[index].lastTimeSpent, spentTime);

    // Update member's last time spent
    users_db[index].lastTimeSpent = spentTime;

    // Display spent time on the LCD
    lcd.clear();
    lcd.print("Spent time ");
    lcd.setCursor(0, 1);
    lcd.print(spentTimeString);
    delay(3000);

    // Display percentage difference from last session on the LCD
    lcd.clear();
    lcd.print("From last time ");
    lcd.setCursor(0, 1);
    lcd.print(difference);

    // Indicate if this is an increase or decrease
    if (difference >= 0)
    {
        lcd.print("% increase");
    }
    else
    {
        lcd.print("% decrease");
    }
    delay(4000);
}

// Function to convert an RtcDateTime object to a formatted date/time string in the format:
// "MM/DD/YYYY HH:MM:SS".
String timeToString(const RtcDateTime &dt)
{
    char datestring[20];

    // Use snprintf_P to format the date/time string safely into the buffer
    snprintf_P(datestring,
               countof(datestring),
               PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(),
               dt.Day(),
               dt.Year(),
               dt.Hour(),
               dt.Minute(),
               dt.Second());

    return String(datestring);
}
