#ifndef CALC_H
#define CALC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <String.h> //thank god
#include "strutil.h"

//#define DEBUG 666

//Error codes
#define CALC_OK 0
#define CALC_INVALID_OPERATOR 1
#define CALC_UNMATCHED_PARENS 2
#define CALC_SOMETHING_HORRIBLY_WRONG 3
#define CALC_NO_LAST_ANSWER 4
#define CALC_INVALID_EXPRESSION 5
#define CALC_UNKNOWN_RADIX 6
#define CALC_NO_EXPRESSION 7

#define CALC_TRIG_EXP 6
#define CALC_NORMAL_EXP 7
#define CALC_SOLVED_EXP 8

class Calculator{
	private:
		BString *_operators;
		BString *_traditionalOperators;
		
		BString *_theExpression;		
		int _errorCode;
		BString *_lastAnswer;
		
		int _responseBase;
		bool _useRadians;
		
		int expressionType(BString *exp);
		bool isOperator(char c);
		bool isNegativeModifier(BString *exp, int p);
		
		void cleanUpSyntax();	
		void parenthetize();
		void findFirstSolvable(int &start, int &stop);
		void solve(int start, int stop);	
		
	public:
		Calculator(void);
		~Calculator(void);
		int calculate(BString *expression, BString *response, int &selStart, int &selStop);
		int calculate(BString *expression, float *answer);
		
		BString getLastAnswer();
		void setLastAnswer(BString ans);
		
		void useRadians();
		void useDegrees();
		
		void setResponseBase(int b);
		int responseBase();
		
};

#endif