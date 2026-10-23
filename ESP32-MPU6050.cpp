#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <MPU6050.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

// ==== Configuración WiFi ====
const char* ssid     = "tu_ssid";
const char* password = "tu_password";

// ==== Servidores ====
WebSocketsServer webSocket = WebSocketsServer(81);
AsyncWebServer   httpServer(80);

// ==== MPU6050 ====
MPU6050 mpu;

// ==== Filtro complementario ====
float roll = 0, pitch = 0, yaw = 0;
const float alpha = 0.98f;

// ==== Buffers para derivadas numéricas ====
struct Vec3 { float x, y, z; };
Vec3 r_prev2 = {0,0,0};
Vec3 r_prev1 = {0,0,0};
Vec3 r_curr  = {0,0,0};
Vec3 v_curr  = {0,0,0};

unsigned long t_prev = 0;

// ==== Funciones vectoriales ====
Vec3 sub(const Vec3&a, const Vec3&b){ return {a.x-b.x, a.y-b.y, a.z-b.z}; }
Vec3 add(const Vec3&a, const Vec3&b){ return {a.x+b.x, a.y+b.y, a.z+b.z}; }
Vec3 scale(const Vec3&a, float s){ return {a.x*s, a.y*s, a.z*s}; }
float dot(const Vec3&a, const Vec3&b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
Vec3 cross(const Vec3&a, const Vec3&b){
  return { a.y*b.z - a.z*b.y,
           a.z*b.x - a.x*b.z,
           a.x*b.y - a.y*b.x };
}
float norm(const Vec3&a){ return sqrtf(dot(a,a)); }
Vec3 normalize(const Vec3&a){
  float n = norm(a);
  if (n < 1e-6f) return {0,0,0};
  return {a.x/n, a.y/n, a.z/n};
}

// ==== WebSocket event ====
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.printf("[WS] Cliente %u conectado\n", num);
  } else if (type == WStype_DISCONNECTED) {
    Serial.printf("[WS] Cliente %u desconectado\n", num);
  }
}

void setup() {
  Serial.begin(115200);

  // ---- LittleFS ----
  if (!LittleFS.begin(true)) {
    Serial.println("Error montando LittleFS");
  } else {
    Serial.println("LittleFS OK");
  }

  Wire.begin(21, 22);

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 no responde!");
    while(1) delay(10);
  }
  Serial.println("MPU6050 OK");

  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print(">>> ABRE EN EL NAVEGADOR: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  // ---- HTTP: sirve index.html desde LittleFS ----
  httpServer.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send(LittleFS, "/index.html", "text/html");
  });
  httpServer.onNotFound([](AsyncWebServerRequest *req){
    req->send(404, "text/plain", "404");
  });
  httpServer.begin();

  // ---- WebSocket ----
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  t_prev = micros();
}

void loop() {
  webSocket.loop();

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float axf = ax / 16384.0f;
  float ayf = ay / 16384.0f;
  float azf = az / 16384.0f;
  float gxf = gx / 131.0f * DEG_TO_RAD;
  float gyf = gy / 131.0f * DEG_TO_RAD;
  float gzf = gz / 131.0f * DEG_TO_RAD;

  unsigned long t_now = micros();
  float dt = (t_now - t_prev) * 1e-6f;
  t_prev = t_now;
  if (dt <= 0 || dt > 0.2f) dt = 0.01f;

  float rollAcc  = atan2f(ayf, azf);
  float pitchAcc = atan2f(-axf, sqrtf(ayf*ayf + azf*azf));

  roll  = alpha * (roll  + gxf * dt) + (1 - alpha) * rollAcc;
  pitch = alpha * (pitch + gyf * dt) + (1 - alpha) * pitchAcc;
  yaw  += gzf * dt;

  Vec3 acc = { axf, ayf, azf };
  v_curr = add(v_curr, scale(acc, dt));
  v_curr = scale(v_curr, 0.95f);
  Vec3 newPos = add(r_curr, scale(v_curr, dt));

  r_prev2 = r_prev1;
  r_prev1 = r_curr;
  r_curr  = newPos;

  Vec3 r1 = scale(sub(r_curr, r_prev1), 1.0f/dt);
  Vec3 r2 = scale(sub(r_curr, add(scale(r_prev1,2.0f), scale(r_prev2,-1.0f))), 1.0f/(dt*dt));
  static Vec3 r2_prev = {0,0,0};
  Vec3 r3 = scale(sub(r2, r2_prev), 1.0f/dt);
  r2_prev = r2;

  Vec3 T = normalize(r1);
  Vec3 r2_perp = sub(r2, scale(T, dot(r2, T)));
  Vec3 N = normalize(r2_perp);
  if (norm(N) < 0.1f) { N = {0,1,0}; }
  Vec3 B = normalize(cross(T, N));

  float speed = norm(r1);
  float kappa = 0.0f, radius = 0.0f, tau = 0.0f;
  Vec3 r1xr2 = cross(r1, r2);
  float denom = norm(r1xr2);
  if (speed > 1e-3f) kappa = denom / (speed*speed*speed);
  if (kappa > 1e-4f) radius = 1.0f/kappa;
  if (denom > 1e-4f) tau = dot(r1xr2, r3) / (denom*denom);

  StaticJsonDocument<512> doc;
  doc["ax"] = axf; doc["ay"] = ayf; doc["az"] = azf;
  doc["gx"] = gxf; doc["gy"] = gyf; doc["gz"] = gzf;
  doc["roll"]  = roll  * RAD_TO_DEG;
  doc["pitch"] = pitch * RAD_TO_DEG;
  doc["yaw"]   = yaw   * RAD_TO_DEG;

  JsonObject Tn = doc.createNestedObject("T");
  Tn["x"]=T.x; Tn["y"]=T.y; Tn["z"]=T.z;
  JsonObject Nn = doc.createNestedObject("N");
  Nn["x"]=N.x; Nn["y"]=N.y; Nn["z"]=N.z;
  JsonObject Bn = doc.createNestedObject("B");
  Bn["x"]=B.x; Bn["y"]=B.y; Bn["z"]=B.z;

  doc["kappa"]  = kappa;
  doc["radius"] = radius;
  doc["tau"]    = tau;

  String out;
  serializeJson(doc, out);
  webSocket.broadcastTXT(out);

  delay(20);
}