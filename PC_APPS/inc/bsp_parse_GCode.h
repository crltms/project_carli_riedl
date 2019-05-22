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

#define STRINGLENGTH 4
#define LINENUMBER 1000

struct gcode {
  int ID;
	char cmd [STRINGLENGTH];
	double x_val;
	double y_val;
};

int parse_GCode (char *filename);
struct gcode *get_gcode_ptr(void);

#endif

/*! EOF */
