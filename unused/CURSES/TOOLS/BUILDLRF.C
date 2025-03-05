#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[])
{
 FILE *fp;

 if (argc < 4)
   {
    fprintf(stderr,"Invalid number of parameters\n");
    return(1);
   }
 if ((fp = fopen(argv[3],"a")) == NULL)
   {
    fprintf(stderr,"Error opening %s\n",argv[3]);
    return(1);
   }
 if (strcmp("DLL",argv[1]) == 0)
    fprintf(fp,"%s ",argv[2]);
 else
    fprintf(fp,"%s &\n",argv[2]);
 fclose(fp);
 return(0);
}
