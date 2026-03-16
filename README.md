# HealthPod – Smart Biomedical Medication Dispensing System

HealthPod is a biomedical engineering prototype designed to automate medication dispensing and improve patient adherence through an embedded control system.  
The device integrates sensors, servo-based mechanical dispensing, and real-time system monitoring through an OLED interface.

This project demonstrates how embedded systems can be applied in healthcare technology to support safer and more reliable medication management.

---

## Project Overview

Medication adherence is a major challenge in healthcare, especially for patients who require regular medication schedules. HealthPod was developed as a prototype biomedical device that assists in controlled medication dispensing using a microcontroller-based system.

The system uses analog sensors to estimate the stock level inside medication compartments and servo motors to release pills on command. An OLED display provides real-time system feedback, including device status and medication availability.

The design focuses on creating a simple, modular, and scalable platform that can later be extended into more advanced smart healthcare devices.

---

## System Architecture

The HealthPod system consists of three main components:

### 1. Embedded Control Unit
An Arduino microcontroller is responsible for managing all system operations including sensor reading, servo control, user commands, and system monitoring.

### 2. Medication Dispensing Mechanism
Each compartment contains a servo-driven mechanical gate that rotates to release a pill when a dispensing command is triggered.

### 3. Monitoring Interface
An OLED display provides a real-time interface that shows system state and medication stock levels.

---

## Key Features

• Automated pill dispensing mechanism  
• Real-time medication stock monitoring  
• OLED display for device status and compartment information  
• Servo-based mechanical dispensing control  
• Adjustable sensor calibration through serial commands  
• Embedded servo control implemented without the Arduino Servo library  

---

## Hardware Components

The prototype system includes the following hardware components:

- Arduino microcontroller
- 3 Servo motors for dispensing control
- 3 analog sensors for medication level detection
- OLED display (SSD1306)
- Mechanical pill dispensing structure

---

## Software Functionality

The Arduino control program performs the following operations:

1. Reads sensor values from each medication compartment  
2. Determines medication stock level (FULL, MEDIUM, LOW)  
3. Displays system information on the OLED screen  
4. Controls servo motors to release medication  
5. Maintains servo home positions during idle operation  
6. Accepts manual dispensing commands via the serial interface  

The servo movement is implemented using custom pulse generation rather than the standard Arduino Servo library, allowing more precise control over timing and motion behavior.

---

## Serial Control Commands

Dispensing commands can be triggered through the Arduino Serial Monitor:

1 → Dispense medication from compartment 1  
2 → Dispense medication from compartment 2  
3 → Dispense medication from compartment 3  

Sensor calibration can also be adjusted:

f / F → Increase or decrease FULL threshold  
m / M → Increase or decrease MEDIUM threshold  

---

## File Structure

HealthPod.ino – Main Arduino control program for the HealthPod system

---

## Future Improvements

This prototype can be expanded into a more advanced smart healthcare system by integrating:

• Medication scheduling and automated timing  
• Wireless communication (WiFi / Bluetooth)  
• Mobile application integration  
• Patient reminder and alert systems  
• Cloud-based medication monitoring  

---

## Author

Hamza Ismail  
Biomedical & Instrumentation Engineering Student
