#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <M5GFX.h>
#include <epdiy.h>

static M5GFX display;

struct TaskTouch {
  int id;
  int y;
};

TaskTouch touchZones[15];

struct Task {
  int id;
  String title;
  bool completed;
};

Task taskList[15];
int taskCount = 0;

const int taskHeight = 36;  
const char* ssid = "Kek_lol";
const char* password = "vov10230412";

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

void update_list();
void update_display();
void delete_item(int id);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Hello World on PaperS3...");

  display.init();
  display.setEpdMode(epd_mode_t::epd_fastest);
  display.clearDisplay();
  display.display();      
  display.waitDisplay();

  // Configure text settings
  display.setTextSize(4);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setCursor(40, 40);

  display.println("Hello, world!");
  display.display();
  display.waitDisplay();

  connectToWiFi();

  update_list();
  update_display();
}
long lastUpdate = 0;

void loop() {
  
  int x, y;
  if (display.getTouchRaw(&x, &y)) {
    display.convertRawXY(&x, &y);
    Serial.printf("Touch at x=%d, y=%d\n", x, y);

    for (int i = 0; i < taskCount; i++) {
      int taskY = touchZones[i].y;
      if (y >= taskY && (y <= taskY + taskHeight)) {
        Serial.printf("Deleting task ID: %d\n", touchZones[i].id);
        delete_item(touchZones[i].id);
        break;
      }
    }

    delay(300);  
  }

  if (WiFi.status() == WL_CONNECTED && millis() - lastUpdate > 60000) {
    update_list();
    update_display();
    lastUpdate = millis();
  }

  delay(100);
}

void update_list() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://192.168.1.3:8000/api/tasks/");
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Отримано JSON:");
    Serial.println(payload);

    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      taskCount = 0;
      for (JsonObject task : doc.as<JsonArray>()) {
        if (taskCount < 15) {
          taskList[taskCount].id = task["id"];
          taskList[taskCount].title = task["title"].as<String>();
          taskList[taskCount].completed = task["completed"];
          taskCount++;
        }
      }
    } else {
      Serial.println("Помилка розбору JSON");
    }
  } else {
    Serial.println("Помилка GET-запиту");
  }

  http.end();
}

void delete_item(int id) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "http://192.168.1.3:8000/api/tasks/" + String(id);
  http.begin(url);

  int httpCode = http.sendRequest("DELETE");

  if (httpCode == 204 || httpCode == 200) {
    Serial.println("Завдання успішно видалено з сервера");
    update_list();
    update_display();
  } else {
    Serial.print("Помилка видалення: ");
    Serial.println(httpCode);
  }

  http.end();
}

void update_display() {
  Serial.println("Оновлення дисплея...");
  display.setEpdMode(epd_mode_t::epd_fastest); 
  display.clearDisplay();

  display.setTextSize(3);
  display.setTextColor(TFT_BLACK, TFT_WHITE);

  int margin = 20;
  int boxWidth = display.width() - 2 * margin;
  int boxHeight = display.height() - 2 * margin;
  display.drawRect(margin, margin, boxWidth, boxHeight, TFT_BLACK);

  int y = margin + 20;

  if (taskCount == 0) {
    display.setCursor(margin + 30, y + 40);
    display.setTextSize(2);
    display.println("Список порожній");
    Serial.println("Список порожній");
  }

  for (int i = 0; i < taskCount; i++) {
    String mark = taskList[i].completed ? "✅ " : "⬜ ";
    String text = mark + String(i + 1) + ". " + taskList[i].title;
    int textWidth = display.textWidth(text.c_str());
    int x = (display.width() - textWidth) / 2;

    display.setCursor(x, y);
    display.println(text);
    Serial.printf("Рендер завдання: %s\n", text.c_str());

  
    touchZones[i].id = taskList[i].id;
    touchZones[i].y = y;

    y += taskHeight;
  }

  display.display();
  display.waitDisplay();
  Serial.println("Оновлення завершено.");
}