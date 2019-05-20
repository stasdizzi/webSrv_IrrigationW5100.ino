// Скетч написан для ситемы полива огорода/газона
// Железо Arduino Uno + Arduino Ethernet Shield W5100 + capacitive soil moisture v1.2
// к выходу WATER_RELE подключено реле, которое открывает клапан полива

#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define WATER_RELE 2
#define ONE_WIRE_BUS 8
#define WATER_SENSOR A4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long ReleOnTime;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
    0xA8, 0x61, 0x0A, 0x4F, 0xFA, 0xED};
IPAddress ip(192, 168, 1, 98);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup()
{

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
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Irrigation system");
  sensors.begin();

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop()
{
  
  int val = analogRead(WATER_SENSOR); //слушаю аналоговый вход

  int temp = sensors.getTempCByIndex(0);

  val = map(val, 597, 287, 0, 100); //пересчитываю в проценты и создаю переменную
 
  sensors.requestTemperatures();
  
  bool releOn;
  int start;
  if (temp < 0)
  {
    releOn = false;
    start = 0;
  }
  else
  {
    if (millis() - ReleOnTime > 30000){
    start = pow(temp, 1.2);
    releOn = val < start;
     ReleOnTime = millis();
    }
    }
  
  digitalWrite(WATER_RELE, releOn ? LOW : HIGH);

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client)
  {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html; charset=utf-8"); //обязательно кодировка шрифта для "не Mac" систем
          client.println("Connection: close");                      // the connection will be closed after completion of the response
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<TITLE>Irrigation system</TITLE>");
          client.println("<body><H1>Оросительная система 3 этаж</H1>");
          client.println("<BODY bgcolor=black text=lime>");
          client.print("Относительная влажность грунта: ");
          client.print(val);
          client.print(" % ");
          client.println("<br />");
          client.print("Температура грунта: ");
          client.print(temp);
          client.print(" C ");
          client.println("<br />");

          if (releOn)
          {
            client.print("Идёт полив");           
          }
          else
          {
            client.print("Влажность грунта достаточная, полив включится при: ");
            client.print(start);
            client.print(" % ");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
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
