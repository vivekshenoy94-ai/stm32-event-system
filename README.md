# STM32 Button Event System (Multi-Press Event Driven Architecture)

------------------------------------------------------------------------

## Overview
This project demonstrates a **robust event-driven embedded system**
using:

-   External Interrupts (EXTI) for button detection
-   Timer Interrupts for debounce
-   State Machine for structured control
-   Event-driven logic for user interraction

The system separates **detection, validation, and action**, ensuring
reliable and scalable behavior.

Additionally, the design introduces a modular button driver abstraction and
state-based application control, enabling clean separation between input handling
and output behavior.

## Hardware Setup
- Button → PC13 (EXTI Rising & Falling Edge)
  - External pull-up configuration:
     - Idle state → HIGH  
     - Button Press → LOW (Falling Edge)  
     - Button Release → HIGH (Rising Edge)  
- LED → PA5 (Output)  
- Timer → TIM2 (~20 ms debounce)  
- MCU → STM32F446RE

## System Evolution

This project evolved through multiple iterations to improve system reliability,
control flow, and architectural clarity.


### Version 1 — Basic State Machine (ISR-Driven)

- Implemented a simple state machine:
  - BUTTON_IDLE
  - BUTTON_PRESSED
- Debounce handled using timer interrupt
- State transitions and actions handled inside ISR

#### Limitations

- Logic tightly coupled inside interrupts
- Hard to scale for complex behaviors
- Difficult to debug and extend
- Violates best practice: heavy ISR processing


### Version 2 — Refactored State Machine (Main-Driven)

- Retained state machine concept but improved structure
- Moved decision-making and actions to main loop
- ISR used only for:
  - Event detection (EXTI)
  - Signal validation (Timer)

#### Improvements

- Clean separation of concerns
- Improved system predictability
- Easier debugging and maintenance
- Scalable for advanced features

### Version 3 — Event-Driven Architecture with Short & Long Press Handling

- Introduced event-based behavior:
  - EVENT_SHORT_PRESS
  - EVENT_LONG_PRESS
- Press duration measured using HAL_GetTick()
- Timing starts *after debounce validation*
- Actions triggered only in main loop

#### System Behavior

- *Short Press (< 2000 ms):*
  - LED toggles once
  - Action is triggered exactly once per press

- *Long Press (> 2000 ms):*
  - LED enters continuous toggle mode (non-blocking)
  - Behavior persists until next user interaction

#### Improvements

- Accurate press duration measurement  
- No duplicate or premature triggers  
- Clear separation between *state (validation)* and *event (action)*  
- Supports advanced features like long press  
- Non-blocking behavior for continuous actions


### Version 4 — Multi-Press Event Handling with Double Click Support

- Added event-based behavior:
  - EVENT_DOUBLE_CLICK

- Introduced new FSM states:
  - BUTTON_WAIT_DOUBLE
  - BUTTON_SECOND_PRESS

- Implemented *time-window based detection*:
  - Double click window: 1000 ms
  - Long press threshold: 2000 ms

- Event evaluation enhanced with *priority handling*:
  - LONG_CLICK > DOUBLE_CLICK > SHORT_CLICK

#### System Behavior

- Short Click (< 2000 ms):
  - Triggered only after double-click timeout expires
  - LED toggles once

- Long Click (> 2000 ms):
  - Highest priority event
  - Overrides double click detection
  - LED enters continuous toggle mode

- Double Click (two presses within 1 second):
  - Detected only after second release
  - LED toggles twice

#### Improvements

- Multi-stage event classification  
- Accurate interaction modeling based on user intent  
- Proper handling of second press with dedicated state  
- Eliminates premature double-click detection  
- Priority-based event resolution  
- Scalable foundation for multi-click and multi-button systems


### Version 5 — Modular Driver Abstraction with State-Based Output Handling (Current)

- Introduced button driver abstraction:
  
  - button.c / button.h
  - Encapsulates FSM, debounce, and event generation

- Application layer redesigned to separate:
  
  - Event handling (input interpretation)
  - State management (system behavior)
  - Output execution (LED control)

- Introduced state-based output control:
  
  - LED_IDLE
  - LED_BLINK

- Introduced action flags for transient behavior:
  
  - Single toggle
  - Double toggle

- Implemented state override mechanism:
  
  - Any new user input exits ongoing states (e.g., blinking)

#### System Behavior

- Short Click
  → Exit BLINK (if active)
  → Toggle LED once

- Double Click
  → Exit BLINK (if active)
  → Toggle LED twice

- Long Click
  → Enter BLINK state (continuous non-blocking toggle)

#### Improvements

- Clear separation of:
  
  - Driver (input handling)
  - Application (decision making)
  - Output (execution)

- Proper distinction between:
  
  - Event (instant trigger)
  - State (continuous behavior)

- Eliminates state overwrite issues

- Ensures all states have valid exit paths

- Improves scalability for multiple inputs and outputs

---
## Key Features
- Interrupt-driven button handling (EXTI)
- State machine-based signal validation
- Event-driven action handling
- Clean separation of ISR and main loop
- Short press detection  
- Long press detection (> 2 seconds)  
- Double click detection (< 1 second window)  
- Multi-stage FSM handling  
- Non-blocking LED behavior
- Deterministic and stable behavior
- Scalable multi-button design

## System Architecture
EXTI → Detect edge\
↓\
Button Driver (FSM) → Event Generation\
↓\
Application Layer → State Update\
↓\
Output Layer → Action Execution

## System Flow
Button Press (PC13)\
→ EXTI Interrupt Triggered
→ FSM updates state
→ Capture press_start_time

Button Release\
→ Measure duration
→ Classify event (SHORT / LONG / DOUBLE)
→ Generate event

Main Loop\
→ Pop event
→ Update application state
→ Execute output behavior

## Advantages
- Stable and deterministic response  
- Accurate press duration measurement  
- Clean separation of detection, validation, and action  
- Supports multi-click interactions  
- Priority-based event resolution  
- Scalable for advanced input handling
- Supports complex user interactions
- Scalable architecture for future extensions

## Considerations
- ISR kept minimal
- FSM ensures valid transitions
- Main loop must remain non-blocking
- External pull-up defines active-low logic

## Learning Outcomes
- EXTI interrupt handling
- Timer-based debounce
- State machine design
- Multi-event input handling
- ISR design best practices
- Debounce using hardware timer
- Event-driven embedded system design
- Separation of concerns in embedded systems
- Time-window based interaction modeling
- Event-driven embedded architecture
- State vs Event separation
- Multi-click interaction modeling
- Driver abstraction in embedded systems
- Scalable system design principles


## Author
Vivek Shenoy K\
Embedded Software Architect
