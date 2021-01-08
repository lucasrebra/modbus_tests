
/*****************************************************************************/
/*  Programa utilizando libreria modbus a modo de master para ESP32.         */
/*  El programa se encargará de realizar tres acciones:                      */
/*  -->Iniciar las diez primeras direcciones con valor fijo.                 */
/*                                                                           */
/*  -->Enviar el numero de segundos del programa desde el ultimo             */
/*     Reset del controlador a la direccion 40 de discret inputs.            */
/*                                                                           */
/*  -->Fijaremos diez direcciones que se puedan leer y escribir (20 a 30)    */
/*                                                                           */
/*  Autor:Lucas Rey Braga                                                    */
/*  Correo:lucas@ingen10.com                                                 */
/*                                                                           */
/*  Librería utilizada disponible en:                                        */
/*           https://github.com/emelianov/modbus-esp8266                     */
/*                                                                           */
/*****************************************************************************/



#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>
#include <ModbusRTU.h>
#include <BluetoothSerial.h>
#include <SerialCommand.h>
/*
#define REGN 10
#define SLAVE_ID 1
#define PULL_ID 1
#define FROM_REG 20
#define SEGUNDERO 30*/

int REGN=0;
int SLAVE_ID=1;
int PULL_ID=1;
int SEGUNDERO=20;

long ts;
int segs;

#define START_CHAR '$'
#define END_CHAR ';'
#define BUFF_CMD_SIZE 20

SerialCommand sCmd;//objeto para comandos serial
BluetoothSerial SerialBT;

ModbusRTU mb1;
ModbusIP mb2;

//Print dispo version
int cmdVersion(char* param, uint8_t len, char* response){
    //char fw_ver[20];
    sprintf(response, "$ACK_VERSION;");
    //sprintf(fw_ver, ", FW=%02d.02d.%02d;",FW_VER_MAJOR,FW_VER_MINOR, FW_VER_PATCH);
    //strcat(response, fw_ver);
    return strlen(response);
}


//Printed when a command is not correct
int cmdNack(char *param, uint8_t len, char* response){
    sprintf(response, "$NACK;");
    return strlen(response);
}

int cmdHreg(char*param, uint8_t len, char* response){
  sprintf(response, "Implementar funcion ");
  return strlen(response);
}

int cmdIP(char*param, uint8_t len, char* response){
  SerialBT.println(WiFi.localIP());
  sprintf(response, "Lectura IP");
  return strlen(response);
}
//Takes the serial string and gets the response for the command

int cmdCell(char *param, uint8_t len, char* response){
    char*p;
    uint16_t cell_on = atoi(param);
    p=strchr(param,'=');
    p++;
    uint16_t cell_off = atoi(p);
    Serial.print("Cell: ");
    Serial.println(cell_on);
    
    sprintf(response, "$ACK_c%d;", cell_off);
    return strlen(response);
}

//Command function by serial that takes a string with structure
//$GETXX; and print by serial the value of the XX register

int cmdGetHreg(char *param, uint8_t len, char* response){
    char*p=param; //pointer
    p+=2;
    uint16_t nHreg = atoi(p);

    Serial.print("El número leido del Holding ");
    Serial.print(nHreg);
    Serial.print(" es ");
    Serial.print(mb1.Hreg(nHreg));

    if(SerialBT.available()){
      SerialBT.print("El número leido del Holding ");
      SerialBT.print(nHreg);
      SerialBT.print(" es ");
      SerialBT.print(mb1.Hreg(nHreg));
    }
    
    return strlen(response);
}

//Command function that takes by serial a string with structure
// SETXX=YY; and introduce on the holding register XX the value YY

int cmdSetHreg(char *param, uint8_t len, char* response){
    char*p=param;

    p=p+2;
    uint16_t nHreg = atoi(p);
    int pValue=mb1.Hreg(nHreg);
    p=strchr(param,'=');
    p++;
    uint16_t value = atoi(p);

    mb1.Hreg(nHreg,value);

    Serial.print("Se ha cambiado el valor del registro ");
    Serial.print(nHreg);
    Serial.print(" que tenia el valor ");
    Serial.print(pValue);
    Serial.print(" por el valor ");
    Serial.print(value);

    return strlen(response);

}
void SerialIO(){

    char c;
    char response[BUFF_CMD_SIZE];
    static char cmd_buffer[BUFF_CMD_SIZE];
    static unsigned int cmd_p = 0;

    while (Serial.available()) {
        c = char(Serial.read());
        if(c == START_CHAR)
          cmd_p = 0;
          cmd_buffer[cmd_p] = c;
          cmd_p += 1;
          cmd_p %= BUFF_CMD_SIZE;//This is for discharging the buffer

      
        if(c == END_CHAR) {
            cmd_buffer[cmd_p] = 0;
            cmd_p = 0;
            sCmd.processCommand(cmd_buffer, response);//converts the string in the command
            //eliminating the end and the start_char
            Serial.println(response);
        }
    }  
}

