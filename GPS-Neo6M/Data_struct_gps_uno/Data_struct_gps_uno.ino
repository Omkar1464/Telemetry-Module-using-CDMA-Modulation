#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

// -----------------------------
// Struct
// -----------------------------
struct SystemPayload {
    struct {
        float latitude;
        float longitude;
        float altitude;
        uint8_t fix_quality;
    } gps_data;
};

SystemPayload payload;

// -----------------------------
// Convert NMEA → Decimal
// -----------------------------
float nmea_to_decimal(const char *nmea_str, char dir)
{
    if (nmea_str == NULL || strlen(nmea_str) == 0)
        return 0.0f;

    float nmea = atof(nmea_str);

    int degrees = (int)(nmea / 100);
    float minutes = nmea - (degrees * 100);

    float decimal = degrees + (minutes / 60.0f);

    if (dir == 'S' || dir == 'W')
        decimal = -decimal;

    return decimal;
}

// -----------------------------
// Parse GGA
// -----------------------------
int parse_gga(char *nmea, SystemPayload *payload)
{
    if (strncmp(nmea, "$GNGGA", 6) != 0 &&
        strncmp(nmea, "$GPGGA", 6) != 0)
    {
        return 0;
    }

    char *token;
    int field_index = 0;

    char latitude_str[20] = {0};
    char longitude_str[20] = {0};
    char lat_dir = 'N';
    char lon_dir = 'E';

    token = strtok(nmea, ",");

    while (token != NULL)
    {
        switch (field_index)
        {
        case 2:
            strcpy(latitude_str, token);
            break;

        case 3:
            lat_dir = token[0];
            break;

        case 4:
            strcpy(longitude_str, token);
            break;

        case 5:
            lon_dir = token[0];
            break;

        case 6:
            payload->gps_data.fix_quality = (uint8_t)atoi(token);
            break;

        case 9:
            payload->gps_data.altitude = atof(token);
            break;
        }

        token = strtok(NULL, ",");
        field_index++;
    }

    payload->gps_data.latitude = nmea_to_decimal(latitude_str, lat_dir);
    payload->gps_data.longitude = nmea_to_decimal(longitude_str, lon_dir);

    return 1;
}

// -----------------------------
// GPS Line Buffer
// -----------------------------
#define BUFFER_SIZE 120
char gpsBuffer[BUFFER_SIZE];
uint8_t indexBuffer = 0;

// -----------------------------
// Setup
// -----------------------------
void setup()
{
    Serial.begin(115200);   // Debug
    Serial1.begin(9600);    // GPS (UNO → use SoftwareSerial instead)

    Serial.println("GPS Parsing Started...");
}

// -----------------------------
// Loop
// -----------------------------
void loop()
{
    while (Serial1.available())
    {
        char c = Serial1.read();

        if (c == '\n')  // End of line
        {
            gpsBuffer[indexBuffer] = '\0';

            if (parse_gga(gpsBuffer, &payload))
            {
                if (payload.gps_data.fix_quality > 0)
                {
                    Serial.print("Lat: ");
                    Serial.println(payload.gps_data.latitude, 6);

                    Serial.print("Lon: ");
                    Serial.println(payload.gps_data.longitude, 6);

                    Serial.print("Alt: ");
                    Serial.println(payload.gps_data.altitude);

                    Serial.print("Fix: ");
                    Serial.println(payload.gps_data.fix_quality);

                    Serial.println("----------------------");
                }
                else
                {
                    Serial.println("No Fix");
                }
            }

            indexBuffer = 0; // reset buffer
        }
        else
        {
            if (indexBuffer < BUFFER_SIZE - 1)
            {
                gpsBuffer[indexBuffer++] = c;
            }
        }
    }
}