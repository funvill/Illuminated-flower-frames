/**
 * Illuminated flower - Stainglass window 
 * Written by: Steven Smethurst 
 * More info https://github.com/funvill/Illuminated-flower-frames
 */
#include <Arduino.h>

const char APP_NAME[] = "Illuminated-flower-frames";
const uint16_t APP_VERSION_MAJOR = 0;
const uint16_t APP_VERSION_MINOR = 0;
const uint16_t APP_VERSION_PATCH = 2;

#define FASTLED_ALLOW_INTERRUPTS 0
#include <Arduino.h>
#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN D4
#define LED_TYPE WS2811
#define COLOR_ORDER RBG
#define NUM_LEDS 126
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 96
#define FRAMES_PER_SECOND 120
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// Settings
const uint16_t SETTING_TIME_FOR_EACH_PATTERN = 10;

// Global
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns

// Segments
uint8_t PATTERNS_FLOWER_COUNT = 7 ; 
uint8_t PATTERNS_FLOWER_PETTLE_COUNT = 13 ; 
uint8_t PATTERNS_FLOWER[] = {

// /- Center 
// | 
// 0     1    2    3    4    5    6    7    8    9   10   11   12 
   5,    4,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  // 0
   22,  21,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  // 1
   50,  41,  42,  43,  44,  45,  46,  47,  48,  49,  51,  52,  53,  // 2
   68,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  // 3
  102,  79,  80,  81,  82,  83,  84,  85, 100, 101, 103, 104, 105,  // 4   
  125, 112, 113, 114, 115, 116, 117, 118, 120, 121, 122, 123, 124,  // 5
   99,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97   // 6
};

uint8_t PATTERNS_CORNERS[] = { 0, 37, 74, 109};
uint8_t PATTERNS_BACKGROUND[] = {0, 1, 2, 3, 18, 19, 20, 34, 35, 36, 37, 39, 40, 55, 70, 71, 72, 73, 74, 75, 76, 77, 78, 106, 107, 108, 109, 110, 111, 119}; 

void PrintStartUpInfo()
{
  Serial.print("\nName: ");
  Serial.print(APP_NAME);
  Serial.print(", Version: ");
  Serial.print(APP_VERSION_MAJOR, DEC);
  Serial.print(".");
  Serial.print(APP_VERSION_MINOR, DEC);
  Serial.print(".");
  Serial.print(APP_VERSION_PATCH, DEC);

  // Serial.print("\nPattern Count: ");
  // Serial.print(ARRAY_SIZE(gPatterns));
  // Serial.print("\n");
}

// Helper functions
// ----------------------------------------------------------------------------

void PatternRotateThoughSubSections(unsigned long &nextUpdate, uint16_t &segmentOffset, const uint8_t *pattern, uint32_t patternSize, uint32_t subsectionSize, long speed, int8_t direction, uint8_t hue)
{
  // Only update the segments on a timer.
  if (nextUpdate < millis())
  {
    nextUpdate = millis() + speed;

    // Get the amount of sub sections for the entire pattern
    uint16_t COUNT_SUBSECTIONS = patternSize / subsectionSize;

    // increase the offset
    segmentOffset += direction;
    uint16_t segmentStart = (segmentOffset % COUNT_SUBSECTIONS);

    // Loop thought the LEDS in the segment and set them to the color.
    for (uint32_t offsetLED = segmentStart * subsectionSize; offsetLED < (segmentStart * subsectionSize) + subsectionSize; offsetLED++)
    {
      leds[pattern[offsetLED]] = CHSV(hue, 200, 255);
    }
  }
}

void PatternSetColor(const uint8_t *pattern, uint32_t patternSize, uint8_t hue)
{
  for (uint32_t offsetLED = 0; offsetLED < patternSize; offsetLED++)
  {
    leds[pattern[offsetLED]] = CHSV(hue, 200, 255);
  }
}

void PatternSegmentRainbow(const uint8_t *pattern, uint32_t patternSize, uint16_t segmentsSize, uint8_t hue)
{
  uint16_t segmentsCount = patternSize / segmentsSize;
  for (uint16_t segmentOffset = 0; segmentOffset < segmentsCount; segmentOffset++)
  {
    PatternSetColor(pattern + (segmentOffset * segmentsSize), segmentsSize, hue + (255 / segmentsCount) * segmentOffset);
  }
}


// ----------------------------------

void TestPattern()
{
  Serial.print("FYI: Pattern: TestPattern\n");

  Serial.print("Red\n");
  fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));
  FastLED.show();
  delay(1000);
  Serial.print("Blue\n");
  fill_solid(leds, NUM_LEDS, CRGB(0, 255, 0));
  FastLED.show();
  delay(1000);
  Serial.print("Green\n");
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
  FastLED.show();
  delay(1000);
  Serial.print("Rainbow\n");
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
  FastLED.show();
  delay(1000);
  
}





