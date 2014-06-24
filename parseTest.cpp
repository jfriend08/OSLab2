     #include <ctype.h>
     #include <stdio.h>
     #include <stdlib.h>
     #include <unistd.h>
     
     int
     main (int argc, char **argv)
     {
       int vflag = 0;
       int sflag = 0;
       char *svalue = NULL;
       int index;
       int c;
     
       opterr = 0;
     
       while ((c = getopt (argc, argv, "vs:")) != -1)
         switch (c)
           {
           case 'v':
             vflag = 1;
             break;
           
           case 's':
             sflag = 1;
             svalue = optarg;
             break;
           case '?':
             if (optopt == 'c')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
             return 1;
           
           default:
             abort ();
           }
     
       printf ("vflag = %d, sflag = %d, svalue = %s\n",
               vflag, sflag, svalue);
     
       for (index = optind; index < argc; index++)
         printf ("Non-option argument %s\n", argv[index]);
       return 0;
     }