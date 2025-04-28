#include "HX711.h"
#include "WiFi.h"
#include "WebServer.h"

const int DOUT = 13;
const int CLK = 12;
const int trigPin = 14;
const int echoPin = 27;

const char* ssid = "Totalplay-A752";
const char* password = "A75246AEz4mjbC5Z";

float duracion;
float referencia_cm;

WebServer servidor(80);
HX711 balanza;

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin,INPUT);
  balanza.begin(DOUT, CLK);
  configurarUltrasonico();
  configurarBalanza();
  
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a WiFi. IP: ");
  Serial.println(WiFi.localIP());

  servidor.on("/", manejarRaiz);
  servidor.begin();
  Serial.println("Servidor web iniciado");
  configurarUltrasonico();
  configurarBalanza();
  delay(1000);

}

void loop() {
  servidor.handleClient();
  float altura = referencia_cm-(medirDistancia());
  float peso = medirPeso();
  int bmi;
  Serial.print("altura: ");
  Serial.print(altura);
  Serial.println(" cm");
  Serial.print("peso: ");
  Serial.print(peso);
  Serial.println(" kg");
  bmi = calcularBMI(altura, peso);
  Serial.print("bmi: ");
  Serial.print(bmi);
  Serial.println(" kg/m^2");
}

void configurarUltrasonico(){
  // toma una medicion de altura como referencia, a partir del
  // primer segundo de arranque
  referencia_cm = medirDistancia()+5;
  Serial.print("Medición de referencia: ");
  Serial.print(referencia_cm);
  Serial.println(" cm");
}

void configurarBalanza(){
  Serial.print("Lectura del valor del ADC:t");
  Serial.println(balanza.read());
  Serial.println("No ponga ningún objeto sobre la balanza");
  Serial.println("Destarando...");
  balanza.set_scale(23342.38); //La escala por defecto es 1
  balanza.tare(20); //El peso actual es considerado Tara.
  Serial.println("Coloque un peso conocido:");
}

float medirPeso(){
  return balanza.get_units(20);
}

int calcularBMI(float altura, float peso){
  return peso / ((altura/100)*(altura/100));
}

float medirDistancia(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duracion = pulseIn(echoPin, HIGH);
  float distancia = duracion * 0.0343 / 2; // Conversión a cm
  return distancia;
}

void manejarRaiz() {
  float altura = referencia_cm - medirDistancia();
  float peso = medirPeso();
  int bmi = calcularBMI(altura,peso);

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Mediciones</title></head><body>";
  html += "<h1>FitMetrics</h1>";
  html += "<p><strong>Altura:</strong> " + String(altura, 2) + " cm</p>";
  html += "<p><strong>Peso:</strong> " + String(peso, 2) + " kg</p>";
  html += "<p><strong>IMC:</strong> " + String(bmi) + " kg/m²</p>";
  html += "<button onclick='location.reload()'>Actualizar</button>";
  html += "</body></html>";

  servidor.send(200, "text/html", html);
}