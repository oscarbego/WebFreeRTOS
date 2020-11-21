#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

#include <queue.h>

#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"

static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t ip[] = { 192, 168, 2, 34 };

#define PREFIX ""
WebServer webserver(PREFIX, 80);


struct Boton {
  int id;
  //char nombre[15]; //btn + Alta/Baja, bomba, On/Off
};

QueueHandle_t cola_Mensaje;

void TaskWs( void *pvParameters );
void TaskBlink( void *pvParameters );
void Actuador(void *pvParameter);
void on_off (void *pvParameter);


void helloCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  server.httpSuccess();
  struct Boton boton;
  
  if (type != WebServer::HEAD)
  {
    boton.id = 0;
    
    if (xQueueSend(cola_Mensaje, &boton, 2000 / portTICK_PERIOD_MS) != pdTRUE)
    {
        Serial.println("error");  
    }
      
    P(helloMsg) = "<h1>El comando a sido enviado</h1>";
    server.printP(helloMsg);
  }
}

void setup()
{

  Serial.begin(9600);
  Ethernet.begin(mac, ip);

  webserver.setDefaultCommand(&helloCmd);
  webserver.addCommand("index.html", &helloCmd);
  webserver.begin();


  cola_Mensaje = xQueueCreate(10, // Queue length
                              sizeof(struct Boton) // Queue item size
                              );
   
/*
    xTaskCreate(
    TaskBlink
    ,  "Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

*/
    xTaskCreate(
    TaskWs
    ,  "TaskWs"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );


    xTaskCreate(
    Actuador
    ,  "Actuador"   // A name just for humans
    ,  64  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

/*
    xTaskCreate(
    on_off
    ,  "on_off"   // A name just for humans
    ,  64  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );


    */
}

void loop()
{

}

void on_off( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  struct Boton boton = { 0 };
  
  uint8_t pushButton = 2;

  pinMode(pushButton, INPUT);

  for (;;) // A Task shall never return or exit.
  {
    int buttonState = digitalRead(pushButton);

    if ( buttonState == 1 )
    {     
      if (xQueueSend(cola_Mensaje, &boton, 2000 / portTICK_PERIOD_MS) != pdTRUE)
      {
        Serial.println("error");  
      }
          
      vTaskDelay( 500 / portTICK_PERIOD_MS );
    }
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}



void TaskWs(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
  for (;;) // A Task shall never return or exit.
  {
    webserver.processConnection();
    vTaskDelay(10);
  }
  
  vTaskDelay(10);
}


void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}


void Actuador(void * pvParameters) {

  (void) pvParameters;
  
  pinMode(12, OUTPUT);
  
  bool bOn_Off = false;
  bool bAlta_Baja = false;
  bool bBomba = false;

  struct Boton boton;
  
  for (;;) 
  {
     
   
    if (xQueueReceive(cola_Mensaje, &boton, portMAX_DELAY) == pdPASS) {
      //Serial.print("Id: ");
      //Serial.println(boton.id);   

         if(boton.id == 0)
         {
            Serial.println("Se apaga o prende el Aire");

            if(digitalRead(12) == LOW)            
              digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)                          
            else            
              digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW              
            
            vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second                 
         }

         if(boton.id == 1)
         {
            Serial.println("Se cambia la velocidad del Aire");
         }
         
         if(boton.id == 2)
         {
            Serial.println("Se prende o apaga la Bomba");
         }
    }
  }

  vTaskDelay(1);
}
