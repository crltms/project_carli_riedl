/*
 * @file BSP_parse_GCode.h
 *
 * @date 05-2019
 * @author: Verena Riedl
 */

#ifndef SRC_BSP_PARSE_GCODE_H_
#define SRC_BSP_PARSE_GCODE_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define STRINGLENGTH 255
#define LINENUMBER 10000
#define COMMANDLENGTH 4

struct gcode {
  int ID;
	char cmd [COMMANDLENGTH];
	double x_val;
	double y_val;
};

int parse_GCode (char *filename);
struct gcode *get_gcode_ptr(void);

#endif

/*! EOF */
