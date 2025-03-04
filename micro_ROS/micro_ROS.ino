// Include libraries to be used

#include <micro_ros_arduino.h>      // micro-ros-arduino library
#include <rcl/rcl.h>                // Core ROS 2 Client Library (RCL) for node management
#include <rcl/error_handling.h>     // Error handling utilities for Micro-ROS
#include <rclc/rclc.h>              // Micro-ROS Client Library for embedded devices
#include <rclc/executor.h>          // Micro-ROS Executor to manage callbacks
#include <std_msgs/msg/int32.h>     // Predefined ROS 2 message type
#include <stdio.h>                  // Standard I/O library for debugging

// Encoder pins and state variables
#define ENCODER_A 35
#define ENCODER_B 34 

volatile int32_t encoder_count = 0;
const byte debounce_delay = 5;

// Nodes
rcl_node_t motor_node;                // ROS 2 Node running on the MCU

// Instantiate executor and its support classes
rclc_executor_t executor;             // Manages task execution (timers, callbacks, etc.)
rclc_support_t support;               // Handles initialization & communication setup
rcl_allocator_t allocator;            // Manages memory allocation

// Publishers
rcl_publisher_t encoder_publisher;    // ROS 2 Publisher for sending messages

// Timers
rcl_timer_t timer;                    // Timer to execute functions at intervals

// Messages
std_msgs__msg__Int32 encoder_msg; 

// MACROS
// Executes a function and calls error_loop() if it fails
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}

// Executes function but ignores failures
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

// Error function
void error_loop() {
  while(1) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }
}

// Encoder ISRs

void IRAM_ATTR encoder_isr_A() {
  bool A = digitalRead(ENCODER_A);
  bool B = digitalRead(ENCODER_B);

  if (A == B) {
    encoder_count++;  // Clockwise
  } else {
    encoder_count--;  // Counter-clockwise
  }
}

void IRAM_ATTR encoder_isr_B() {
  bool A = digitalRead(ENCODER_A);
  bool B = digitalRead(ENCODER_B);

  if (A != B) {
    encoder_count++;  // Clockwise
  } else {
    encoder_count--;  // Counter-clockwise
  }
}

// Define callbacks
void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
  RCLC_UNUSED(last_call_time);                                           // Prevents compiler warnings about an unused parameter

  if (timer != NULL) {                                                   // Ensures the timer event is valid before executing actions
    // Read the current encoder count (use a temporary variable to avoid issues with the interrupt)
    int32_t current_count;
    noInterrupts();
    current_count = encoder_count;
    interrupts();

    // Update the message with the current encoder count
    encoder_msg.data = current_count;

    // Publish the message         
    RCSOFTCHECK(rcl_publish(&encoder_publisher, &encoder_msg, NULL));   // Publishes msg to the ROS 2 topic
  }
}

void setup() {

  // Setup microcontroller pins
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  // Attach interrupts to encoder pins A & B for quadrature detection
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoder_isr_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), encoder_isr_B, CHANGE);

  // Initializes communication between ESP32 and the ROS 2 Agent (serial)
  set_microros_transports();

  // Connection delay
  delay(2000);

  // Initializes memory allocation for micro-ROS operations
  allocator = rcl_get_default_allocator();

  // Creates a ROS 2 support structure to manage the execution context
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Create node
  RCCHECK(rclc_node_init_default(&motor_node, "mcu_encoder_node", "", &support));

  // Create publisher
  RCCHECK(rclc_publisher_init_default(
  &encoder_publisher,
  &motor_node,
  ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
  "motor_encoder_data"));

  // Create timer
  const unsigned int timer_timeout = 100;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  // Initializes the micro-ROS executor, which manages tasks and callbacks
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));

  // Register timer with executor
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  // Initialize message
  encoder_msg.data = 0;
}

void loop() {
  // Executor spin
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)));
  delay(10);
}