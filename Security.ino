#include <ESP8266WiFi.h>                                                // Библиотека для создания Wi-Fi подключения (клиент или точка доступа)
#include <ESP8266WebServer.h>                                           // Библиотека для управления устройством по HTTP (например из браузера)
#include <FS.h>                                                         // Библиотека для работы с файловой системой
#include <ESP8266FtpServer.h>                                           // Библиотека для работы с SPIFFS по FTP

const byte security_active = 1;                                                   // Пин подключения сигнального контакта реле
const byte look_active = 13;
const byte look_noactive = 2;
const char *ssid = "VAZ-2103";                                          // Название генерируемой точки доступа
const char *passPhrase = "DGYvaz03091995";

ESP8266WebServer HTTP(80);                                              // Определяем объект и порт сервера для работы с HTTP
FtpServer ftpSrv;                                                       // Определяем объект для работы с модулем по FTP (для отладки HTML)

void setup() {
   Serial.begin(9600);
  pinMode(security_active,OUTPUT);                                                // Определяем пин реле как исходящий                                                   // Инициализируем вывод данных на серийный порт со скоростью 9600 бод
  pinMode(look_active,OUTPUT);
  pinMode(look_noactive,OUTPUT);
  WiFi.softAP(ssid, passPhrase);                                        // Создаём точку доступа
  digitalWrite(security_active, LOW);
  digitalWrite(look_active, HIGH);
  digitalWrite(look_noactive, HIGH);
  
  SPIFFS.begin();                                                       // Инициализируем работу с файловой системой                          
  HTTP.begin();                                                         // Инициализируем Web-сервер
  ftpSrv.begin("relay","relay");                                        // Поднимаем FTP-сервер для удобства отладки работы HTML (логин: relay, пароль: relay)

  Serial.print("\nMy IP to connect via Web-Browser or FTP: ");          // Выводим на монитор серийного порта сообщение о том, что сейчас будем выводить локальный IP
  Serial.println(WiFi.softAPIP());                                      // Выводим локальный IP-адрес ESP8266
  Serial.println("\n");

// Обработка HTTP-запросов
  HTTP.on("/security_switch_active", [](){                                        // При HTTP запросе вида http://192.168.4.1/relay_switch
      HTTP.send(200, "text/plain", security_switch_active());                     // Отдаём клиенту код успешной обработки запроса, сообщаем, что формат ответа текстовый и возвращаем результат выполнения функции relay_switch 
  });
  HTTP.on("/security_switch_noactive", [](){                                        // При HTTP запросе вида http://192.168.4.1/relay_switch
      HTTP.send(200, "text/plain", security_switch_noactive());                     // Отдаём клиенту код успешной обработки запроса, сообщаем, что формат ответа текстовый и возвращаем результат выполнения функции relay_switch 
  });
  HTTP.on("/look_on", [](){                                        // При HTTP запросе вида http://192.168.4.1/relay_switch
      HTTP.send(200, "text/plain", look_on());                     // Отдаём клиенту код успешной обработки запроса, сообщаем, что формат ответа текстовый и возвращаем результат выполнения функции relay_switch 
  });
   HTTP.on("/look_off", [](){                                        // При HTTP запросе вида http://192.168.4.1/relay_switch
      HTTP.send(200, "text/plain", look_off());                     // Отдаём клиенту код успешной обработки запроса, сообщаем, что формат ответа текстовый и возвращаем результат выполнения функции relay_switch 
  });
  HTTP.onNotFound([](){                                                 // Описываем действия при событии "Не найдено"
  if(!handleFileRead(HTTP.uri()))                                       // Если функция handleFileRead (описана ниже) возвращает значение false в ответ на поиск файла в файловой системе
      HTTP.send(404, "text/plain", "Not Found");                        // возвращаем на запрос текстовое сообщение "File isn't found" с кодом 404 (не найдено)
  });
}

void loop() {
    HTTP.handleClient();                                                // Обработчик HTTP-событий (отлавливает HTTP-запросы к устройству и обрабатывает их в соответствии с выше описанным алгоритмом)
    ftpSrv.handleFTP();                                                 // Обработчик FTP-соединений  
}



