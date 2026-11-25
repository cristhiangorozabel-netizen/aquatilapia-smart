#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>

// ============================================================================
// 1. CONFIGURACION DE RED WI-FI Y BASE DE DATOS EN LA NUBE (FIREBASE)
// Comparte internet desde tu celular o usa el Wi-Fi del laboratorio/expoferia
// ============================================================================
const char* WIFI_SSID = "TU_RED_WIFI";
const char* WIFI_PASS = "TU_CLAVE_WIFI";

// Pega aqui la URL de tu Firebase Realtime Database (sin barra / al final)
// Ejemplo: "https://aquatilapia-iot-default-rtdb.firebaseio.com"
const char* FIREBASE_HOST = "https://aquatilapia-iot-default-rtdb.firebaseio.com";

// ============================================================================
// 2. CONTROLADOR NATIVO I2C PARA PANTALLA LCD 16x2 (SIN LIBRERIAS EXTERNAS)
// ============================================================================
class LCD_I2C : public Print {
  uint8_t _addr;
  uint8_t _bl = 0x08;
  void expWrite(uint8_t data) {
    Wire.beginTransmission(_addr);
    Wire.write(data | _bl);
    Wire.endTransmission();
  }
  void pulseEn(uint8_t data) {
    expWrite(data | 0x04);
    delayMicroseconds(1);
    expWrite(data & ~0x04);
    delayMicroseconds(50);
  }
  void write4(uint8_t nibble) {
    expWrite(nibble);
    pulseEn(nibble);
  }
  void sendByte(uint8_t val, uint8_t mode) {
    write4((val & 0xF0) | mode);
    write4(((val << 4) & 0xF0) | mode);
  }
public:
  LCD_I2C(uint8_t addr, uint8_t cols, uint8_t rows) : _addr(addr) {}
  void init() {
    Wire.begin(21, 22);
    delay(50);
    expWrite(0x00);
    delay(100);
    write4(0x30);
    delayMicroseconds(4500);
    write4(0x30);
    delayMicroseconds(4500);
    write4(0x30);
    delayMicroseconds(150);
    write4(0x20);
    sendByte(0x28, 0);
    sendByte(0x0C, 0);
    sendByte(0x06, 0);
    clear();
  }
  void backlight() { _bl = 0x08; expWrite(0); }
  void clear() { sendByte(0x01, 0); delayMicroseconds(2000); }
  void setCursor(uint8_t col, uint8_t row) {
    uint8_t offsets[] = {0x00, 0x40};
    sendByte(0x80 | (col + offsets[row & 0x01]), 0);
  }
  virtual size_t write(uint8_t val) {
    sendByte(val, 0x01);
    return 1;
  }
};

LCD_I2C lcd(0x27, 16, 2);

// ============================================================================
// 3. DEFINICION DE PINES DEL CIRCUITO FISICO ESP32
// ============================================================================
const int PIN_POT_O2     = 36; // Potenciometro simulador O2 (VP)
const int PIN_POT_TEMP   = 39; // Potenciometro simulador Temperatura (VN)
const int PIN_HUM_TIERRA = 34; // Sensor real de humedad de sustrato
const int PIN_TRIG       = 5;  // Ultrasonico HC-SR04 TRIG
const int PIN_ECHO       = 19; // Ultrasonico HC-SR04 ECHO
const int PIN_BTN        = 18; // Pulsador fisico (Pagina LCD / Modo AUT-MAN)

const int PIN_LED_VERDE   = 25; // Indicador parametros optimos
const int PIN_LED_ROJO    = 26; // Indicador alerta
const int PIN_BUZZER_O2   = 27; // Alarma sonora oxigeno critico
const int PIN_BUZZER_AGUA = 32; // Alarma sonora nivel bajo de agua

const int PIN_RELE_AIRE  = 23; // Canal 1 Rele: Aireador continuo 24/7
const int PIN_RELE_BOMBA = 4;  // Canal 2 Rele: Mini bomba recirculacion

#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// Umbrales biologicos y operativos para cultivo de tilapia
const float O2_MINIMO         = 5.0;
const float TEMP_MINIMA       = 25.0;
const float TEMP_MAXIMA       = 30.0;
const float WATER_MAX_DIST_CM = 15.0;
const float HUM_MAX_RIEGO     = 75.0;

// Variables de temporizador y estado
unsigned long tiempoRiegoSeg  = 10;
unsigned long tiempoEsperaSeg = 20;
bool faseRiegoActiva          = true;
unsigned long inicioFaseRiego = 0;
unsigned long segRestantes    = 10;
int estadoRiegoCode           = 2;

