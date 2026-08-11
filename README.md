# Embedded-Secure-Gate-Controller
# STM32 Secure Smart Gate (Embedded Approach)

This repository contains the firmware and documentation for the "Prototipazione di un Cancello Automatico: Un Approccio Embedded" project[cite: 2]. The system models a synchronized, dual-motor automatic gate controlled via a secure Sub-GHz wireless remote.

**Authors:** Antonio Cicatelli, Gianni D'Avanzo, Carmine Gentile, Enrico Gentile.

## 📖 Project Overview
The objective of this project is to build a robust and secure embedded system to handle the opening/closing cycles of a gate[cite: 2]. The system features synchronized dual motors, obstacle detection, and a secure wireless transmission protocol to prevent malicious intrusions (such as Replay Attacks or unauthorized IDs).

## ⚙️ Architecture and Hardware
The project leverages the STMicroelectronics ecosystem, dividing the logic among a remote control and two motor units:
*   **Microcontrollers:** STM32F401RE and STM32F3DISCOVERY boards[cite: 2].
*   **Wireless Communication:** X-Nucleo IDS01A4 (Sub-GHz RF modules) for multicast packet transmission[cite: 2].
*   **Actuators & Sensors:** SG90 Servomotors, HC-SR04 Ultrasonic sensors (for obstacle detection), and Photocells[cite: 2].

## 🛡️ Core Functionalities

### Finite State Machine (FSM)
The control logic is driven by an FSM with the following robust states:
*   *Closed, Open, Opening, Closing, Obstacle, Stopped*[cite: 2].
The system smoothly transitions between states based on remote commands (e.g., toggling state) or sensor interrupts (e.g., halting if the ultrasonic sensor detects an obstacle)[cite: 2].

### Cyber-Security Module
To ensure a secure environment, the RF transmission integrates cryptographic checks:
*   **HMAC SHA-256:** Used to generate a cryptographic hash of the payload[cite: 2].
*   **Anti-Replay Attack:** A progressive counter is maintained, updated, and saved to the Flash memory to invalidate old, intercepted packets[cite: 2].
*   **ID Validation:** Each payload includes a verified Remote ID to reject commands from unauthorized transmitters[cite: 2].

## 📁 Repository Structure
*   `/src/Remote_Control`: STM32CubeIDE project for the Sub-GHz remote control.
*   `/src/Gate_Motor_Left`: STM32CubeIDE project for the left wing motor and synchronization.
*   `/src/Gate_Motor_Right`: STM32CubeIDE project for the right wing motor and obstacle detection.
*   `/docs`: Project presentation and schematics.
*   `/media`: Videos and photos of the physical hardware setup and tests.
