#include "calculator.h"

Calculator::Calculator(){
	_theExpression = NULL;
	_lastAnswer = NULL;
	_errorCode = 0;
	
	_traditionalOperators = new BString("*/^%><&|+-");

	_operators = new BString(*_traditionalOperators);
	_operators->Prepend("sctSCT");
	
	useDegrees();
	setResponseBase(10);
}

Calculator::~Calculator(){
	delete _operators;
	delete _traditionalOperators;

	if (_lastAnswer) delete _lastAnswer;
}

void Calculator::setLastAnswer(BString ans){
	_lastAnswer = new BString(ans);
}

BString Calculator::getLastAnswer(){
	BString la;
	
	if (_lastAnswer)
		la.SetTo(*_lastAnswer);
	
	return la;
}

void Calculator::useRadians(){
	_useRadians = true;
}
void Calculator::useDegrees(){
	_useRadians = false;
}

int Calculator::responseBase(){
	return _responseBase;
}

void Calculator::setResponseBase(int base){
	_responseBase = base;
}



//*******************************************************************
//Internal utilities, helpers
//*******************************************************************

void Calculator::findFirstSolvable(int &start, int &stop){
	if ((_theExpression == NULL) || (!_theExpression->CountChars())){
		#ifdef DEBUG
		printf("\nCalculator::findFirstSolvable() : _theExpression is empty.\n");
		#endif
		return;
	}
	
	int index, length = _theExpression->Length();
	
	for (index = 0; index < length; index++){
		if (_theExpression->ByteAt(index) == '(') start = index;
		if (_theExpression->ByteAt(index) == ')'){
			stop = index;
			break;
		}
	}
	
	stop++;
	#ifdef DEBUG
	printf("\nCalculator::findFirstSolvable() : first solvable at (%d to %d) in %s\n\n", start, stop, _theExpression->String());
	#endif
}

bool Calculator::isOperator(char o){
	BString *ops = _operators;
	
	for (int i = 0; i < ops->Length(); i++){
		if (ops->ByteAt(i) == o) return true;
	}		
	return false;
}

bool Calculator::isNegativeModifier(BString *exp, int p){
	//hateful. But there needs to be a mechanism to distinguish when a minus sign means 'subtract' and
	//when it means 'make the following number negative' -- exp is assumed to include parenthesizing
	//(30 - 50) vs (-50 * 2)
	
	//first is it actually a '-'?
	if (exp->ByteAt(p) == '-'){
	
		if (p == 0) return true; //just in case (HACK! HACK!)
		if (isOperator(exp->ByteAt(p - 1))) return true;
		if (exp->ByteAt(p - 1) == '(') return true;

	}
	
	return false;
} 

int Calculator::expressionType(BString *exp){
	//this function simply tries to determine which of three formats the expression might be
	// 1 (operand operator operand) -- CALC_NORMAL_EXP
	// 2 (trigfunction operand) -- CALC_TRIG_EXP
	// 3 (operand) -- CALC_SOLVED_EXP
	
	
		
	if (			countOccurances(exp, 's') + countOccurances(exp, 'c') + countOccurances(exp, 't') + 
					countOccurances(exp, 'S') + countOccurances(exp, 'C') + countOccurances(exp, 'T')
		 > 0 ) return CALC_TRIG_EXP;
	
	if (				countOccurances(exp, '+') + countOccurances(exp, '*') +
						countOccurances(exp, '/') + countOccurances(exp, '^') + countOccurances(exp, '%') +
						countOccurances(exp, '<') + countOccurances(exp, '>') + countOccurances(exp, '&') +
						countOccurances(exp, '|')
		> 0) return CALC_NORMAL_EXP;
	
	//the minus sign as operator vs negator is a tough nut to crack
	if (countOccurances(exp, '-') > 1) return CALC_NORMAL_EXP;
	
	int minusPos = exp->FindFirst('-');
	if (minusPos != B_ERROR)
		if (!isNegativeModifier(exp, minusPos)) return CALC_NORMAL_EXP;
	
	return CALC_SOLVED_EXP;
}