bool manualMode            = false;
int  lcdPage               = 0;
bool lastBtnState          = HIGH;
unsigned long btnPressTime = 0;
bool longPressHandled      = false;
unsigned long lastUpdate   = 0;
unsigned long lastCloudSync = 0;

float oxigeno       = 0.0;
float temperatura   = 0.0;
float humedadTierra = 0.0;
float distAgua      = 0.0;
bool estadoAireador = true;
bool estadoBomba    = false;
bool alertaGeneral  = false;

void conectarWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    lcd.setCursor(intentos % 16, 1);
    lcd.print(".");
    intentos++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Conectado!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Modo Local Activo");
    lcd.setCursor(0, 1);
    lcd.print("Sin red WiFi");
  }
  delay(1500);
  lcd.clear();
}

void enviarDatosNube() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String url = String(FIREBASE_HOST) + "/telemetria/actual.json";
  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");

    unsigned long uptimeSec = millis() / 1000UL;
    String json = "{";
    json += "\"o2\":" + String(oxigeno, 1) + ",";
    json += "\"temp\":" + String(temperatura, 1) + ",";
    json += "\"hum\":" + String(humedadTierra, 0) + ",";
    json += "\"nivel\":" + String(distAgua, 1) + ",";
    json += "\"aire\":" + String(estadoAireador ? "true" : "false") + ",";
    json += "\"bomba\":" + String(estadoBomba ? "true" : "false") + ",";
    json += "\"manual\":" + String(manualMode ? "true" : "false") + ",";
    json += "\"alerta\":" + String(alertaGeneral ? "true" : "false") + ",";
    json += "\"rcode\":" + String(estadoRiegoCode) + ",";
    json += "\"rem\":" + String(segRestantes) + ",";
    json += "\"uptime\":" + String(uptimeSec);
    json += "}";

    http.PUT(json);
    http.end();
  }
}

void setup() {
  Serial.begin(115200);

  digitalWrite(PIN_RELE_AIRE, RELAY_ON);
  digitalWrite(PIN_RELE_BOMBA, RELAY_OFF);
  pinMode(PIN_RELE_AIRE, OUTPUT);
  pinMode(PIN_RELE_BOMBA, OUTPUT);

  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_BUZZER_O2, OUTPUT);
  pinMode(PIN_BUZZER_AGUA, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AQUATILAPIA IOT");
  lcd.setCursor(0, 1);
  lcd.print("BioAquaTech EC");
  delay(1500);

  conectarWiFi();
  inicioFaseRiego = millis();
}

float medirDistanciaAgua() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duracion = pulseIn(PIN_ECHO, HIGH, 30000);
  if (duracion == 0) return 99.0;
  return (duracion * 0.0343) / 2.0;
}

void revisarBoton() {
  bool btnState = digitalRead(PIN_BTN);

  if (btnState == LOW && lastBtnState == HIGH) {
    btnPressTime = millis();
    longPressHandled = false;
  }

  if (btnState == LOW && !longPressHandled && (millis() - btnPressTime >= 1500)) {
    manualMode = !manualMode;
    longPressHandled = true;
    faseRiegoActiva = true;
    inicioFaseRiego = millis();
    lcd.clear();
  }

  if (btnState == HIGH && lastBtnState == LOW) {
    if (!longPressHandled && (millis() - btnPressTime >= 50)) {
      lcdPage = (lcdPage + 1) % 2;
      lcd.clear();
    }
  }

  lastBtnState = btnState;
}

