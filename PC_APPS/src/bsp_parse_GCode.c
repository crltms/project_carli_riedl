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
	int num_of_cmd=0;


	pF = fopen (filename, "r");
	if (pF == NULL) {
		printf ("%s can't be opened\n", filename);
		ret = fclose (pF);
		if (ret != 0) {
			printf ("%s wasn't closed correctly\n", filename);
		}
		return -1;
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
				return -1;
			}
			if ( ( (gcode[i].cmd[0] == 'G') && (gcode[i].cmd[1] == '0')) || (i > LINENUMBER) || (feof (pF))) {
				if((gcode[i].cmd[0] == 'G') && (gcode[i].cmd[1] == '0')){
					fseek(pF,2,SEEK_CUR);
	        ret=fscanf(pF,"%lf Y%lf",&gcode[i].x_val,&gcode[i].y_val);
					if (ret == 0) {
						printf ("Error when reading file\n");
						ret = fclose (pF);
						if (ret != 0) {
							printf ("%s wasn't closed correctly\n", filename);
						}
						return -1;
					}
					gcode[i].ID=i;
					num_of_cmd=i;
					// printf("%i %s %f %f\n",gcode[i].ID,gcode[i].cmd,gcode[i].x_val,gcode[i].y_val);
				}
        break;
			}
		}
		i++;
	}

	ret = fclose (pF);
	if (ret != 0) {
		printf ("%s wasn't closed correctly\n", filename);
		return -1;
	}

	printf ("everything went right %i\n",num_of_cmd);

	return num_of_cmd;
}
struct gcode *get_gcode_ptr(void){
	return gcode;
}

/*! EOF */
