#ifndef _SERIALCOMMAND_H_
#define _SERIALCOMMAND_H_

#include <Arduino.h>
#include <inttypes.h>
#include <string.h>

#define MAX_CMD_SIZE 32
#define MAX_COMMANDS 64

#define DEFAULT_END_CHAR ';'
#define DEFAULT_HDR_SIZE 1
#define DFAULT_COMMAND_SIZE 1

typedef int (* cbFunction)(char* param, uint8_t len, char* response);


typedef struct commandCallback {
    char header[2];
    cbFunction function;
} commandCallback;


class SerialCommand {
public:
    const char* name;
    uint8_t command_size;
    uint8_t hdr_size; 
    char end_char;

    SerialCommand();
    int processCommand(char* command, char* response);
    int addCommand(const char *header, cbFunction function);
    void setDefaultHandler(cbFunction function);
private:
    commandCallback commandList[MAX_COMMANDS];
    byte nCommands;
    cbFunction defaultHandler;
    char buffer[MAX_CMD_SIZE];
    int bufPos;
};

#endif //_SERIALCOMMAND_H_