void loop() {
  revisarBoton();

  unsigned long ahora = millis();
  unsigned long duracionFaseMs = (faseRiegoActiva ? tiempoRiegoSeg : tiempoEsperaSeg) * 1000UL;
  if (ahora - inicioFaseRiego >= duracionFaseMs) {
    faseRiegoActiva = !faseRiegoActiva;
    inicioFaseRiego = ahora;
    duracionFaseMs = (faseRiegoActiva ? tiempoRiegoSeg : tiempoEsperaSeg) * 1000UL;
  }
  segRestantes = (duracionFaseMs - (ahora - inicioFaseRiego) + 999UL) / 1000UL;

  // Lectura de sensores y control cada 300 ms
  if (ahora - lastUpdate >= 300) {
    lastUpdate = ahora;

    int rawO2   = analogRead(PIN_POT_O2);
    int rawTemp = analogRead(PIN_POT_TEMP);
    int rawHum  = analogRead(PIN_HUM_TIERRA);

    oxigeno       = (rawO2 / 4095.0) * 10.0;
    temperatura   = 15.0 + (rawTemp / 4095.0) * 20.0;
    humedadTierra = ((4095.0 - rawHum) / (4095.0 - 1300.0)) * 100.0;
    if (humedadTierra < 0.0) humedadTierra = 0.0;
    if (humedadTierra > 100.0) humedadTierra = 100.0;

    distAgua = medirDistanciaAgua();

    bool faltaOxigeno = (oxigeno < O2_MINIMO);
    bool tempFuera    = (temperatura < TEMP_MINIMA || temperatura > TEMP_MAXIMA);
    bool faltaAgua    = (distAgua > WATER_MAX_DIST_CM);
    bool tierraSeca   = (humedadTierra < HUM_MAX_RIEGO);

    estadoAireador = true;
    digitalWrite(PIN_RELE_AIRE, RELAY_ON);

    if (manualMode) {
      estadoBomba     = true;
      alertaGeneral   = false;
      estadoRiegoCode = 2;

      digitalWrite(PIN_RELE_BOMBA, RELAY_ON);
      digitalWrite(PIN_BUZZER_O2, LOW);
      digitalWrite(PIN_BUZZER_AGUA, LOW);
      digitalWrite(PIN_LED_VERDE, HIGH);
      digitalWrite(PIN_LED_ROJO, LOW);
    } else {
      digitalWrite(PIN_BUZZER_O2, faltaOxigeno ? HIGH : LOW);

      if (faltaAgua) {
        estadoBomba     = false;
        estadoRiegoCode = 0;
        digitalWrite(PIN_RELE_BOMBA, RELAY_OFF);
        digitalWrite(PIN_BUZZER_AGUA, HIGH);
      } else if (!tierraSeca) {
        estadoBomba     = false;
        estadoRiegoCode = 1;
        digitalWrite(PIN_RELE_BOMBA, RELAY_OFF);
        digitalWrite(PIN_BUZZER_AGUA, LOW);
      } else {
        digitalWrite(PIN_BUZZER_AGUA, LOW);
        if (faseRiegoActiva) {
          estadoBomba     = true;
          estadoRiegoCode = 2;
          digitalWrite(PIN_RELE_BOMBA, RELAY_ON);
        } else {
          estadoBomba     = false;
          estadoRiegoCode = 3;
          digitalWrite(PIN_RELE_BOMBA, RELAY_OFF);
        }
      }

      alertaGeneral = (faltaOxigeno || tempFuera || faltaAgua);
      if (alertaGeneral) {
        digitalWrite(PIN_LED_VERDE, LOW);
        digitalWrite(PIN_LED_ROJO, HIGH);
      } else {
        digitalWrite(PIN_LED_VERDE, HIGH);
        digitalWrite(PIN_LED_ROJO, LOW);
      }
    }

    if (lcdPage == 0) {
      lcd.setCursor(0, 0);
      lcd.print("O2:");
      lcd.print(oxigeno, 1);
      lcd.print("mg/L ");
      lcd.setCursor(11, 0);
      lcd.print(manualMode ? "[MAN]" : "[AUT]");

      lcd.setCursor(0, 1);
      lcd.print("T:");
      lcd.print(temperatura, 1);
      lcd.print((char)223);
      lcd.print("C ");
      lcd.setCursor(9, 1);
      lcd.print("H:");
      lcd.print((int)humedadTierra);
      lcd.print("%  ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Niv:");
      if (distAgua >= 90.0) {
        lcd.print("ERR   ");
      } else {
        lcd.print(distAgua, 1);
        lcd.print("cm ");
      }
      lcd.setCursor(11, 0);
      lcd.print(manualMode ? "[MAN]" : "[AUT]");

      lcd.setCursor(0, 1);
      lcd.print("A:");
      lcd.print(estadoAireador ? "ON " : "OFF");
      lcd.print(" B:");
      lcd.print(estadoBomba ? "ON " : "OFF");
      lcd.print(" T:");
      if (segRestantes < 10) lcd.print("0");
      lcd.print(segRestantes);
      lcd.print("s");
    }
  }

  // Envio de telemetria real a la web publica cada 2.5 segundos
  if (ahora - lastCloudSync >= 2500) {
    lastCloudSync = ahora;
    enviarDatosNube();
  }
}