String security_switch_active() {                                                 // Функция переключения реле 
byte security_state;                                                    //  запоминаем, что надо поменять на высокий
 digitalWrite(security_active, HIGH);                                          // меняем значение на пине подключения реле
 digitalWrite(look_active, LOW);
 delay(250);
 digitalWrite(look_active, HIGH);
 return String(security_state);                                                 // возвращаем результат, преобразовав число в строку
}
String security_switch_noactive() {                                                 // Функция переключения реле 
byte security_state;                                                    //  запоминаем, что надо поменять на высокий
 digitalWrite(security_active, LOW);                                          // меняем значение на пине подключения реле
 digitalWrite(look_noactive, LOW);
 delay(250);
 digitalWrite(look_noactive, HIGH);
 return String(security_state);                                                 // возвращаем результат, преобразовав число в строку
}
String look_on() {                                                 // Функция переключения реле 
byte security_state;                                                    //  запоминаем, что надо поменять на высокий                                          // меняем значение на пине подключения реле                                                    //  запоминаем, что надо поменять на высокий
 digitalWrite(security_active, HIGH);                                          // меняем значение на пине подключения реле
 digitalWrite(look_active, LOW);
 delay(250);
 digitalWrite(look_active, HIGH);
 return String(security_state);                                         // возвращаем результат, преобразовав число в строку
}
String look_off() {                                                 // Функция переключения реле 
byte security_state;                                                    //  запоминаем, что надо поменять на высокий
 digitalWrite(security_active, LOW);                                          // меняем значение на пине подключения реле
 digitalWrite(look_noactive, LOW);
 delay(250);
 digitalWrite(look_noactive, HIGH);
 return String(security_state);                                                 // возвращаем результат, преобразовав число в строку
}




bool handleFileRead(String path){                                       // Функция работы с файловой системой
  if(path.endsWith("/")) path += "index.html";                          // Если устройство вызывается по корневому адресу, то должен вызываться файл index.html (добавляем его в конец адреса)
  String contentType = getContentType(path);                            // С помощью функции getContentType (описана ниже) определяем по типу файла (в адресе обращения) какой заголовок необходимо возвращать по его вызову
  if(SPIFFS.exists(path)){                                              // Если в файловой системе существует файл по адресу обращения
    File file = SPIFFS.open(path, "r");                                 //  Открываем файл для чтения
    size_t sent = HTTP.streamFile(file, contentType);                   //  Выводим содержимое файла по HTTP, указывая заголовок типа содержимого contentType
    file.close();                                                       //  Закрываем файл
    return true;                                                        //  Завершаем выполнение функции, возвращая результатом ее исполнения true (истина)
  }
  return false;                                                         // Завершаем выполнение функции, возвращая результатом ее исполнения false (если не обработалось предыдущее условие)
}

String getContentType(String filename){                                 // Функция, возвращающая необходимый заголовок типа содержимого в зависимости от расширения файла
  if (filename.endsWith(".html")) return "text/html";                   // Если файл заканчивается на ".html", то возвращаем заголовок "text/html" и завершаем выполнение функции
  else if (filename.endsWith(".css")) return "text/css";                // Если файл заканчивается на ".css", то возвращаем заголовок "text/css" и завершаем выполнение функции
  else if (filename.endsWith(".js")) return "application/javascript";   // Если файл заканчивается на ".js", то возвращаем заголовок "application/javascript" и завершаем выполнение функции
  else if (filename.endsWith(".png")) return "image/png";               // Если файл заканчивается на ".png", то возвращаем заголовок "image/png" и завершаем выполнение функции
  else if (filename.endsWith(".jpg")) return "image/jpeg";              // Если файл заканчивается на ".jpg", то возвращаем заголовок "image/jpg" и завершаем выполнение функции
  else if (filename.endsWith(".gif")) return "image/gif";               // Если файл заканчивается на ".gif", то возвращаем заголовок "image/gif" и завершаем выполнение функции
  else if (filename.endsWith(".ico")) return "image/x-icon";            // Если файл заканчивается на ".ico", то возвращаем заголовок "image/x-icon" и завершаем выполнение функции
  return "text/plain";                                                  // Если ни один из типов файла не совпал, то считаем что содержимое файла текстовое, отдаем соответствующий заголовок и завершаем выполнение функции
}