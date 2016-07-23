#ifndef STRUTIL_H
#define STRUTIL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <String.h> //thank god

bool contains(BString *str, char c);
int countOccurances(BString *str, char c);
int findNthOccurance(BString *str, char c, int n);
int findCharBeforePosition(BString *str, char c, int p); 
int findCharAfterPosition(BString *str, char c, int p);

int findElementOfSetBeforePosition(BString *str, BString *set, char *whichElement, int p);
int findElementOfSetAfterPosition(BString *str, BString *set, char *whichElement, int p);


#endif