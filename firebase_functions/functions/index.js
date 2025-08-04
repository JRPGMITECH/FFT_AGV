const { onValueWritten } = require("firebase-functions/v2/database");
const { setGlobalOptions } = require("firebase-functions");
const { getMessaging } = require("firebase-admin/messaging");
const { initializeApp } = require("firebase-admin/app");

initializeApp();
setGlobalOptions({ maxInstances: 10 });

// 🔥 Esta función escucha a cualquier ESP en su ruta específica
exports.alarmaDesdeCualquierESP = onValueWritten("/datos_esp32/{esp_id}/alarma", async (event) => {
  const before = event.data.before.val();
  const after = event.data.after.val();
  const espId = event.params.esp_id;  // Ejemplo: "esp1", "esp2", etc.

  // ✅ Reaccionar si cambió a 1, 2 o 3 (y no repite el mismo valor)
  if (before !== after && [1, 2, 3].includes(after)) {
    const agvId = `AGV${espId.replace("esp", "")}`; // ← Esto convierte "esp1" → "AGV1"

    const mensaje = {
      notification: {
        title: "¡Alerta en FFT!",
        body: `⚠️ Alarma ${after} activada desde ${agvId}`,
      },
      android: {
        notification: {
          channelId: "scada_channel",
          priority: "high",
        },
      },
      topic: "alarma_scada"
    };

    try {
      for (let i = 0; i < 10; i++) {
        await getMessaging().send(mensaje);
        console.log(`🔁 Notificación ${i + 1}/10 enviada desde ${agvId}.`);
        await new Promise(resolve => setTimeout(resolve, 5000)); // 5 segundos
      }
      console.log(`✅ Notificaciones completadas desde ${agvId}`);
    } catch (error) {
      console.error(`❌ Error al enviar notificación desde ${agvId}:`, error);
    }
  }
});




























