// Importamos las librerías necesarias
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:flutter/services.dart';
import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_messaging/firebase_messaging.dart';
import 'dart:async';

@pragma('vm:entry-point')
Future<void> _onBackgroundMessage(RemoteMessage message) async {
  print("🔔 [Background] Recibido: \${message.notification?.body}");
}

final FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin = FlutterLocalNotificationsPlugin();

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  FirebaseMessaging.onBackgroundMessage(_onBackgroundMessage);
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  bool alarmaActiva = false;
  bool bloqueoActivo = false;
  Timer? alertaTimer;
  List<String> mensajesAlarmas = [];

  @override
  void initState() {
    super.initState();
    FirebaseMessaging messaging = FirebaseMessaging.instance;
    messaging.requestPermission(alert: true, sound: true);
    messaging.getToken().then((token) => print("🟢 Token FCM del dispositivo: \$token"));
    messaging.subscribeToTopic('alarma_scada');
    FirebaseMessaging.onMessage.listen(procesarMensaje);
    FirebaseMessaging.onMessageOpenedApp.listen(procesarMensaje);
    initializeNotifications();
    createNotificationChannel();
  }

  void procesarMensaje(RemoteMessage message) {
    String? body = message.notification?.body;

    final exp = RegExp("AGV(\\d+)");
    final match = exp.firstMatch(body ?? "");
    final agv = match != null ? "AGV${match.group(1)}" : "AGV?";

    int valorAlarma = 0;
    final alarmaExp = RegExp(r'Alarma (\d) activada');
    final alarmaMatch = alarmaExp.firstMatch(body ?? "");
    if (alarmaMatch != null) {
      valorAlarma = int.tryParse(alarmaMatch.group(1)!) ?? 0;
    }

    if (bloqueoActivo && valorAlarma != 0) {
      print("🔒 Bloqueado. Ignorando alarma \$valorAlarma");
      return;
    }

    if (valorAlarma == 0) {
      setState(() {
        alarmaActiva = false;
        mensajesAlarmas.clear();
        bloqueoActivo = false;
      });
      alertaTimer?.cancel();
      alertaTimer = null;
      print("✅ Alarma desbloqueada por valor 0");
      return;
    }

    String textoVisible;
    if (valorAlarma == 1) {
      textoVisible = "$agv ALARMA 1 DESCARRILAMIENTO DE AGV";
    } else if (valorAlarma == 2) {
      textoVisible = "$agv ALARMA 2 OBSTRUCCIÓN DEL ELEVADOR";
    } else if (valorAlarma == 3) {
      textoVisible = "$agv ALARMA 3 PROBLEMA CON EL ELEVADOR";
    } else {
      textoVisible = "$agv ALARMA DESCONOCIDA";
    }

    setState(() {
      if (!mensajesAlarmas.contains(textoVisible)) {
        mensajesAlarmas.add(textoVisible);
      }
      alarmaActiva = true;
    });

    showNotification(textoVisible);
    startAlarmaEfectos(textoVisible);
  }

  Future<void> initializeNotifications() async {
    const AndroidInitializationSettings initializationSettingsAndroid = AndroidInitializationSettings('@mipmap/ic_launcher');
    const InitializationSettings initializationSettings = InitializationSettings(android: initializationSettingsAndroid);
    await flutterLocalNotificationsPlugin.initialize(initializationSettings);
  }

  Future<void> createNotificationChannel() async {
    const AndroidNotificationChannel channel = AndroidNotificationChannel(
      'scada_channel',
      'SCADA Alertas',
      description: 'Canal para notificaciones de alarma SCADA',
      importance: Importance.max,
    );
    final androidPlugin = flutterLocalNotificationsPlugin.resolvePlatformSpecificImplementation<AndroidFlutterLocalNotificationsPlugin>();
    await androidPlugin?.createNotificationChannel(channel);
  }

  Future<void> showNotification(String cuerpo) async {
    const AndroidNotificationDetails androidDetails = AndroidNotificationDetails(
      'scada_channel',
      'SCADA Alertas',
      channelDescription: 'Notificaciones de alarma SCADA',
      importance: Importance.max,
      priority: Priority.high,
      playSound: true,
      enableVibration: true,
      ticker: 'ticker',
    );
    const NotificationDetails notificationDetails = NotificationDetails(android: androidDetails);
    await flutterLocalNotificationsPlugin.show(
      0,
      '🚨 Alerta AGV',
      cuerpo,
      notificationDetails,
    );
  }

  void startAlarmaEfectos(String cuerpo) {
    alertaTimer?.cancel();
    alertaTimer = Timer.periodic(Duration(seconds: 5), (timer) {
      if (alarmaActiva) {
        showNotification(cuerpo);
      } else {
        timer.cancel();
        alertaTimer = null;
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'SCADA Contador',
      home: Scaffold(
        backgroundColor: Colors.white,
        body: Stack(
          children: [
            Positioned(
              bottom: 0,
              left: 0,
              right: 0,
              child: Image.asset(
                'assets/fondo_empresa.png',
                fit: BoxFit.fitWidth,
                width: double.infinity,
                height: 80,
              ),
            ),
            Padding(
              padding: const EdgeInsets.only(top: 80.0),
              child: Align(
                alignment: Alignment.topCenter,
                child: Text(
                  'Monitoreo de líneas FFT',
                  style: TextStyle(
                    fontSize: 28,
                    fontWeight: FontWeight.bold,
                    color: Colors.blueAccent,
                  ),
                ),
              ),
            ),
            Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  ...mensajesAlarmas.map((msg) => Padding(
                    padding: const EdgeInsets.symmetric(vertical: 8.0),
                    child: Text(
                      msg,
                      textAlign: TextAlign.center,
                      style: TextStyle(
                        fontSize: 24,
                        color: Colors.redAccent,
                        fontWeight: FontWeight.bold,
                        shadows: [Shadow(offset: Offset(1, 1), blurRadius: 3, color: Colors.black26)],
                      ),
                    ),
                  )).toList(),
                  const SizedBox(height: 30),
                  ElevatedButton.icon(
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.black,
                      foregroundColor: Colors.white,
                      padding: EdgeInsets.symmetric(horizontal: 24, vertical: 12),
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(12),
                      ),
                      elevation: 10,
                    ),
                    onPressed: () {
                      setState(() {
                        alarmaActiva = false;
                        mensajesAlarmas.clear();
                        bloqueoActivo = true;
                      });
                      alertaTimer?.cancel();
                      alertaTimer = null;
                    },
                    icon: Icon(Icons.restart_alt),
                    label: const Text('RESET', style: TextStyle(fontSize: 18, letterSpacing: 2)),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}