//*******************************************************************
//*******************************************************************
//*******************************************************************
//*******************************************************************

int Calculator::calculate(BString *expression, float *answer){
	BString answerStr;
	int start, stop;
	
	int success;
	
	int rb = responseBase();
	setResponseBase(10);
	success = calculate(expression, &answerStr, start, stop);
	setResponseBase(rb);

	if (success == CALC_OK){
		*answer = atof(answerStr.String());
		return CALC_OK;
	}
	else
		return success;
}
		

int Calculator::calculate(BString *expression, BString *response, int &selStart, int &selStop){
	_theExpression = expression;
	_errorCode = 0;
	
	#ifdef DEBUG
	printf("Calculator::calculate() : starting new calculation. ----------------------- \n");
	#endif
	
	cleanUpSyntax();
	
	while ((_theExpression->ByteAt(0) == '(') && (_errorCode == 0)){
		findFirstSolvable(selStart, selStop);
		solve(selStart, selStop);
	}				
	
	if (_errorCode != 0){
		
		switch(_errorCode){
			case CALC_UNMATCHED_PARENS:{
				response->SetTo("Unmatched parens.");		
				break;
			}
			
			case CALC_INVALID_OPERATOR:{
				response->SetTo("Invalid operator used.");
				break;
			}
			
			case CALC_NO_LAST_ANSWER:{
				response->SetTo("'ans' has not been stored yet.");
				break;
			}
			
			case CALC_NO_EXPRESSION:{
				response->SetTo("");
				break;
			}
			
			default: {
				*response = response->SetTo("Default error. Sorry we can't be more specific.");
				break;
			}
		}
	}
	else{

		if (_lastAnswer) delete _lastAnswer;
		_lastAnswer = new BString(*_theExpression);

		if (_responseBase != 10){
			char answer[255];
			switch(_responseBase){
				case 2: {

					int shift = sizeof(long) * 8 - 1;
					long mask = 0;

					long number = (long)atof(_theExpression->String());
					BString binaryStr;
					
					for (int r = 0; r <= shift; r++){
						mask = (1 << (shift - r));
						
						if (mask & number) binaryStr.Append('1', 1);
						else binaryStr.Append('0', 1);
					}
					
					response->SetTo(binaryStr);
					break;					
				}
				case 8: sprintf(answer, "%.11lo", (unsigned long)atof(_theExpression->String())); response->SetTo(answer); break;
				case 16: sprintf(answer, "%.8lx", (unsigned long)atof(_theExpression->String())); response->SetTo(answer); break;
				
				default: response->SetTo("Unimplemented numerical base. Try 2, 8, 10 or 16."); _errorCode = CALC_UNKNOWN_RADIX;
			}		
		}
		else{
			response->SetTo(*_theExpression);
		}
		
		
		
		#ifdef DEBUG
		printf("Calculator::calculate() : stored %s as _lastAnswer\n", _lastAnswer->String());
		#endif
	}	
	return (_errorCode != CALC_OK);
}	


//*************************************************
//*************************************************
//*************************************************
//*************************************************