// Default patterns
// ----------------------------------
// rainbow, rainbowWithGlitter, confetti, sinelon, bpm, juggle,
void addGlitter(fract8 chanceOfGlitter)
{
  if (random8() < chanceOfGlitter)
  {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}
void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}
void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++)
  { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle()
{
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++)
  {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}



// Custom patterns.
// ----------------------------------------------------------------------------



void rotatingFlowers() {

  fadeToBlackBy(leds, NUM_LEDS, 3);

  long speed = 80;
  int8_t direction = 1; 

  static unsigned long nextUpdate = 0 ;
  static uint16_t segmentOffset = 0 ; 
  PatternRotateThoughSubSections( nextUpdate, segmentOffset, PATTERNS_FLOWER, ARRAY_SIZE(PATTERNS_FLOWER)-PATTERNS_FLOWER_PETTLE_COUNT, PATTERNS_FLOWER_PETTLE_COUNT, speed, direction, gHue); 
  
  static unsigned long nextUpdateCenter = 0 ;
  static uint16_t segmentOffsetCenter = 0 ; 
  PatternRotateThoughSubSections( nextUpdateCenter, segmentOffsetCenter, PATTERNS_FLOWER + (PATTERNS_FLOWER_PETTLE_COUNT * 6), PATTERNS_FLOWER_PETTLE_COUNT, 1, speed / 2.5, direction * -1, gHue +15 ); 
}

void rotatingPettles(long speed, int8_t direction) {

  fadeToBlackBy(leds, NUM_LEDS, speed/7);

  static unsigned long nextUpdate[7] = {0,0,0,0,0,0,0}; 
  static uint16_t segmentOffset[7] = {0,0,0,0,0,0,0}; 
  for( uint8_t flowerOffset = 0 ; flowerOffset < PATTERNS_FLOWER_COUNT ; flowerOffset++ ) {  
    PatternRotateThoughSubSections( nextUpdate[flowerOffset], segmentOffset[flowerOffset], PATTERNS_FLOWER + (PATTERNS_FLOWER_PETTLE_COUNT * flowerOffset), PATTERNS_FLOWER_PETTLE_COUNT, 1, speed, direction, gHue + ((255/PATTERNS_FLOWER_COUNT) * flowerOffset)); 
  }  
}

void rotatingPettlesForwards() {
  rotatingPettles(30, 1) ;
}
void rotatingPettlesBackwards() {
  rotatingPettles(30, -1) ;
}



void spirialOut() {

  // Only update the segments on a timer.
  static unsigned long nextUpdate = 0 ;
  static uint8_t hue = 0 ;
  if (nextUpdate < millis())
  {
    nextUpdate = millis() + 10;
    hue += 1; 
  }


  // Center flower 
  PatternSetColor(PATTERNS_FLOWER + (PATTERNS_FLOWER_PETTLE_COUNT*6), PATTERNS_FLOWER_PETTLE_COUNT, hue + ((255/3)*1));

  // Outside flowers 
  for( uint8_t offset = 0 ; offset < 6 ; offset++ ) {
    PatternSetColor(PATTERNS_FLOWER + (PATTERNS_FLOWER_PETTLE_COUNT*offset), PATTERNS_FLOWER_PETTLE_COUNT, hue + ((255/3)*2));
  }

  // Background
  PatternSetColor(PATTERNS_BACKGROUND, ARRAY_SIZE(PATTERNS_BACKGROUND), hue + ((255/3)*3));

  // Center dot
  leds[99] += CHSV(hue + ((255/3)*0), 255, 192);

}

void DifferentColorFlowers() {
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  for( uint8_t flowerOffset = 0 ; flowerOffset < PATTERNS_FLOWER_COUNT ; flowerOffset++ ) {
    // Set the pettles 
    PatternSetColor(PATTERNS_FLOWER + (PATTERNS_FLOWER_PETTLE_COUNT*flowerOffset), PATTERNS_FLOWER_PETTLE_COUNT, gHue + ((255/PATTERNS_FLOWER_COUNT)*flowerOffset));
    // Set the center 
    uint8_t hue = gHue + (255/(PATTERNS_FLOWER_COUNT+1)); 
    leds[ PATTERNS_FLOWER[ (flowerOffset * PATTERNS_FLOWER_PETTLE_COUNT) + 0 ] ] = CHSV(hue, 200, 255); 
  }
}

void Cross() {
  
  const long delay = 100; 
  // fadeToBlackBy(leds, NUM_LEDS, 10 );
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));

  static unsigned long nextUpdate = 0 ;
  static uint16_t segmentOffset = 0 ;
  static uint16_t segmentStart = 0 ; 

  for( uint8_t flowerOffset = 0 ; flowerOffset < PATTERNS_FLOWER_COUNT ; flowerOffset++ ) {
  
    // Only update the segments on a timer.
    if (nextUpdate < millis())
    {
      nextUpdate = millis() + delay;

      // increase the offset
      segmentOffset += 1;
      // Ensure that we don't overflow.
      segmentStart = (segmentOffset % 3); 
    }

    leds[ PATTERNS_FLOWER[ (flowerOffset * PATTERNS_FLOWER_PETTLE_COUNT) +  1 + segmentStart ] ] = CHSV(gHue, 200, 255);
    leds[ PATTERNS_FLOWER[ (flowerOffset * PATTERNS_FLOWER_PETTLE_COUNT) +  4 + segmentStart ] ] = CHSV(gHue, 200, 255);
    leds[ PATTERNS_FLOWER[ (flowerOffset * PATTERNS_FLOWER_PETTLE_COUNT) +  7 + segmentStart ] ] = CHSV(gHue, 200, 255);
    leds[ PATTERNS_FLOWER[ (flowerOffset * PATTERNS_FLOWER_PETTLE_COUNT) + 10 + segmentStart ] ] = CHSV(gHue, 200, 255);

  }
}

