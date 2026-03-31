#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "main.h"

extern ADC_HandleTypeDef hadc1;

void MX_ADC1_Init(void);
uint32_t ADC_ReadSingle(void);

#endif /* INC_ADC_H_ */
