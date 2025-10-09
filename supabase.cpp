#include "supabase.h"
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

const char* ssid = "leonardo";
const char* password = "14159265358";

const char* supabaseHost = "oblxfxybbcfumqrvdxyc.supabase.co";
const int httpsPort = 443;

const char* supabaseKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im9ibHhmeHliYmNmdW1xcnZkeHljIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NTk5NDI4NTAsImV4cCI6MjA3NTUxODg1MH0.hyJwVEbkXdyz8IZvXs0hOAgHVfpPyHcKhCDAE9D8jDQ";

WiFiSSLClient wifiSSLClient;
HttpClient client = HttpClient(wifiSSLClient, supabaseHost, httpsPort);

void conectarWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado ao WiFi!");
}

void inserirNumeroNoSupabase(int codigo, int rampa) {
  String jsonData = "{\"codigo\": " + String(codigo) + ", \"rampa\": " + String(rampa) + "}";
  String endpoint = "/rest/v1/dados_teste";

  client.beginRequest();
  client.post(endpoint);
  client.sendHeader("apikey", supabaseKey);
  client.sendHeader("Authorization", String("Bearer ") + supabaseKey);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Prefer", "return=representation");
  client.sendHeader("Content-Length", jsonData.length());
  client.beginBody();
  client.print(jsonData);
  client.endRequest();

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.println("Resposta:");
  Serial.println(response);

  if (statusCode == 201) {
    Serial.println("✅ Inserção realizada com sucesso!");
  } else {
    Serial.println("❌ Erro na inserção. Verifique permissões e dados.");
  }
}
