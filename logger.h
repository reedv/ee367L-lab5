/*
 * logger.h
 *
 *  Created on: Mar 4, 2016
 *      Author: reedvillanueva
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#define LOG_PRINT(...) log_print(__FILE__, __LINE__, __VA_ARGS__ )

char* print_time();
void log_print(char* filename, int line, char *fmt,...);

#endif /* LOGGER_H_ */
