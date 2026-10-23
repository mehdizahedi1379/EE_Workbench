/*
 * میز کار مهندس برق و الکترونیک - ESP32 Web Server
 * ------------------------------------------------
 * صفحهٔ HTML (فشرده‌شده با gzip) داخل فایل index_html.h در حافظهٔ فلش
 * ذخیره شده و توسط ESP32 سرو می‌شود. تمام محاسبات در مرورگر انجام می‌شود.
 *
 * حالت‌ها:
 *   1) اتصال به مودم خانگی (STA)  -> آدرس: http://eeworkbench.local  یا IP چاپ‌شده در Serial
 *   2) اگر وصل نشد، خودکار حالت Access Point -> وای‌فای "EE-Workbench" سپس http://192.168.4.1
 *
 * بورد: هر ESP32 (ESP32 Dev Module) - هستهٔ Arduino-ESP32 نسخه 2.x یا 3.x
 * کتابخانهٔ اضافه لازم نیست (WiFi, WebServer, ESPmDNS جزو هسته‌اند).
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "index_html.h"

// ====== تنظیمات - این‌ها را تغییر دهید ======
const char* STA_SSID     = "iPhone";      // نام وای‌فای خانه
const char* STA_PASSWORD = "123456789";  // رمز وای‌فای خانه
const char* AP_SSID      = "EE-Workbench";        // نام وای‌فای ESP32 در حالت AP
const char* AP_PASSWORD  = "12345678";            // حداقل ۸ کاراکتر؛ برای شبکهٔ باز "" بگذارید
const char* HOST_NAME    = "eeworkbench";         // http://eeworkbench.local
const uint32_t STA_TIMEOUT_MS = 15000;            // مهلت اتصال به مودم
// =============================================

WebServer server(80);

void handleRoot() {
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Cache-Control", "public, max-age=86400");
  server.send_P(200, "text/html; charset=utf-8",
                (const char*)INDEX_HTML_GZ, INDEX_HTML_GZ_LEN);
}

void handleNotFound() {
  // هر مسیر ناشناخته (مثلاً صفحهٔ تشخیص اتصال گوشی) به صفحهٔ اصلی هدایت شود
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

bool connectSTA() {
  if (strlen(STA_SSID) == 0 || String(STA_SSID) == "YOUR_WIFI_NAME") return false;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOST_NAME);
  WiFi.begin(STA_SSID, STA_PASSWORD);
  Serial.printf("Connecting to %s ", STA_SSID);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < STA_TIMEOUT_MS) {
    delay(300);
    Serial.print('.');
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void startAP() {
  WiFi.mode(WIFI_AP);
  if (strlen(AP_PASSWORD) >= 8) WiFi.softAP(AP_SSID, AP_PASSWORD);
  else                          WiFi.softAP(AP_SSID);
  Serial.printf("AP started. SSID: %s\n", AP_SSID);
  Serial.print("Open: http://");
  Serial.println(WiFi.softAPIP());   // معمولاً 192.168.4.1
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nEE Workbench - ESP32");

  if (connectSTA()) {
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
    if (MDNS.begin(HOST_NAME)) {
      MDNS.addService("http", "tcp", 80);
      Serial.printf("Open: http://%s.local\n", HOST_NAME);
    }
  } else {
    Serial.println("WiFi connection failed -> starting Access Point");
    startAP();
  }

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
  delay(1);
}