void Calculator::solve(int iStart, int iEnd){
	float firstOp, secondOp;
	const char *start;
	char op;
	BString exp;
	
	_theExpression->MoveInto(exp, iStart, iEnd - iStart);
	start = exp.String();

	#ifdef DEBUG
	printf("Calculator::solve() : working expression is %s\n", start);	
	#endif
	
	int expType = expressionType(&exp);
	
	if (expType == CALC_SOLVED_EXP){
		sscanf(start, "(%f)", &secondOp);

		#ifdef DEBUG
		printf("Calculator::solve() : CALC_SOLVED_EXP 2nd: %f\n", secondOp);
		#endif
	}	

	else if (expType == CALC_NORMAL_EXP){ 
	
		sscanf(start, "(%f%c%f)",&firstOp,&op, &secondOp);
	
		#ifdef DEBUG
		printf("Calculator::solve() : CALC_NORMAL_EXP 1st: %f op: %c 2nd: %f\n", firstOp, op, secondOp);
		#endif
	
		switch (op){
			case '+': {
				secondOp = secondOp + firstOp;		
				break;
			}
			
			case '-': {
				secondOp = firstOp - secondOp;		
				break;
			}
	
			case '*': {
				secondOp = secondOp * firstOp;		
				break;
			}
	
			case '/': {
				secondOp = firstOp / secondOp;		
				break;
			}
			
			case '^': {
				secondOp = pow(firstOp, secondOp);
				break;
			}
			
			case '%': {
				while (firstOp >= secondOp)
					firstOp -= secondOp;
				secondOp = firstOp;
				break;
			}
					
			case '>': {
				secondOp = (long)firstOp >> (long)secondOp;
				break;
			}
			
			case '<': {
				secondOp = (long)firstOp << (long)secondOp;
				break;
			}
			
			case '&': {
				secondOp = (long) firstOp & (long)secondOp;
				break;
			}
			
			case '|': {
				secondOp = (long) firstOp | (long)secondOp;
				break;
			}
			
			default: {
				_errorCode = CALC_INVALID_OPERATOR;
				break;
			}
		}
	}

	else if (expType == CALC_TRIG_EXP){

		sscanf(start, "(%c%f)",&op, &secondOp);
	
		#ifdef DEBUG
		printf("Calculator::solve() : TRIG Expression op: %c radians: %f\n", op, secondOp);
		#endif

		switch (op){
			case 's': {
				secondOp = sin((double)	_useRadians ? secondOp : (secondOp * PI / 180.0));
				break;
			}
			
			case 'c': {
				secondOp = cos((double)	_useRadians ? secondOp : (secondOp * PI / 180.0));
				break;
			}
			
			case 't': {
				secondOp = tan((double)	_useRadians ? secondOp : (secondOp * PI / 180.0));
				break;
			}	
			
			case 'S': {
				secondOp = asin((double)	_useRadians ? secondOp : (secondOp * PI / 180.0));
				break;
			}
			
			case 'C': {
				secondOp = acos((double)	_useRadians ? secondOp : (secondOp * PI / 180.0));
				break;
			}
			
			case 'T': {
				secondOp = atan((double)	_useRadians ? secondOp : (secondOp * PI / 180.0));
				break;
			}	
			
			default: {
				_errorCode = CALC_INVALID_OPERATOR;
				break;
			}
		}			
	}
	else{
		_errorCode = CALC_INVALID_EXPRESSION;
	}
	
	//NOW PUT ANSWER BACK INTO EQUATION STRING
	if (_errorCode == CALC_OK){
		char number[255];
		memset(number, '\0', 255);
		sprintf(number, "%f", secondOp);
				
		BString result( number );
		*_theExpression = _theExpression->Insert(result, iStart);	

		#ifdef DEBUG
		printf("Calculator::solve() : Calculation successful, master expression is now %s\n\n", _theExpression->String());	
		#endif
	}
	else{
		*_theExpression = _theExpression->Insert(exp, iStart);
	
		#ifdef DEBUG
		printf("Calculator::solve() : Calculation failed, master expression is now %s\n\n", _theExpression->String());	
		#endif
	}
}


