/*
 * @file bsp_parse_GCode.c
 *
 * @date: 05-2019
 * @author: Verena Riedl
 */
#define STRINGLENGTH 50
#define LINENUMBER 100
#include <bsp_parse_GCode.h>

_Bool parse_GCode (char *filename)
{
    FILE *pF;
    int ret = 0;
    char command[LINENUMBER][STRINGLENGTH];
    int i =0;

    pF=fopen(filename,"r");
    if (pF == NULL){
      printf("%s can't be opened\n",filename);
      ret=fclose (pF);
      if (ret != 0){
        printf("%s wasn't closed correctly\n",filename);
      }
      return false;
    }

    while (!feof(pF)){
      fgets(command[i],STRINGLENGTH,pF);
      if (command[i] == NULL){
        printf("Error when reading file\n");
        ret=fclose (pF);
        if (ret != 0){
          printf("%s wasn't closed correctly\n",filename);
        }
        return false;
      }
      printf("%i command: %s",i, command[i]);
      i++;
      if(i>LINENUMBER){
        break;
      }
    }

    ret= fclose(pF);
    if (ret != 0){
      printf("%s wasn't closed correctly\n",filename);
      return false;
    }

    printf("everything went right\n");

    return true;
}


/*! EOF */
