# FFT_AGV – Autonomous Guided Vehicle System

This project implements a complete Automated Guided Vehicle (AGV) system used to transport television bases as part of the Full Functionality Test (FFT) process in a production line. The AGVs are powered by ESP32-S3 microcontrollers running an embedded state machine for autonomous operation and communication.

Each AGV is connected via Wi-Fi and transmits operational data to Firebase Realtime Database. Firebase Cloud Functions are used to detect key events and trigger push notifications to a mobile app, enabling real-time monitoring of AGV status and alerts.

A mobile application developed in Flutter serves as the user interface, capable of displaying live status updates and receiving alerts, even when the app is in the background or the device is locked. Each AGV is uniquely identified, and the system supports multiple units reporting simultaneously to the same backend.

The architecture is modular and scalable, allowing for future integration of additional control logic, sensors, or cloud services.

## Key Technologies
- ESP32-S3 with Arduino and FreeRTOS
- Firebase Realtime Database and Cloud Functions
- Flutter Android application with push notifications

## Project Structure
- `esp32/`: Firmware and state machine for the AGV
- `flutter_app/`: Flutter-based mobile application
- `firebase_functions/`: Cloud backend to handle notifications and database triggers
- `docs/`: Technical documentation and architecture references

## Project Status
- Base structure completed
- Active development in progress
