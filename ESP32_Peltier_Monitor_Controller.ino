#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ---------------- PIN DEFINITIONS ----------------
#define DHTPIN 25
#define DHTTYPE DHT22

#define VOLTAGE_PIN 34
#define CURRENT_PIN 35

#define RELAY1 26
#define RELAY2 27

// ---------------- TIMINGS ----------------
const unsigned long ON_TIME  = 60000UL;   // 1 minute
const unsigned long OFF_TIME = 120000UL;  // 2 minutes

// ---------------- ACS712 SETTINGS ----------------
// Change sensitivity according to your module:
// 5A  -> 0.185
// 20A -> 0.100
// 30A -> 0.066
const float ACS_OFFSET = 2.5;
const float ACS_SENSITIVITY = 0.066;

// ---------------- OBJECTS ----------------
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  // LOW-trigger relay OFF
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  dht.begin();

  lcd.setCursor(0,0);
  lcd.print("System Ready");
  delay(2000);
  lcd.clear();
}

void displayValues(float t, float v, float c, const char *status)
{
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(t,1);
  lcd.print(" V:");
  lcd.print(v,1);

  lcd.setCursor(0,1);
  lcd.print("I:");
  lcd.print(c,2);
  lcd.print(" ");
  lcd.print(status);
}

void loop() {

  // -------- Read Temperature --------
  float temperature = dht.readTemperature();

  if (isnan(temperature))
    temperature = 0;

  // -------- Read Voltage --------
  int rawVoltage = analogRead(VOLTAGE_PIN);

  // Adjust multiplier after calibration
  float voltage = (rawVoltage * 3.3 / 4095.0) * (25.0 / 3.3);

  // -------- Read Current --------
  int rawCurrent = analogRead(CURRENT_PIN);

  float sensorVoltage = (rawCurrent * 3.3) / 4095.0;
  float current = (sensorVoltage - ACS_OFFSET) / ACS_SENSITIVITY;

  if (current < 0)
    current = 0;

  // -------- Serial --------
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C  Voltage: ");
  Serial.print(voltage);
  Serial.print(" V  Current: ");
  Serial.print(current);
  Serial.println(" A");

  // -------- FAN ON --------
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);

  displayValues(temperature, voltage, current, "ON");

  delay(ON_TIME);

  // Read sensors again after ON period
  temperature = dht.readTemperature();
  if (isnan(temperature))
    temperature = 0;

  rawVoltage = analogRead(VOLTAGE_PIN);
  voltage = (rawVoltage * 3.3 / 4095.0) * (25.0 / 3.3);

  rawCurrent = analogRead(CURRENT_PIN);
  sensorVoltage = (rawCurrent * 3.3) / 4095.0;
  current = (sensorVoltage - ACS_OFFSET) / ACS_SENSITIVITY;
  if (current < 0)
    current = 0;

  // -------- FAN OFF --------
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  displayValues(temperature, voltage, current, "OFF");

  delay(OFF_TIME);
}
