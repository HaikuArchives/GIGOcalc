#include "strutil.h"



bool contains(BString *str, char c){
	for (int i = 0; i < str->Length(); i++)
		if (str->ByteAt(i) == c) return true;
		
	return false;
}

//understands n=0 to be first occurance
int findNthOccurance(BString *str, char c, int n){
	int count = 0;
	for (int i = 0; i < str->Length(); i++){
		if (str->ByteAt(i) == c) count++;
		if (count == n + 1) return i;
	}
	
	return -1;
}

int countOccurances(BString *str, char c){
	int count = 0;
	
	for (int i =0; i < str->Length(); i++)
		if (str->ByteAt(i) == c) count++;
		
	return count;
}

int findCharBeforePosition(BString *str, char c, int p){
	for (int i = p; i >= 0; i --)
		if (str->ByteAt(i) == c) return i;
	
	return -1;
}

int findCharAfterPosition(BString *str, char c, int p){
	for (int i = p; i < str->Length(); i++)
		if (str->ByteAt(i) == c) return i;
		
	return -1;
}

int findElementOfSetBeforePosition(BString *str, BString *set, char *whichElement, int p){
	for (int i = p; i >= 0; i--){
		if (contains(set, str->ByteAt(i))){
			*whichElement = str->ByteAt(i);
			return i;
		}
	}
			
	return -1;
}

int findElementOfSetAfterPosition(BString *str, BString *set, char *whichElement, int p){
	for (int i = p; i < str->Length(); i++){
		if (contains(set, str->ByteAt(i))){
			*whichElement = str->ByteAt(i);
			return i;
		}
	}
			
	return -1;
}