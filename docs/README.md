FFT_AGV – Autonomous Guided Vehicle System | v1.0.0

FFT_AGV is an embedded and mobile software system designed to control and monitor the behavior of a custom-built Autonomous Guided Vehicle (AGV). It combines real-time sensor input, state machine logic, elevation control, regenerative braking, and remote alerting capabilities through Firebase and a Flutter-based mobile interface.

📌 System Features

🔄 State Machine Architecture (FreeRTOS, Core 1):

Manages AGV operational flow: forward movement, halting, reversing, and elevation triggers

Detects entry/exit points using magnetic sensors

Monitors operation cycles to determine alarm conditions

Sends alarms to Firebase when abnormal conditions are detected:

Alarm 1: Transfer cycle incomplete due to derailment, blocked rails, faulty sensors, or motor non-activation

Alarm 2: Main motor driver overvoltage caused by mechanical blockage or electrical fault

Alarm 3: Obstruction during elevation (e.g., misaligned object preventing lift-up)

Alarm 4: Incomplete lowering cycle (e.g., home sensor fault, motor inactive)

📈 Linear Actuator Control:

Controls electric piston-based lift mechanism

Triggers elevation/lowering at defined positions

🧠 Configuration Mode for Elevation Levels:

Allows manual control of actuator to set target positions

Stores values persistently using ESP32's NVS (non-volatile storage)

🚦 Magnetic Sensors:

Detect specific AGV path locations: start, stop, and action zones

⚡ Current Monitoring + Dynamic Braking:

Monitors motor current and system load

Implements dynamic braking to dissipate or recover energy safely

📡 Firebase Integration (Core 0):

Realtime Database receives alarms and status updates

Cloud Function (notificarAlarma) triggers push notifications to subscribed clients

Alarm data is structured under:

/datos_esp32/contador_entero: 0 | 1 | 2 | 3 | 4

Where 1–4 represent the four distinct alarms

📱 Flutter Mobile App:

Subscribes to Firebase Messaging Topic alarma_scada

Receives and displays alarm notifications on screen with clear AGV ID and alarm type

Activates vibration and sound alerts on mobile when alarms arrive

Includes reset button to acknowledge alarms and stop repeated alerts

📁 Repository Structure

/flutter_app       # Flutter mobile application (push notifications, UI)
/functions         # Firebase backend (Realtime DB triggers, messaging)
/esp32             # Firmware for ESP32 (state machine, actuator control, sensors)
/docs              # Technical documentation
.gitignore         # Local exclusions (builds, keys, credentials)
firebase.json      # Firebase configuration
.firebaserc        # Firebase project setup

🛡️ Security Notice

This public repository does not contain any private credentials. Sensitive data such as:

Wi-Fi credentials

Firebase authentication tokens

Platform API keys
are all managed locally and excluded via .gitignore.

🔖 Version: v1.0.0

Status:

Initial FreeRTOS-based state machine completed

Elevation configuration and actuator control implemented

Firebase reporting and four alarm conditions integrated

Flutter app push notifications, vibration, and sound alerts functional

Upcoming Work:

UI refinements and real-time status overlays in Flutter

OTA configuration options

Enhanced topic-based event classification in backend