void Calculator::cleanUpSyntax()
{

	#ifdef DEBUG
	printf("Calculator::cleanUpSyntax() : BEGIN : master expression is %s\n", _theExpression->String());	
	#endif

	_theExpression->ToLower();	
	_theExpression->RemoveSet(" "); //trim out spaces
	
	if (_theExpression->Length() == 0){
		_errorCode = CALC_NO_EXPRESSION;

		#ifdef DEBUG
		printf("Calculator::cleanUpSyntax() : No expression, leaving...\n");	
		#endif

		return;
	}
	
	
	//for the time being, wrap all master expressions in parens.
	//*_theExpression = _theExpression->Insert('(', 1, 0);
	//*_theExpression = _theExpression->Insert(')', 1, _theExpression->Length());


	//replace instances of 'ans' with the string value of the last answer
	if (_theExpression->IFindFirst("ans") != B_ERROR)
		if (_lastAnswer != NULL)
			*_theExpression = _theExpression->IReplaceAll("ans", _lastAnswer->String());
		else
			_errorCode = CALC_NO_LAST_ANSWER;	


	//replace operator strings with single-byte codes or expressions
	*_theExpression = _theExpression->IReplaceAll("sin", "s");
	*_theExpression = _theExpression->IReplaceAll("cos", "c");	
	*_theExpression = _theExpression->IReplaceAll("tan", "t");
	*_theExpression = _theExpression->IReplaceAll("asin", "S");
	*_theExpression = _theExpression->IReplaceAll("acos", "C");
	*_theExpression = _theExpression->IReplaceAll("atan", "T");
	*_theExpression = _theExpression->IReplaceAll(">>", ">");
	*_theExpression = _theExpression->IReplaceAll("<<", "<");
	*_theExpression = _theExpression->IReplaceAll("and", "&");
	*_theExpression = _theExpression->IReplaceAll("or", "|");	
	*_theExpression = _theExpression->IReplaceAll("pi", "3.14159265");	
	
	
			
	parenthetize();

	//count if there are an equal number of left and right parens	
	int count = 0;
	for (int index = 0; index < _theExpression->Length(); index ++)
	{
		if (_theExpression->ByteAt(index) == '(')
			count++;
		else if (_theExpression->ByteAt(index) == ')')
			count--;
	}

	if (count != 0){
		_errorCode = CALC_UNMATCHED_PARENS; 
		return;
	}
	


	#ifdef DEBUG	
	printf("Calculator::cleanUpSyntax() : END : cleaned master expression is %s\n\n", _theExpression->String());	
	#endif
}

void Calculator::parenthetize(){
//General idea: see 'equations.txt'

	char op;
	
	//cycle through operators
	for (int opIndex = 0; opIndex < _operators->Length(); opIndex++){
		op = _operators->ByteAt(opIndex);	
		
		int opCount = countOccurances(_theExpression, op);
		
		//find each occurance of said operator
		for (int i = 0; i < opCount; i++){
		
			int opPos = findNthOccurance(_theExpression, op, i);
						
			if (opPos == -1){
				#ifdef DEBUG
				printf("Calculator::parenthetize : couldn't find instance %d of operator %c\n", i, op);
				#endif
				break;
			}
			
			if (!isNegativeModifier(_theExpression, opPos)){
	
				
	
				#ifdef DEBUG
				printf("Calculator::parenthetize : about to mangle operator %c(occurance %d) expression is %s\n", op, i, _theExpression->String());
				#endif
	
	
	
				//********************************************
				//what we've got to do is find the first operators to left and right of this operator
				//which are not inside of a nested () group;
	
				int leftParenPoint = 0, rightParenPoint = _theExpression->Length();								
				int parenSum, j;
				
				parenSum = 0;
				for (j = opPos - 1; j >= 0; j--){
					if (_theExpression->ByteAt(j) == '(') parenSum--;
					if (_theExpression->ByteAt(j) == ')') parenSum++;
					
					if (isOperator(_theExpression->ByteAt(j)) && (!isNegativeModifier(_theExpression, j)) && (parenSum <= 0)){
						leftParenPoint = j + 1; //we want it to be to the right of the operator
						break;
					}			
				}
				
				parenSum = 0;
				for (j = opPos + 1; j < _theExpression->Length(); j++){
					if (_theExpression->ByteAt(j) == '(') parenSum++;
					if (_theExpression->ByteAt(j) == ')') parenSum--;
					
					if (isOperator(_theExpression->ByteAt(j)) && (!isNegativeModifier(_theExpression, j)) && (parenSum <= 0)){
						rightParenPoint = j; //insert function of string pushes remainder right
						break;				//so this will insert to left of operator
					}			
				}
				
	
				//do right side first so as to not corrupt indices into string 
				_theExpression->Insert(')', 1, rightParenPoint);
				_theExpression->Insert('(', 1, leftParenPoint);
				
				
				
				#ifdef DEBUG
				printf("\t\toperator %c(occurance %d) expression is now %s\n\n", op, i, _theExpression->String());
				#endif
				
			}//if(!isNegativeModifier())...
				
					
		} //for (int i
	} //for (int opIndex

}









