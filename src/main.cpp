#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

int SwitchPin = 2;
int thermistors[2] = {A0, A11};
int ThermistorPin = A0;

enum STATE {
    HEATING,
    COOLING
};

STATE state[] = { COOLING, COOLING };


float R1 = 10000;

float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void displaySplashScreen()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Project");

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 14);

    display.println("Burning");
    display.println("Abdomen!");

    display.setTextSize(1);
    display.setCursor(0, 51);
    display.println("Classified Top Secret");

    display.display();

    delay(5000);
}


void updateDisplay()
{
    display.clearDisplay();
    /* Draw Header */
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("FRONT");
    display.setCursor(64, 0);
    display.print("BACK");
    //Draw seperator
    int seperatorX = 52;
    display.drawLine(seperatorX,0,seperatorX,64, WHITE);

    for (int i=0; i<sizeof(thermistors); i++) {
        display.setCursor(i * 70, 20); //TODO: Fix flex width
        display.setTextSize(3);
        display.print(readTemperature(i));
        display.display();

        // Draw Status
        switch (state[i])
        {
        case HEATING:
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(64 * i, 51); //TODO: Fix dynamic lenght
            display.println("HEATING");
            break;
        case COOLING:
            // what should be displayed if cooling
            break;

        default:
            break;
        }
    }

}

float readTemperature(int position)
{
    float logR2, R2, T;
    int Vo;
    #ifndef DEBUG:
    Vo = analogRead(ThermistorPin);
    R2 = R1 * (1023.0 / (float)Vo - 1.0);
    logR2 = log(R2);
    T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
    T = T - 273.15;
    T = (T * 9.0)/ 5.0 + 32.0;
    #else
    T = random(100);
    #endif
    return T;
}



float runPositionTemp(int position)
{
    float temp = readTemperature(position);
    Serial.println("Temp: ");
    Serial.print(temp);
    Serial.print(" Position: ");
    Serial.println(position);
    if (temp > 65.0)
    {
        digitalWrite(SwitchPin, LOW);
        Serial.println("COOLING");
        state[position] = COOLING;
    }
    else
    {
        digitalWrite(SwitchPin, HIGH);
        Serial.println("HEATING!");
        state[position] = HEATING;
    }
    return temp;
}

void setup()
{
    Serial.begin(115200);
    pinMode(SwitchPin, OUTPUT);
    // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED
    Wire.begin(5, 4);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    displaySplashScreen();
    randomSeed(42);
}

void loop()
{
    updateDisplay();

    float tempFront = runPositionTemp(0);
    float tempBack = runPositionTemp(1);
    Serial.print("Temperature front: ");
    Serial.println(tempFront);
    Serial.print("Temperature back: ");
    Serial.println(tempBack);
    delay(5000);
}
