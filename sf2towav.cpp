/*
sf2towav - Extract WAV files from Soundfont 2 (SF2) Files
Copyright (C) 1998 - 2020 Jeremy Smith
-----
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
-----
INFORMATION
  www: https://github.com/jeremylsmith/sf2towav/
  e-mail: jeremy@decompiler.org
*/

#include "stdio.h"
#include "stdlib.h"
#include <string.h>

FILE *inp = NULL, *outp = NULL;

void
put32 (unsigned long d)
{
  unsigned char *ptr = (unsigned char *) &d;

  putc (ptr[0], outp);
  putc (ptr[1], outp);
  putc (ptr[2], outp);
  putc (ptr[3], outp);
};

unsigned long
bit32convl (FILE * inp)
{
  unsigned long d;
  unsigned char *ptr = (unsigned char *) &d;
  ptr[0] = getc (inp);
  ptr[1] = getc (inp);
  ptr[2] = getc (inp);
  ptr[3] = getc (inp);
  return (d);
};


int sf2towav (char *);

/*Used in sf2towav*/

char item[6];
char string[512];
int inputchar;

void
getstring ()
{
  int thisbyte, stringcount;
  stringcount = 0;
	while((thisbyte !=0) && (stringcount<20))
    {
      thisbyte = getc (inp);
      string[stringcount] = thisbyte;
      stringcount++;
    };
  string[stringcount] = '\0';
  while (stringcount < 20)
    {
      getc (inp);
      stringcount++;
    };
};

unsigned long
bit32conv ()
{
  unsigned long temp;
  int inputchar;

  inputchar = getc (inp);
  temp = (unsigned long) inputchar *16777216;
  inputchar = getc (inp);
  temp += (unsigned long) inputchar *65536;
  inputchar = getc (inp);
  temp += (unsigned long) inputchar *256;
  inputchar = getc (inp);
  temp += (unsigned long) inputchar;
  return (temp);
}

/*unsigned long bit32convl ()
{
long temp;

inputchar=getc(inp);
printf("crash");
exit(0);
temp=(long)inputchar;
inputchar=getc(inp);
temp+=(long)inputchar*256;
inputchar=getc(inp);
temp+=(long)inputchar*65536;
inputchar=getc(inp);
temp+=(long)inputchar*16777216;
return (temp);
}*/

void
getitem ()
{
  item[0] = getc (inp);
  item[1] = getc (inp);
  item[2] = getc (inp);
  item[3] = getc (inp);
  item[4] = '\0';
};

//returns 0 = is sf2
int
is_sf2 (char *infilename)
{
  unsigned long longnumber;
  unsigned long RIFFlen, WAVElen, SMALLWAVElen, DATACHUNKlen, PATCHCHUNKlen;
  unsigned long WAVEfilepos;
  unsigned long allwavsn, allwavsc;
  unsigned long waveoutcount, fileoutcount, waveloopcount, patchcount;
  unsigned long tempfile;
  unsigned long wavstart, wavend, wavsloop, waveloop, wavsamprate;

  char filename[512];
  int onebyte;

  inp = fopen (infilename, "rb");

  if (!inp)
    return 1;

  /*Check for RIFF */
  getitem ();


  if (strcmp (item, "RIFF") != 0)
    return 1;
  RIFFlen = bit32convl (inp);
  //printf("Windows multimedia file ID found\n");

  /*Check this is a Soundfont */
  getitem ();
  if (strcmp (item, "sfbk") != 0)
    return 1;
  printf ("Soundfont ID found\n");

  /*Check this is a LIST */

  getitem ();
  if (strcmp (item, "LIST") != 0)
    return 1;

  fclose (inp);

  return 0;
}


