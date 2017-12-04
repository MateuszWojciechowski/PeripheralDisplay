#include <Timer.h>
#include <Adafruit_NeoPixel.h>
#define PIN 13
#define NUM_PIXELS 3
#define ITERATIONS 1020

Timer t;
int timerID;  //przechowuje ID timera
bool pulsing[3] = {false, false, false};  //czy dioda jest w stanie pulsowania
bool lastPulseState = true; //czy ostatnio dioda była włączono czy wyłączona
String diodeColor[3] = {"black","black","black"}; //kolor na jaki ma świecić dioda podczas pulsowania
bool keepalive = false; //jeśli true to otrzymano wiadomość keepalive

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
  timerID = t.every(1000, pulse);
  t.every(300000, alive);
  Serial.begin(9600);
}

void loop() {
  t.update(); //uruchamia Timer
  if (Serial.available())
  {
    /*
      Format komend
      1. Zmiana koloru
        nr_diody:kolor:czas;
      2. Pulsowanie
        nr_diody:pulse:okres;
      3. Keepalive
    */
    if(Serial.find("keepalive;"))
    {
      keepalive = true;
      Serial.readStringUntil(';');
      Serial.println("KEEPALIVE");
    }
    int pixel = Serial.readStringUntil(':').toInt();
    String color = Serial.readStringUntil(':');  
    int time = Serial.readStringUntil(';').toInt();
    if (color == "pulse")
    {
      pulsing[pixel] = !pulsing[pixel];
      //zatrzymany jest poprzedni timer i uruchomiony kolejny z nowym okresem pulsowania
      t.stop(timerID);
      timerID = t.every(time, pulse);
      t.update();
    }
    else 
    {
      int *newColor;
      newColor = getColor(color); //pobiera kolor w formacie GRB na podstawie otrzymanego symbolu koloru
      diodeColor[pixel] = color;  //zapamiętanie koloru na wypadek pulsowania
      fade(pixel, newColor, time);
    }
  }
}

//funkcja sprawdzająca czy telefon jest w zasięgu
void alive()
{
  if(!keepalive)
  {
    //wykonuje się jak telefon zniknął z zasięgu
    pixels.clear();
    pixels.show();
  }
  keepalive = false;
}
//funkcja obsługująca pulsowanie diod
void pulse()
{
  pulsingFade(500);
}

//funkcja obsługująca zsynchronizowane pulsowanie
void pulsingFade(int time) {
  //pobranie kolorow diod
  int G[NUM_PIXELS], R[NUM_PIXELS], B[NUM_PIXELS];
  for (int i=0; i < NUM_PIXELS; i++) {
    int *color = getColor(diodeColor[i]);
    G[i] = color[0];
    R[i] = color[1];
    B[i] = color[2];

    //jeśli dioda jest w stanie pulsowania i ostatnio była jasna to przyciemnij kolory
    if (pulsing[i]) {
      if (lastPulseState) {
        G[i] /= 4;
        R[i] /= 4;
        B[i] /= 4;
      }
    }
  }
  lastPulseState = !lastPulseState;

  //kroki poszczeglnych kolorow w poszczeglnych diodach
  int stepG[NUM_PIXELS], stepR[NUM_PIXELS], stepB[NUM_PIXELS];
  for (int i=0; i < NUM_PIXELS; i++) {
    stepG[i] = calculateStep(prevGreenValue[i], G[i]);
    stepR[i] = calculateStep(prevRedValue[i], R[i]);
    stepB[i] = calculateStep(prevBlueValue[i], B[i]);
  }

  //iteracje
  for (int j=0; j < ITERATIONS; j++) {
    //iteracje po diodach
    for (int i=0; i < NUM_PIXELS; i++) {
      greenValue[i] = calculateValue(stepG[i], greenValue[i], j);
      redValue[i] = calculateValue(stepR[i], redValue[i], j);
      blueValue[i] = calculateValue(stepB[i], blueValue[i], j);

      pixels.setPixelColor(i, pixels.Color(greenValue[i], redValue[i], blueValue[i]));
    }
    pixels.show();
    delay(time/ITERATIONS);
  }

  //ustawienie zmiennych prev
  for (int i=0; i < NUM_PIXELS; i++) {
    prevGreenValue[i] = greenValue[i];
    prevRedValue[i] = redValue[i];
    prevBlueValue[i] = blueValue[i];
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