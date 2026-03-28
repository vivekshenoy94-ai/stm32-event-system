# STM32 Button Event System (EXTI + Timer Interrupt)

------------------------------------------------------------------------

## Overview
This project demonstrates a robust event-driven embedded system using:
External Interrupts (EXTI) for button detection\
Timer Interrupts for debounce and validation\
The system avoids polling and keeps processing within interrupt context,\
ensuring fast response and controlled execution.

## Hardware Setup
Button → PC13 (External Interrupt Input)  
LED → PA5 (Output)  
Timer → TIM2 (Debounce Timer)  
MCU → STM32F446RE


## Key Features
Interrupt-driven button handling (EXTI)\
Timer-based debounce handling\
Fully interrupt-driven flow (no main-loop processing)\
State machine-based control\
ISR kept short and structured\
Deterministic and responsive behavior



## System Flow
Button Press (PC13)  
→ EXTI Interrupt Triggered  
→ ISR (HAL_GPIO_EXTI_Callback)  
→ Start Timer (Debounce)  
→ Timer Interrupt (TIM2)  
→ Validate Button Press  
→ LED Toggle (PA5)



## Advantages
Eliminates button bounce issues\
No dependency on main loop\
Fast and deterministic response\
Fully interrupt-driven design\
Clean separation of detection and validation

## Considerations
Slightly longer ISR execution (timer ISR handles logic)\
Care must be taken to keep ISR efficient\
Not ideal for complex processing inside ISR

## Learning Outcomes
EXTI interrupt handling\
Timer interrupt usage\
State handling\
ISR design best practices\
Debounce using hardware timer\
Event-driven embedded system design


## Author
Vivek Shenoy K\
Embedded SW Architect
