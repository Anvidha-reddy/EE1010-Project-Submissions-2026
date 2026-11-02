// =====================================================================
// PT100 Digital Thermometer - Arduino Uno + 16x2 LCD
// 17-point linear least-squares calibration (Updated Readings)
//
// T_cal = CAL_A * T_raw + CAL_B
// =====================================================================

#include <LiquidCrystal.h>
#include <math.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// ---------- Sensor settings ----------
const float R_REF = 178.0;

// ---------- Calibration data ----------
// X_OURS = raw PT100 temperature in Celsius
// Y_ACTUAL = reference thermometer temperature in Celsius

const int N = 17;

const float X_OURS[N] = {
  56.05, 60.63, 61.92, 64.55, 67.18, 
  69.69, 73.90, 76.57, 73.90, 71.18, 
  76.84, 77.93, 78.02, 78.27, 79.31, 
  79.31, 79.31
};

const float Y_ACTUAL[N] = {
  55.3, 63.9, 64.3, 71.9, 72.9, 
  74.8, 82.9, 84.3, 82.9, 80.8, 
  85.5, 91.2, 92.8, 93.2, 94.2, 
  95.1, 96.8
};

float CAL_A = 1.0;
float CAL_B = 0.0;

// ---------- PT100 constants (IEC 60751) ----------
const float R0 = 100.0;
const float A  = 3.9083e-3;
const float B  = -5.775e-7;

// ---------- Settings ----------
const int SAMPLES = 256;
const float SMOOTH = 0.85;

float tempSmooth = NAN;

// ---------- Calculate least-squares coefficients ----------
void leastSquares()
{
  float sx = 0.0;
  float sy = 0.0;

  for (int i = 0; i < N; i++)
  {
    sx += X_OURS[i];
    sy += Y_ACTUAL[i];
  }

  float xm = sx / N;
  float ym = sy / N;

  float sxy = 0.0;
  float sxx = 0.0;

  for (int i = 0; i < N; i++)
  {
    sxy += (X_OURS[i] - xm) * (Y_ACTUAL[i] - ym);
    sxx += (X_OURS[i] - xm) * (X_OURS[i] - xm);
  }

  if (sxx > 0.0)
  {
    CAL_A = sxy / sxx;
    CAL_B = ym - CAL_A * xm;
  }

  Serial.println();
  Serial.println("===== CALIBRATION RESULTS =====");

  Serial.print("Calibration points: ");
  Serial.println(N);

  Serial.print("CAL_A = ");
  Serial.println(CAL_A, 6);

  Serial.print("CAL_B = ");
  Serial.println(CAL_B, 6);

  Serial.print("Equation: T_cal = ");
  Serial.print(CAL_A, 6);
  Serial.print(" * T_raw + ");
  Serial.println(CAL_B, 6);

  Serial.println("===============================");
}

// ---------- Read averaged ADC ----------
float readADC()
{
  long sum = 0;

  for (int i = 0; i < SAMPLES; i++)
  {
    sum += analogRead(A0);
    delayMicroseconds(300);
  }

  return sum / (float)SAMPLES;
}

// ---------- Show errors ----------
void showError(const char *line1, const char *label, float value)
{
  Serial.print("ERROR: ");
  Serial.print(line1);
  Serial.print(" ");
  Serial.print(label);
  Serial.println(value, 1);

  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print(label);
  lcd.print(value, 1);
  lcd.print("          ");

  tempSmooth = NAN;
  delay(500);
}

// ---------- Setup ----------
void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);

  lcd.clear();
  lcd.print("PT100 Digital");
  lcd.setCursor(0, 1);
  lcd.print("Calibrating...");
  delay(1500);

  leastSquares();

  lcd.clear();
  lcd.print("Calibration OK");
  delay(1000);
  lcd.clear();

  Serial.println();
  Serial.println("Starting PT100...");
  Serial.println("ADC | Resistance | Raw Temp | Cal Temp | Display");
}

// ---------- Main loop ----------
void loop()
{
  float adc = readADC();

  if (adc < 10.0)
  {
    showError("PT100 shorted?  ", "ADC: ", adc);
    return;
  }

  if (adc > 1010.0)
  {
    showError("PT100 open?     ", "ADC: ", adc);
    return;
  }

  // Calculate resistance
  float rpt = R_REF * adc / (1023.0 - adc);

  if (rpt < 60.0 || rpt > 400.0)
  {
    showError("R out of range  ", "R: ", rpt);
    return;
  }

  // Convert resistance to raw temperature
  float x = rpt / R0 - 1.0;
  float disc = A * A + 4.0 * B * x;

  if (disc < 0.0)
  {
    showError("Calc error      ", "R: ", rpt);
    return;
  }

  float tempRaw = (2.0 * x) / (A + sqrt(disc));

  // Apply least-squares calibration
  float tempC = CAL_A * tempRaw + CAL_B;

  // Smooth output
  if (isnan(tempSmooth))
  {
    tempSmooth = tempC;
  }
  else
  {
    tempSmooth =
        SMOOTH * tempSmooth + (1.0 - SMOOTH) * tempC;
  }

  // Serial Monitor output
  Serial.print("ADC=");
  Serial.print(adc, 1);

  Serial.print("  R=");
  Serial.print(rpt, 2);
  Serial.print(" ohm");

  Serial.print("  T_raw=");
  Serial.print(tempRaw, 2);

  Serial.print("  T_cal=");
  Serial.print(tempC, 2);

  Serial.print("  T=");
  Serial.println(tempSmooth, 2);

  // LCD: temperature
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(tempSmooth, 1);
  lcd.print((char)223);
  lcd.print("C     ");

  // LCD: resistance
  lcd.setCursor(0, 1);
  lcd.print("R: ");
  lcd.print(rpt, 1);
  lcd.print(" ohm     ");

  delay(300);
}
