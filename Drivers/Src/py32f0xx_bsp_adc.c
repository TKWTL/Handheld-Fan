#include "py32f0xx_bsp_adc.h"

void BSP_ADC_Config()
{
    LL_ADC_InitTypeDef ADC_Init;
    LL_ADC_REG_InitTypeDef LL_ADC_REG_InitType;

    LL_ADC_Reset(ADC1);

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);

    // Calibrate start
    if (LL_ADC_IsEnabled(ADC1) == 0)
    {
        LL_ADC_StartCalibration(ADC1);
        while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0);
        /* Delay 1ms(>= 4 ADC clocks) before re-enable ADC */
        LL_mDelay(1);
    }
    // Calibrate end

    ADC_Init.Clock = LL_ADC_CLOCK_SYNC_PCLK_DIV64;
    ADC_Init.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_Init.LowPowerMode = LL_ADC_LP_MODE_NONE;
    ADC_Init.Resolution = LL_ADC_RESOLUTION_12B;
    LL_ADC_Init(ADC1, &ADC_Init);
    LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_239CYCLES_5);

    // Regular ADC config
    LL_ADC_REG_InitType.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
    LL_ADC_REG_InitType.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
    LL_ADC_REG_InitType.Overrun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;
    LL_ADC_REG_InitType.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
    LL_ADC_REG_InitType.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    LL_ADC_REG_Init(ADC1, &LL_ADC_REG_InitType);
    // Set common path and internal channel
    LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT | LL_ADC_PATH_INTERNAL_TEMPSENSOR);
    // Delay to ensure temperature sensor becomes stable
    LL_mDelay(1);
    // Select temperature sensor
        
    // Enable ADC
    //LL_ADC_Enable(ADC1);
}

