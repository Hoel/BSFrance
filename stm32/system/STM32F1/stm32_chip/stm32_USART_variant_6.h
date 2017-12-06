
const stm32_af_pin_list_type chip_af_usart_rx [] = {
//UART4
    { UART4 , GPIOC, GPIO_PIN_11 , AF_NO_REMAP    }, 
//UART5
    { UART5 , GPIOD, GPIO_PIN_2  , AF_NO_REMAP    }, 
//USART1
    { USART1, GPIOA, GPIO_PIN_10 , AF__HAL_AFIO_REMAP_USART1_DISABLE}, 
    { USART1, GPIOB, GPIO_PIN_7  , AF__HAL_AFIO_REMAP_USART1_ENABLE}, 
//USART2
    { USART2, GPIOA, GPIO_PIN_3  , AF__HAL_AFIO_REMAP_USART2_DISABLE}, 
    { USART2, GPIOD, GPIO_PIN_6  , AF__HAL_AFIO_REMAP_USART2_ENABLE}, 
//USART3
    { USART3, GPIOB, GPIO_PIN_11 , AF__HAL_AFIO_REMAP_USART3_DISABLE}, 
    { USART3, GPIOC, GPIO_PIN_11 , AF__HAL_AFIO_REMAP_USART3_PARTIAL}, 
    { USART3, GPIOD, GPIO_PIN_9  , AF__HAL_AFIO_REMAP_USART3_ENABLE}, 
}; 

const stm32_af_pin_list_type chip_af_usart_tx [] = {
//UART4
    { UART4 , GPIOC, GPIO_PIN_10 , AF_NO_REMAP    }, 
//UART5
    { UART5 , GPIOC, GPIO_PIN_12 , AF_NO_REMAP    }, 
//USART1
    { USART1, GPIOA, GPIO_PIN_9  , AF__HAL_AFIO_REMAP_USART1_DISABLE}, 
    { USART1, GPIOB, GPIO_PIN_6  , AF__HAL_AFIO_REMAP_USART1_ENABLE}, 
//USART2
    { USART2, GPIOA, GPIO_PIN_2  , AF__HAL_AFIO_REMAP_USART2_DISABLE}, 
    { USART2, GPIOD, GPIO_PIN_5  , AF__HAL_AFIO_REMAP_USART2_ENABLE}, 
//USART3
    { USART3, GPIOB, GPIO_PIN_10 , AF__HAL_AFIO_REMAP_USART3_DISABLE}, 
    { USART3, GPIOC, GPIO_PIN_10 , AF__HAL_AFIO_REMAP_USART3_PARTIAL}, 
    { USART3, GPIOD, GPIO_PIN_8  , AF__HAL_AFIO_REMAP_USART3_ENABLE}, 
}; 
