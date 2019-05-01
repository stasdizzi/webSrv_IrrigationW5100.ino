
#include <SPI.h>
#include <Ethernet.h>
#define WATER_RELE 2
#define WATER_SENSOR A4



// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 98);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {

  pinMode(WATER_RELE, OUTPUT);
  
  
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Irrigation system");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {

   // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html; charset=utf-8");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // обновление страницы в сек.
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<TITLE>Irrigation system</TITLE>");
          client.println("<body><H1>Оросительная система 3 этаж</H1>");
          client.println("<BODY bgcolor=black text=lime>");
          
  int val = analogRead(WATER_SENSOR);//слушаю аналоговый вход
  //int absolut = analogRead(WATER_SENSOR); //прямое значение для юстировки сенсора
    
 
          val = map(val,606,313,0,100); //пересчитываю в проценты и создаю переменную
           delay (1000);
           
            
            client.print("Относительная влажность грунта: ");
            client.print(val);
            client.print(" % ");
            client.println("<br />");
          //  client.print(absolut);  //строка для юстировки датчика влажности
          //  client.println("<br />"); //строка для юстировки датчика влажности/
            if (val < 20) {
    digitalWrite(WATER_RELE, LOW);
            client.print("Идёт полив");
          delay (1000);
            }
            else {
    digitalWrite(WATER_RELE, HIGH);
             client.print("Полив остановлен, влажность грунта достаточная");

          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  }
  
