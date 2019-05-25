/*
 * @file bsp_parse_GCode.c
 *
 * @date: 05-2019
 * @author: Verena Riedl
 */
#include <bsp_parse_GCode.h>

struct gcode gcode[LINENUMBER];

int parse_GCode (char *filename)
{
	FILE *pF;
	int ret = 0;
	int i = 0;
	int num_of_cmd = 0;
	int inc = 0;
	double inch_cm = 1;


	pF = fopen (filename, "r");
	if (pF == NULL) {
		printf ("%s can't be opened\n", filename);
		return 0;
	}

	while ( (!feof (pF)) && (i < LINENUMBER)) {
		while (1) {
			fgets (gcode[i].cmd, STRINGLENGTH, pF);
			if (gcode[i].cmd == NULL) {
				printf ("Error when reading file\n");
				ret = fclose (pF);
				if (ret != 0) {
					printf ("%s wasn't closed correctly\n", filename);
				}
				return 0;
			}
			if ( (gcode[i].cmd[0] == 'G') && (gcode[i].cmd[1] == '9') && (gcode[i].cmd[2] == '1')) {
				inc = 1;
			}
			if ( (gcode[i].cmd[0] == 'G') && (gcode[i].cmd[1] == '2') && (gcode[i].cmd[2] == '0')) {
				inch_cm = 2.54;
			}
			if ( ( (gcode[i].cmd[0] == 'G') && (gcode[i].cmd[1] == '0')) || (i > LINENUMBER) || (feof (pF))) {
				if ( (gcode[i].cmd[0] == 'G') && (gcode[i].cmd[1] == '0')) {
					fseek (pF, 1, SEEK_CUR);
					if (fgetc (pF) == 'X') {
						ret = fscanf (pF, "%lf Y%lf", &gcode[i].x_val, &gcode[i].y_val);
						gcode[i].x_val = gcode[i].x_val * inch_cm;
						gcode[i].y_val = gcode[i].y_val * inch_cm;
						if ( (inc == 1) && (i != 0)) {
							// printf("%f %f %f %f\n",gcode[i].x_val,gcode[0].x_val,gcode[i].y_val,gcode[0].y_val);
							gcode[i].x_val = gcode[i].x_val + gcode[i - 1].x_val;
							gcode[i].y_val = gcode[i].y_val + gcode[i - 1].y_val;
						}
						if (ret == 0) {
							printf ("Error when reading file\n");
							ret = fclose (pF);
							if (ret != 0) {
								printf ("%s wasn't closed correctly\n", filename);
							}
							return 0;
						}
						gcode[i].ID = i;
						num_of_cmd = i;
						// printf ("%i %i %s %f %f\n", inc, gcode[i].ID, gcode[i].cmd, gcode[i].x_val, gcode[i].y_val);
					}
					else{
						i=i-1;
					}
				}
				break;
			}
		}
		i++;
	}

	ret = fclose (pF);
	if (ret != 0) {
		printf ("%s wasn't closed correctly\n", filename);
		return 0;
	}

	printf ("everything went right %i\n", num_of_cmd);

	return num_of_cmd;
}
struct gcode *get_gcode_ptr (void)
{
	return gcode;
}

/*! EOF */
