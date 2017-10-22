/*
 * Usunąłem komendę state
 */
#include <Timer.h>
#include <Adafruit_NeoPixel.h>
#define PIN 13
#define NUM_PIXELS 3
#define ITERATIONS 1020

Timer t;
bool pulsing[3] = {false, false, false};  //czy dioda jest w stanie pulsowania
bool lastPulseState[3] = {false, false, false}; //czy ostatnio dioda była włączono czy wyłączona
String diodeColor[3] = {"black","black","black"}; //kolor na jaki ma świecić dioda podczas pulsowania

int redValue[3] = {0, 0, 0};
int greenValue[3] = {0, 0, 0};
int blueValue[3] = {0, 0, 0};

int prevRedValue[3] = {0, 0, 0};
int prevGreenValue[3] = {0, 0, 0};
int prevBlueValue[3] = {0, 0, 0};

//Colors in GRB format
int black[3] = { 0, 0, 0 };
int white[3] = { 255, 255, 255 };
int red[3] = { 0, 255, 0 };
int green[3] = { 255, 0, 0 };
int blue[3] = { 0, 0, 255 };
int yellow[3] = { 255, 255, 0 };
int purple[3] = { 0, 255, 255 };
int ltblue[3] = { 255, 0, 255 };
int orange[3] = { 75, 255, 0 };

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  pixels.begin();
  pixels.setBrightness(100);
  pixels.show();
  t.every(1000, pulse);
  Serial.begin(9600);
}

void loop() {
  t.update();
  if (Serial.available())
  {
    int pixel = Serial.readStringUntil(':').toInt();
    String color = Serial.readStringUntil(':');    
    if (color == "pulse")
    {
      if (pulsing[pixel] == false)
      {
        pulsing[pixel] = true;
      }
      else
      {
        pulsing[pixel] = false;
      }
    }
    else 
    {
      diodeColor[pixel] = color;  //przypisuje aktualny kolor danej diody
      int time = Serial.readStringUntil(';').toInt();
      int *newColor;
      newColor = getColor(color); //pobiera kolor w formacie GRB na podstawie otrzymanego symbolu koloru
      diodeColor[pixel] = color;  //zapamiętanie koloru na wypadek pulsowania
      fade(pixel, newColor, time);
    }

  }
}

//funkcja obsługująca pulsowanie diod
void pulse()
{
  //iteruję po wszystkich diodach
  for (int i=0; i < NUM_PIXELS; i++)
  {
    //jeśli pulsowanie wyłączone to sprawdź czy dioda przypadkiem nie została wyłączona
    if (pulsing[i] == false)
    {
      if (lastPulseState[i] == false)
      {
        String colorSymbol = diodeColor[i];
        int *color;
        color = getColor(colorSymbol);
        fade(i, color, 500);
        lastPulseState[i] = true;
      }
      continue;
    }

    //pobranie koloru diody
    String colorSymbol = diodeColor[i];
    int *color;
    color = getColor(colorSymbol);

    //jeśli jest wyłączona
    if (lastPulseState[i] == false)
    {
      fade(i, color, 500);
      lastPulseState[i] = true;
    }
    //jeśli jest włączona
    else
    {
      int darkColor[3];
      //przyciemnij aktualny kolor
      for(int i=0; i<3; i++)
      {
        darkColor[i] = color[i] / 4;
      }
      fade(i, darkColor, 1000);
      lastPulseState[i] = false;
    }
  }
}

//funkcja obliczająca zmianę wartości koloru w pojedynczej iteracji
int calculateStep(int prevValue, int newValue)
{
  int step = newValue - prevValue;
  if (step)
  {
    step = ITERATIONS / step;
  }
  return step;
}

//funkcja obliczająca kolejną wartość danego koloru
int calculateValue(int step, int value, int i)
{
  if ((step) && i%step == 0)
  {
    if (step > 0)
    {
      value += 1;
    }
    else if (step < 0)
    {
      value -= 1;
    }
  }

  if (value > 255)
  {
    value = 255;
  }
  else if (value < 0)
  {
    value = 0;
  }
  return value;
}

//funkcja zmieniająca kolor podanej diody
void fade(int pixel, int color[3], int time)
{
  int G = color[0];
  int R = color[1];
  int B = color[2];

  int stepG = calculateStep(prevGreenValue[pixel], G);
  int stepR = calculateStep(prevRedValue[pixel], R);
  int stepB = calculateStep(prevBlueValue[pixel], B);

  for (int i = 0; i <= ITERATIONS; i++)
  {
    redValue[pixel] = calculateValue(stepR, redValue[pixel], i);
    greenValue[pixel] = calculateValue(stepG, greenValue[pixel], i);
    blueValue[pixel] = calculateValue(stepB, blueValue[pixel], i);

    pixels.setPixelColor(pixel, pixels.Color(greenValue[pixel], redValue[pixel], blueValue[pixel]));
    pixels.show();

    delay(time/ITERATIONS);
  }

  prevRedValue[pixel] = redValue[pixel];
  prevGreenValue[pixel] = greenValue[pixel];
  prevBlueValue[pixel] = blueValue[pixel];
}

//funkcja zwracająca zapis koloru w formacie GRB
int *getColor(String color)
{
  if (color == "R")
  {
    return red;
  }
  else if (color == "G")
  {
    return green;
  }
  else if (color == "B")
  {
    return blue;
  }
  else if (color == "W")
  {
    return white;
  }
  else if (color == "Y")
  {
    return yellow;
  }
  else if (color == "P")
  {
    return purple;
  }
  else if (color == "L")
  {
    return ltblue;
  }
  else if (color == "O")
  {
    return orange;
  }
  else
  {
    return black;
  }
}

