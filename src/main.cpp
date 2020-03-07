#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))
#define DEBUG
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

int switchPins[2] = {16, 26};
int thermistors[2] = {4, 0};
float temperatures[2];
float readTemperature(int position);
void globalReadTemperatures();



enum STATE {
    COOLING,
    HEATING
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

    for (int i=0; i<ARRAY_SIZE(thermistors); i++) {
        display.setCursor(i * 70, 20); //TODO: Fix flex width
        display.setTextSize(3);
        display.print((int)temperatures[i]);

        #ifdef DEBUG
        Serial.print("[");Serial.print(i);Serial.print("] state is ");
        Serial.println(state[i]);
        #endif

        // Draw Status
        switch (state[i])
        {
        case HEATING:
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(64 * i, 51); //TODO: Fix dynamic lenght
            display.print("HEATING");
            break;
        case COOLING:
            // what should be displayed if cooling
            break;

        default:
            break;
        }
    }
    // Draw Display.
    display.display();
}


/* Steinhard Calculation */
float readTemperature(int position)
{
    float logR2, R2, T;
    int Vo;

    Vo = analogRead(thermistors[position]);
    R2 = R1 * (4095.0 / (float)Vo - 1.0);
    logR2 = log(R2);
    T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
    T = T - 273.15;
    // T = (T * 9.0)/ 5.0 + 32.0;

    #ifdef DEBUG
    Serial.print("[");Serial.print(position);
    Serial.print("] Analog read from GPIO ");
    Serial.print(thermistors[position]);
    Serial.print(": "); Serial.println(Vo);
    #endif

    return T;
}



float runPositionTemp(int position)
{
    float temp = temperatures[position];
    Serial.print("[");
    Serial.print(position);Serial.print("] Temp: ");
    Serial.println(temp);

    if (temp > 65.0)
    {
        digitalWrite(switchPins[position], LOW);
        Serial.println("COOLING");
        state[position] = COOLING;
    }
    else
    {
        digitalWrite(switchPins[position], HIGH);
        Serial.println("HEATING!");
        state[position] = HEATING;
    }
    return temp;
}

void setup()
{
    Serial.begin(115200);
    pinMode(switchPins[0], OUTPUT);
    pinMode(switchPins[1], OUTPUT);
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

void globalReadTemperatures(){
    for (int i=0; i < ARRAY_SIZE(temperatures); i++) {
        Serial.print("int i in global read: ");
        Serial.println(i);
        temperatures[i] = readTemperature(i);
    }
}

void loop()
{
    globalReadTemperatures();  // read temperatures here
    updateDisplay();
    // do a control cycle per sensor.
    runPositionTemp(0);
    runPositionTemp(1);
    delay(5000);
}
