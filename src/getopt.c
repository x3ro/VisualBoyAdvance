/*
 * VisualBoyAdvanced - Nintendo Gameboy/GameboyAdvance (TM) emulator
 * Copyrigh(c) 1999-2002 Forgotten (vb@emuhq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * getopt.c
 *
 * Angus Mackay Thu Feb 17 10:46:37 PST 2000
 *
 * A small replacement getopt that compiles easily on all platforms.
 * This was written specificly for win32.
 *
 */

#include <stdio.h>
#include <string.h>

#ifndef USE_SYSGETOPT

int getopt(int argc, char *const *argv, const char *opts);

char *optarg;
int optind, optopt;
int opterr = 1;

static void getopt_init()
{
  optind = 1;
}

int getopt(int argc, char *const *argv, const char *opts)
{
  static int init_done = 0;
  static int suboptionpos = 1;

  if(!init_done) { getopt_init(); init_done = 1; }

  optarg = NULL;

  if(optind == argc)
  {
    /* we are done */
    return(-1);
  }

  if(argv[optind][0] == '-')
  {
    char *argp;

    /* test for end of arg marker */
    if(argv[optind][1] == '-' && argv[optind][2] == '\0')
    {
      suboptionpos = 1;
      optind++;
      return(-1);
    }

    for(argp=&(argv[optind][suboptionpos]); *argp != '\0'; argp++)
    {
      char *optp;
      int numcolon = 0;
      char *p;

      if((optp=strchr(opts, *argp)) == NULL)
      {
        if(opterr != 0)
        {
          fprintf(stderr, "%s: illegal option -- %c\n", argv[0], *argp);
        }
        optopt = *argp;
        suboptionpos++;
        if(argv[optind][suboptionpos] == '\0')
        {
          suboptionpos = 1;
          optind++;
        }
        return('?');
      }

      /* zero, one or two colons? */
      for(p=(optp+1); *p == ':'; p++) { numcolon++; }
      switch(numcolon)
      {
        /* no argument */
        case 0:
          suboptionpos++;
          if(argv[optind][suboptionpos] == '\0')
          {
            suboptionpos = 1;
            optind++;
          }
          return(*optp);
          break;

        /* manditory argument */
        case 1:
          /* the argument is seperated by a space */
          if(argp[1] == '\0')
          {
            /* ther are more args */
            if(optind+1 == argc)
            {
              suboptionpos++;
              if(argv[optind][suboptionpos] == '\0')
              {
                suboptionpos = 1;
                optind++;
              }
              if(opterr != 0)
              {
                fprintf(stderr, "%s: option requires an argument -- %c\n", 
                        argv[0], *argp);
              }
              optopt = *argp;
              return('?');
            }

            optind++;
            suboptionpos = 1;
            optarg = argv[optind];
            optind++;
            return(*optp);
          }

          /* the argument is attached */
          optarg = argp+1;
          suboptionpos = 1;
          optind++;
          return(*optp);
          break;

        /* optional argument */
        case 2:
          /* the argument is seperated by a space */
          if(argp[1] == '\0')
          {
            optind++;
            suboptionpos = 1;
            optarg = NULL;
            return(*optp);
          }

          /* the argument is attached */
          suboptionpos = 1;
          optarg = argp+1;
          optind++;
          return(*optp);
          break;

        /* a case of too many colons */
        default:
          suboptionpos++;
          if(argv[optind][suboptionpos] == '\0')
          {
            suboptionpos = 1;
            optind++;
          }
          optopt = '?';
          return('?');
          break;
      }
    }
    suboptionpos = 1;
  }
  else
  {
    /* we are done */
    return(-1);
  }

  /* we shouldn't get here */
  return(-1);
}
#endif

#ifdef TEST_DRIVER
int main(int argc, char **argv)
{
  int opt;
  char *optstring = "Aab:c::defg:::h";
  int i;

  printf("testing with option string of \"%s\"\n", optstring);

  while((opt=getopt(argc, argv, optstring)) != -1)
  {
    switch(opt)
    {
      case 'A':
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
        printf("option: %c, optarg: %s, optind: %d\n", 
               opt, optarg == NULL ? "(null)" : optarg, optind);
        break;

      default:
        fprintf(stderr, "invalid arguments sucker!\n");
        printf("optopt: %c, optind: %d\n", optopt, optind);
        exit(1);
        break;
    }
  }

  for(i=optind; i<argc; i++)
  {
    printf("arg %d: %s\n", i, argv[i]);
  }

  return 0;
}
#endif
