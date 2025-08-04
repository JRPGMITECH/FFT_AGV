System Architecture – FFT_AGV

🧠 Component Overview

1. ESP32 Subsystem

Core 1: Executes the main finite state machine

Controls driving logic, elevation triggers, and state transitions

Detects failure conditions and triggers alarms 1–4 based on runtime diagnostics

Core 0: Handles Wi-Fi and Firebase communications

Sends alarm codes to Realtime Database

2. Sensors and Actuation

Magnetic Sensors: Identify specific line positions (entry/exit zones)

Current Sensors: Monitor power draw on main motor

Electric Linear Actuator: Raises/lowers the AGV base

Includes limit/home sensors for full cycle validation

3. Alarm Mapping (via contador_entero)

/alarma = 0   // Normal
/alarma = 1   // Alarm 1: derailment or path obstruction
/alarma = 2   // Alarm 2: motor overvoltage
/alarma = 3   // Alarm 3: lift blocked
/alarma = 4   // Alarm 4: lift did not complete cycle

4. Firebase Backend

Realtime DB receives values from ESP32

Cloud Function notificarAlarma triggers push notification if value > 0

Messages are published to FCM topic alarma_scada

5. Flutter Mobile App

Listens to topic alarma_scada

Parses alarm value and shows:

On-screen banner (AGV ID + Alarm type)

Device vibration + sound

Reset button disables alert locally

🔁 System Flow Diagram

[ESP32 - Core 1: FSM] → [Core 0: Firebase] → [Realtime DB] → [Function: notificarAlarma] → [Flutter App Alert]

💡 Notes

Elevation configuration (via manual buttons) writes height values to NVS

System state and sensor health are managed dynamically by FSM

Modular design enables future extensions (e.g., OTA config, more AGVs)