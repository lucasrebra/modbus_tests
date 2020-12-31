
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

#include <Arduino.h>
#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#include <SerialCommand.h>

//Offsets de las direcciones registro en los diferentes tipos de protocolo
const int SEGUNDERO = 20; 
const int FIJOS = 0;
const int LEER_ESCRIBIR = 11;
const int nreg=10;

//ModbusIP object
ModbusIP mb;

//Variables utilizadas para el control temporal
long ts;
int segs = 0;


#define START_CHAR '$'
#define END_CHAR ';'
#define BUFF_CMD_SIZE 20

SerialCommand sCmd;//objeto para comandos serial

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

int cmdGetHreg(char *param, uint8_t len, char* response){
    char*p; //pointer
    uint16_t cell_on = atoi(param);
    Serial.println(mb.Hreg(cell_on));

    Serial.print("Cell: ");
    Serial.println(cell_on);
    
    sprintf(response, "$ACK_c%d;", cell_on);
    return strlen(response);
}

int cmdSetHreg(char *param, uint8_t len, char* response){
    char*p; 
    uint16_t cell_on = atoi(param);

    p=strchr(param,'=');
    p++;
    uint16_t cell_off = atoi(p);
    Serial.print("Cell: ");
    Serial.println(cell_on);
    
    sprintf(response, "$ACK_c%d;", cell_off);
    mb.Hreg(cell_on,cell_off);
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

//CALLBACKS--> Cada vez que se produzca una peticion del cliente

//CALLBACK LECTURA
uint16_t cbRead(TRegister* reg, uint16_t val) {
  Serial.print("Read. Reg RAW#: ");
  Serial.print(reg->address.address);
  Serial.print(" Old: ");
  Serial.print(reg->value);
  Serial.print(" New: ");
  Serial.println(val);
  return val;
}

//CALLBACK ESCRITURA
uint16_t cbWrite(TRegister* reg, uint16_t val) {
  Serial.print("Write. Reg RAW#: ");
  Serial.print(reg->address.address);
  Serial.print(" Old: ");
  Serial.print(reg->value);
  Serial.print(" New: ");
  Serial.println(val);
  return val;
}



/*
 * Programa de configuracion: 
 * -Configuracion de la conexion wifi utilizada
 * -Inicializacion de la conexion modbus IP
 * -Configuracion de las direccion segun el tipo de protocolo
 */

void setup() {
    Serial.begin(115200);

    //Conexion WIFI
    WiFi.begin("MOVISTAR_8380", "paderni9");
    //WiFi.begin("twave-24", "KD6rUYrv");
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println( "" );
    Serial.println( "WiFi connected" );  
    Serial.println( "IP address: " );
    Serial.println( WiFi.localIP() );

    //Empieza la conexion modbus como master
    mb.server();		

    //Configuracion de las direcciones de registro
    mb.addHreg( SEGUNDERO, 0, 1 );//discret input en direccion segundero
    if (!mb.addHreg(FIJOS, 100, nreg)) Serial.println("Error");//init registros fijos
    if (!mb.addHreg(LEER_ESCRIBIR, 1000, nreg)) Serial.println("Error");//init registros rw

    //Callback leer y escribir
    //mb.onGetHreg(LEER_ESCRIBIR, cbRead, nreg); // Add callback on Coils value get
    //mb.onSetHreg(LEER_ESCRIBIR, cbWrite, nreg);

    //Inicializamos la variable ts para que mida ms
    ts = millis();


    //Comandos disponibles
    sCmd.addCommand("V", cmdVersion);
    sCmd.addCommand("G",cmdGetHreg);
    sCmd.addCommand("S",cmdSetHreg);
    sCmd.setDefaultHandler(cmdNack);

}


void loop() {
   //Call once inside loop() - se lanzan las conexiones configuradas
   mb.task();

   //Al estar nosotros mandando señales cada cierto tiempo lo tratare como si
   //fuera una entrada analógica, debido a que es así como funcionar en una 
   //aplicacion real.

    //Cada segundo se actualiza la direccion
  if ( millis() > ts + 2000 ) 
  {
    segs++;
    ts = millis();
    mb.Hreg(SEGUNDERO,segs);//Escribimos en la direccion los segundos
  }
    
    SerialIO();
    delay(10);
}