//Funcion para conexion comandos serial por bluetooth
void SerialBluetooth(){

    char c;
    char response[BUFF_CMD_SIZE];
    static char bt_buffer[BUFF_CMD_SIZE];
    static unsigned int bt_p = 0;

    while (SerialBT.available()) {
        c = char(SerialBT.read());
        if(c == START_CHAR)
          bt_p = 0;
          bt_buffer[bt_p] = c;
          bt_p += 1;
          bt_p %= BUFF_CMD_SIZE;//This is for discharging the buffer

      
        if(c == END_CHAR) {
            bt_buffer[bt_p] = 0;
            bt_p = 0;
            sCmd.processCommand(bt_buffer, response);//converts the string in the command
            //eliminating the end and the start_char
            SerialBT.println(response);
        }
    }  
}




 // Callback function to read corresponding DI
uint16_t cbRead(TRegister* reg, uint16_t val) {
  SerialBT.print("Registro: ");
  SerialBT.print(reg->address.address);
  SerialBT.print(" Viejo: ");
  SerialBT.print(reg->value);
  SerialBT.print(" Nuevo: ");
  SerialBT.println(val);
  return val;
}

//  Callback cambio de valor
uint16_t cbWrite(TRegister* reg, uint16_t val) {
  SerialBT.print("Write. Reg RAW#: ");
  SerialBT.print(reg->address.address);
  SerialBT.print(" Old: ");
  SerialBT.print(reg->value);
  SerialBT.print(" New: ");
  SerialBT.println(val);
  return val;
}
  

void setup() {
  Serial.begin(115200);

  /*Configuramos conexion wifi*/

  WiFi.begin("twave-24", "KD6rUYrv");
  //WiFi.begin("MOVISTAR_8380","paderni9");
  SerialBT.println( WiFi.localIP() );

  /*Configuramos el puerto serial Bluetoth*/

  SerialBT.begin("ESP32test"); //Bluetooth device name
  SerialBT.print("The device started, now you can pair it with bluetooth!\n");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    SerialBT.print(".");
  }

  SerialBT.print("\n");
  SerialBT.print("WiFi connected\n");  
  SerialBT.print("IP address:\n");
  SerialBT.print(WiFi.localIP());//Imprimimos IP de la conexion

  /*Congiguramos la comunicación modbus RTU*/
  Serial.begin(9600, SERIAL_8N1);
  mb1.begin(&Serial);

  
  /*Objeto modbus2--> modbusIP*/

  mb2.server();
  mb2.addHreg(REGN,100,20);//empieza en 0, valor predefinido 100 y 20 registros
  mb2.addHreg(SEGUNDERO);//añadimos holding segundero


  /*Setup mb1--> ModbusRTU*/

  mb1.slave(SLAVE_ID);
  //Registros de valores fijos
  //mb1.addHreg(REGN,100,10);
  //Registro segundero
  //mb1.addIreg(SEGUNDERO);
  //mb.Hreg(REGN, 100);
  //mb1.onGetHreg(REGN,cbRead,10);
  //mb1.onSetHreg(REGN,cbWrite,10);

  //Comandos disponibles
    sCmd.addCommand("V", cmdVersion);
    sCmd.addCommand("GET",cmdGetHreg);
    sCmd.addCommand("SET",cmdSetHreg);
    sCmd.addCommand("IP",cmdIP);
    sCmd.setDefaultHandler(cmdNack);
  
}

void loop() {
  
  if ( millis() > ts + 2000 ) 
  {
    segs++;
    ts = millis();
    mb1.Hreg(SEGUNDERO,segs);//Escribimos en la direccion RTU los segundos
    //mb2.Hreg(SEGUNDERO,segs);//Escribimos en la direccion TCP los segundos
    SerialBT.print(mb1.Hreg(SEGUNDERO));
  }

  mb1.task();
  mb2.task();

  SerialIO();//Funcion comandos serial 
  SerialBluetooth();//Funcion comandos serial bluetooth

  delay(50);
}