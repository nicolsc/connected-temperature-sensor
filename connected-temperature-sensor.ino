#include <SoftwareSerial.h>
#include <Akeru.h>
#include <idDHT11.h>

#define DHT11_PIN 2
#define INTERRUPT_NUMBER 0

#define SERIAL_BAUD 9600
#define STARTUP_DELAY 2000
#define SIGFOX_LED_PIN 13
#define SIGFOX_POWER 5
#define LOOP_DELAY 600000

//sigfox data struct
struct sigfoxData{
  float temperature;
  float humidity;
  int error;
} data;

//declaration
void dht11_wrapper(); // must be declared before the lib initialization

// Lib instantiate
idDHT11 DHT11(DHT11_PIN,INTERRUPT_NUMBER,dht11_wrapper);

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(STARTUP_DELAY); //let the modem wake-up gently

  Akeru.begin();
  Akeru.setPower(SIGFOX_POWER);
  
  pinMode(SIGFOX_LED_PIN, OUTPUT);
  digitalWrite(SIGFOX_LED_PIN, LOW);
}
// This wrapper is in charge of calling 
// mus be defined like this for the lib work
void dht11_wrapper() {
  DHT11.isrCallback();
}
void loop()
{
  Serial.print("\nRetrieving information from sensor: ");
  Serial.print("Read sensor: ");
  
  sendData(DHT11.acquireAndWait());
  
  delay(LOOP_DELAY);
}
boolean sendData(int result){
  
  data.error = result;
  data.temperature = DHT11.getCelsius();
  data.humidity = DHT11.getHumidity();
  
  if (!Akeru.isReady()){
      Serial.println("Cannot send Sigfox message right now"); 
      Serial.println("Probably due to the 1 msg per 10' limit enforced by the lib");
      return false;
  }
  
  digitalWrite(SIGFOX_LED_PIN, HIGH);
  Serial.print("Sending data (");
  Serial.print(data.temperature);
  Serial.print(" — ");
  Serial.print(data.humidity);
  Serial.print(" — ");
  Serial.print(data.error);
  Serial.println(") over Sigfox");
  if (!Akeru.send(&data, sizeof(data))){
    Serial.println("An error occured while sending message");
    digitalWrite(SIGFOX_LED_PIN, LOW);
    return false;
  }
  digitalWrite(SIGFOX_LED_PIN, LOW);
  return true;

}
String getErrorMessage(int result){
  switch (result)
  {
  case IDDHTLIB_OK: 
    return "OK";
    break;
  case IDDHTLIB_ERROR_CHECKSUM: 
    return "Error\n\r\tChecksum error"; 
    break;
  case IDDHTLIB_ERROR_ISR_TIMEOUT: 
    return "Error\n\r\tISR time out error"; 
    break;
  case IDDHTLIB_ERROR_RESPONSE_TIMEOUT: 
    return "Error\n\r\tResponse time out error"; 
    break;
  case IDDHTLIB_ERROR_DATA_TIMEOUT: 
    return "Error\n\r\tData time out error"; 
    break;
  case IDDHTLIB_ERROR_ACQUIRING: 
    return "Error\n\r\tAcquiring"; 
    break;
  case IDDHTLIB_ERROR_DELTA: 
    return "Error\n\r\tDelta time to small"; 
    break;
  case IDDHTLIB_ERROR_NOTSTARTED: 
    return "Error\n\r\tNot started"; 
    break;
  default: 
    return "Unknown error"; 
    break;
  }
}