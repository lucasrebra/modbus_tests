/**
* \file SerialCommand.cpp
* \brief Generic handler for managing serial I/O communications
*
* Copyright (C) 2020 INGEN10 Ingenieria S.L.
* http://www.ingen10.com
*
* LEGAL NOTICE:
* All information contained herein is, and remains property of INGEN10 INGENIERIA SL.
* Dissemination of this information or reproduction of this material is strictly
* forbidden unless prior written permission is obtained from its owner.
* ANY REPRODUCTION, MODIFICATION, DISTRIBUTION, PUBLIC PERFORMANCE, OR PUBLIC DISPLAY
* OF, OR THROUGH USE OF THIS SOURCE CODE IS STRICTLY PROHIBITED, AND IT IS A VIOLATION
* OF INTERNATIONAL TRADE TREATIES AND LAWS.
* THE RECEIPT OR POSSESSION OF THIS DOCUMENT DOES NOT CONVEY OR IMPLY ANY RIGHTS.
*
* \author   JRB
* \date     10/05/2020
*
*/

#include "SerialCommand.h"



/**
 * Constructor makes sure some things are set.
 */
SerialCommand::SerialCommand()
{
    name = "generic hnd.";
    nCommands = 0;
    bufPos = 0;
    defaultHandler = NULL;
    hdr_size = DEFAULT_HDR_SIZE;
    end_char = DEFAULT_END_CHAR;
    command_size = DFAULT_COMMAND_SIZE;
}

/**
 * Adds a "command" and a handler function to the list of available commands.
 * This is used for matching a found token in the buffer, and gives the pointer
 * to the handler function to deal with it.
 */
int SerialCommand::addCommand(const char *header, cbFunction function)
{
    if (nCommands >= MAX_COMMANDS) {
        return 1;
    }
    strncpy(commandList[nCommands].header, header, 2);
    commandList[nCommands].function = function;
    nCommands++;
    return 0;
}

/**
 * This sets up a handler to be called in the event that the receveived command string
 * isn't in the list of commands.
 */
void SerialCommand::setDefaultHandler(cbFunction function) {
  defaultHandler = function;
}

/**
 * This checks the Serial stream for characters, and assembles them into a buffer.
 * When the terminator character (default '\n') is seen, it starts parsing the
 * buffer for a prefix command, and calls handlers setup by addCommand() member
 */
int SerialCommand::processCommand(char* command, char* response)
{
    int i;
    bool cmd_found = 0;
    int param_len = 0;
    int resp_len = 0;
    char *newcommand, *param;  

    newcommand = command + hdr_size;
    param = command + hdr_size + command_size;

    for(i=0, param_len=0; i<MAX_CMD_SIZE; i++, param_len++){
        if(*(param+i)==end_char)
            break;
    }


    for (i = 0; i < nCommands; i++) {
        if (strncmp(commandList[i].header, newcommand, command_size) == 0) {
            cmd_found = 1;
            resp_len = commandList[i].function(param, param_len, response);
            break;
        }
    }


    if (!cmd_found && (defaultHandler != NULL)) {
      (*defaultHandler)(param, 0, response);
    }

    return resp_len;
}