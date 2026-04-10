#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#define LED 19
#define OLED_SDA 26
#define OLED_SCL 25

// WiFi
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// MQTT
const char *mqtt_server = "***";
const int mqtt_port = 8883;
const char *mqtt_username = "***";
const char *mqtt_password = "***";

WiFiClientSecure espClient;
PubSubClient client(espClient);

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

unsigned long lastMsg = 0;

// ================= WiFi =================
void setup_wifi()
{
  Serial.print("Đang kết nối WiFi... ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối");
}

// ================= MQTT Callback =================
void callback(char *topic, byte *payload, unsigned int length)
{
  String cmd = "";
  for (int i = 0; i < length; i++)
  {
    cmd += (char)payload[i];
  }

  Serial.println("Lệnh nhận: " + cmd);

  // Điều khiển LED
  if (cmd == "On")
  {
    digitalWrite(LED, HIGH);
  }
  else if (cmd == "Off")
  {
    digitalWrite(LED, LOW);
  }

  // Hiển thị OLED
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 20, "MQTT Message:");
  display.drawStr(0, 40, cmd.c_str());
  display.sendBuffer();
}

// ================= Reconnect =================
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Kết nối MQTT...");
    if (client.connect("ESP32_Client", mqtt_username, mqtt_password))
    {
      Serial.println(" OK");
      client.subscribe("/home/light/cmd");
      client.subscribe("/home/oled/msg");
    }
    else
    {
      Serial.println(" Lỗi, thử lại sau 5s");
      delay(5000);
    }
  }
}

// ================= Setup =================
void setup()
{
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  setup_wifi();

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 20, "MQTT Ready");
  display.sendBuffer();
}

// ================= Loop =================
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    client.publish("/home/test", "ESP32 alive");
  }
}