int
sf2towav (char *infilename, char *outpath)
{
  unsigned long longnumber;
  unsigned long RIFFlen, WAVElen, SMALLWAVElen, DATACHUNKlen, PATCHCHUNKlen;
  unsigned long WAVEfilepos;
  unsigned long allwavsn, allwavsc;
  unsigned long waveoutcount, fileoutcount, waveloopcount, patchcount;
  unsigned long tempfile;
  unsigned long wavstart, wavend, wavsloop, waveloop, wavsamprate;

  char filename[512];
  int onebyte;

  inp = fopen (infilename, "rb");

  if (!inp)
    {
      printf ("Cannot open input Soundfont file\n");
      return 1;
    }

/*Check for RIFF*/
  getitem ();


  if (strcmp (item, "RIFF") != 0)
    return 1;
  RIFFlen = bit32convl (inp);
//printf("Windows multimedia file ID found\n");

/*Check this is a Soundfont*/
  getitem ();
  if (strcmp (item, "sfbk") != 0)
    return 1;
  printf ("Soundfont ID found\n");

/*Check this is a LIST*/

  getitem ();
  if (strcmp (item, "LIST") != 0)
    return 1;

  DATACHUNKlen = bit32convl (inp);
  fseek (inp, ftell (inp) + DATACHUNKlen, SEEK_SET);
  getitem ();
  if (strcmp (item, "LIST") != 0)
    return 1;
  WAVElen = bit32convl (inp);

  WAVEfilepos = ftell (inp);
  fseek (inp, ftell (inp) + WAVElen, SEEK_SET);
  getitem ();

  PATCHCHUNKlen = bit32convl (inp);

  getitem ();
  if (strcmp (item, "pdta") != 0)
    return 1;
  patchcount = 0;
  while (patchcount < PATCHCHUNKlen)
    {
      getitem ();
      longnumber = bit32convl (inp);
      if (strcmp (item, "shdr") != 0)
	{
	  fseek (inp, ftell (inp) + longnumber, SEEK_SET);
	  patchcount += longnumber;
	  continue;
	};
      allwavsn = longnumber;
      longnumber = ftell (inp);
      allwavsc = 0;
      while (allwavsc * 2 < allwavsn)
	{
	  getstring ();
	  printf ("File: %s\n", string);
	  wavstart = bit32convl (inp);
	  wavend = bit32convl (inp);
	  wavsloop = bit32convl (inp);
	  waveloop = bit32convl (inp);
	  wavsamprate = bit32convl (inp);

	  onebyte = getc (inp);
	  onebyte = getc (inp);
	  onebyte = getc (inp);
	  onebyte = getc (inp);
	  onebyte = getc (inp);
	  onebyte = getc (inp);

/*printf("wavstart %ld\n",wavstart);
printf("wavend %ld\n",wavend);
printf("wavsloop %ld\n",wavsloop);
printf("waveloop %ld\n",waveloop);
printf("wavsamprate %ld\n",wavsamprate);*/

	  sprintf (filename, "%s\\%s.wav", outpath, string);

	  if ((outp = fopen (filename, "w+b")) == NULL)
	    {
	      printf ("Cannot create output file");
	      return 1;
	    }
	  fprintf (outp, "RIFF");

	  put32 (0x2cL + ((wavend - wavstart) * 2L));
	  fprintf (outp, "WAVE");
	  fprintf (outp, "fmt ");
	  put32 (0x10L);
	  putc (1, outp);
	  putc (0, outp);
	  putc (1, outp);
	  putc (0, outp);
	  put32 (wavsamprate);
//put32(0);
//put32((wavsamprate*16)*2);
	  putc (0x88, outp);
	  putc (0x58, outp);
	  putc (0x01, outp);
	  putc (0x00, outp);

	  putc (2, outp);
	  putc (0, outp);
	  putc (0x10, outp);
	  putc (0, outp);
	  fprintf (outp, "data");
	  put32 ((wavend - wavstart) * 2L);

	  tempfile = ftell (inp);
	  fseek (inp, WAVEfilepos + (wavstart * 2) + 12L, SEEK_SET);
	  waveoutcount = 0;
	  while (waveoutcount < wavend - wavstart)
	    {
	      onebyte = getc (inp);
	      putc (onebyte, outp);
	      onebyte = getc (inp);
	      putc (onebyte, outp);
	      waveoutcount++;
	    };
/*waveoutcount=0;
while (waveoutcount<32L)
{
putc(0,outp);
waveoutcount++;
};*/

	  fclose (outp);
	  fseek (inp, tempfile, SEEK_SET);
	  allwavsc += 26;
	};
    };
  if (inp)
    fclose (inp);

  return 0;







/*

getitem();
getitem();

waveoutcount=0;
fileoutcount=0;
while (waveoutcount<WAVElen)
{
SMALLWAVElen=bit32convl(inp);

sprintf(filename,"wib%ld.wav",fileoutcount);

if((outp=fopen(filename,"w+b"))==NULL)
{
printf("Can't open output file\n");
return;
}
waveloopcount=0;
printf("%ld\n",SMALLWAVElen);
return;
while (waveoutcount<SMALLWAVElen)
{
onebyte=getc(inp);
putc(onebyte,outp);
waveloopcount++;
};
fclose(outp);

waveoutcount+=SMALLWAVElen;
fileoutcount++;
};
printf("Done\n");

return;
*/
};


int
main (int argc, char *argv[])
{
  if (argc == 2)
    {
      sf2towav (argv[1], ".");
    }
  else
    {
      printf ("SF2toWAV Version 1.0\n");
      printf ("(c)2020 Jeremy Smith\n");
      printf ("Extracts all WAV samples from a Soundfont 2 file\n");
      printf ("To use: sf2towav Soundfont filename\n");
      printf ("Eg 'sf2towav linndrum.sf2'\n");
    }

}

