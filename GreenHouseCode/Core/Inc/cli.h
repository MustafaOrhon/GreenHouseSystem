/*
 * cli.h
 *
 *  Created on: May 7, 2024
 *      Author: Mustafa
 */

#ifndef INC_CLI_H_
#define INC_CLI_H_

#include "stdint.h"
#include "stddef.h"
/* Type definition for command handlers */
typedef void (*CLI_CommandHandler)(void);

/* Command structure */
typedef struct {
    const char *command;
    size_t command_size;
    CLI_CommandHandler handler;
} sCLICommand_t;

/* Function declarations */
void CLI_Init(void);
void CLI_PostCommand(const char *data);
void CLI_SendData(const char *data, uint16_t size);

#endif /* INC_CLI_H_ */
