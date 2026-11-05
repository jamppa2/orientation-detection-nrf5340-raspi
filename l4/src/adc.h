#ifndef ADC_H_KJJ
#define ADC_H_KJJ

struct Measurement
{
   uint16_t x;
   uint16_t y;
   uint16_t z;
   uint16_t direction;
};

int initializeADC(void);
struct Measurement readADCValue(void);
void printDebugInfo(void);


#endif



