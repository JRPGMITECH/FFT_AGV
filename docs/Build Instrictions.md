Build & Deployment Instructions – FFT_AGV

This section provides the exact steps to build, flash, and deploy all parts of the FFT_AGV system.

🧠 ESP32 Firmware

Requirements:

Arduino IDE or PlatformIO

Board: ESP32-S3 DevKitM-1 (or compatible)

Libraries: WiFi, FirebaseESP32, Preferences

Build Steps:

Open the main .ino file in the /esp32/ directory.

Connect your ESP32 via USB.

Select the correct board and port.

Flash the firmware.

Post-Flash:

On first boot, enter configuration mode to define lift target positions.

These will be stored in NVS.

🔥 Firebase Cloud Functions

Requirements:

Node.js (LTS version)

Firebase CLI (npm install -g firebase-tools)

Build Steps:

Navigate to /functions/

Run:

npm install
firebase deploy --only functions

Function notificarAlarma will now listen for changes on /datos_esp32/contador_entero

📱 Flutter Mobile App

Requirements:

Flutter SDK (3.x)

Android Studio / VSCode

Firebase account linked to app project

Build Steps:

Navigate to /flutter_app/

Ensure you have google-services.json inside /android/app/

Run:

flutter pub get
flutter run

Features:

Receives push notifications from alarma_scada

Triggers vibration and sound

Shows alarm type on screen

Reset button disables further alerts until value changes

✅ Checklist Summary



Your system should now operate fully end-to-end 💡