/*
void Groups()
{
  PatternSetColor(STARS, ARRAY_SIZE(STARS), gHue + (255 / 4) * 0);
  PatternSetColor(EDGES, ARRAY_SIZE(EDGES), gHue + (255 / 4) * 1);
  PatternSetColor(HEXES, ARRAY_SIZE(HEXES), gHue + (255 / 4) * 2);
  PatternSetColor(CENTERS, ARRAY_SIZE(CENTERS), gHue + (255 / 4) * 3);
}

void Triangles()
{
  fadeToBlackBy(leds, NUM_LEDS, 4);
  static unsigned long nextUpdate = 0;
  static uint16_t segmentOffset = 0;
  PatternRotateThoughSubSections(nextUpdate, segmentOffset, PATTERN_TRIANGLES, ARRAY_SIZE(PATTERN_TRIANGLES), 5, 100, 1, gHue);
}

void Clock()
{
  fadeToBlackBy(leds, NUM_LEDS, 4);

  static unsigned long edgesNextUpdate = 0;
  static uint16_t edgesSegmentOffset = 0;
  PatternRotateThoughSubSections(edgesNextUpdate, edgesSegmentOffset, EDGES, ARRAY_SIZE(EDGES), 1, 150, 1, gHue);

  static unsigned long starsNextUpdate = 0;
  static uint16_t starsSegmentOffset = 0;
  PatternRotateThoughSubSections(starsNextUpdate, starsSegmentOffset, STARS, ARRAY_SIZE(STARS), 1, 150, -1, gHue + ((255 / 3) * 1));

  static unsigned long centersNextUpdate = 0;
  static uint16_t centersSegmentOffset = 0;
  PatternRotateThoughSubSections(centersNextUpdate, centersSegmentOffset, CENTERS, ARRAY_SIZE(CENTERS), 1, 300, 1, gHue + ((255 / 3) * 2));

  PatternSetColor(HEXES, ARRAY_SIZE(HEXES), gHue + (255 / 3) * 3);
}

void Rings()
{
  PatternSetColor(PATTERN_RINGS_ONE, ARRAY_SIZE(PATTERN_RINGS_ONE), gHue + (255 / 3) * 0);
  PatternSetColor(PATTERN_RINGS_TWO, ARRAY_SIZE(PATTERN_RINGS_TWO), gHue + (255 / 3) * 1);
  PatternSetColor(PATTERN_RINGS_THREE, ARRAY_SIZE(PATTERN_RINGS_THREE), gHue + (255 / 3) * 2);
}
*/

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {
  // Customs Patterns
  // --------------------------------------------
  DifferentColorFlowers, 
  Cross,
  rotatingFlowers,
  rotatingPettlesForwards, rotatingPettlesBackwards,
  spirialOut,

  // Default Patterns
  // --------------------------------------------
  rainbow, confetti, sinelon, bpm, juggle
  };

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
  Serial.print("[");
  Serial.print(millis());
  Serial.print("] FYI: Changing pattern to offset: ");
  Serial.print(gCurrentPatternNumber);
  Serial.print("\n");
}

void setup()
{
  delay(3000); // 3 second delay for recovery

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  PrintStartUpInfo();
  TestPattern();
}

void CheckSerialInput() {
  fadeToBlackBy(leds, NUM_LEDS, 4);

  if (Serial.available() > 0) {
    String strRaw = Serial.readStringUntil('\n');

    int strOffset = atoi(strRaw.c_str()); // Serial.parseInt();
    leds[strOffset] = CHSV(gHue, 200, 255);

    Serial.print("[");
    Serial.print(millis());
    Serial.print("] FYI: Setting offset. strOffset=");
    Serial.print(strOffset);
    Serial.print("\n");

    delay(1000);
    FastLED.show();

  }
  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

}

void loop()
{
  // CheckSerialInput();
  // return ;

  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS(20) { gHue++; }                              // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS(SETTING_TIME_FOR_EACH_PATTERN) { nextPattern(); } // change patterns periodically
}
