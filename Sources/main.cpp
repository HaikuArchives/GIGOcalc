#include "frontend.h"

int main( int argc, char **argv )
{

	if (argc == 1){
		
		BApplication *thisApp = new CalcApp;
		thisApp->Run();
		delete thisApp;	
	}
	else{
	
		Calculator theCalc;
		BString expression, response;
		int selStart = 0, selStop = 0;

		expression.SetTo(argv[1]);
			
		int error = theCalc.calculate(&expression, &response, selStart, selStop);

		if (!error){
			printf("%s\n", response.String());
		}
		else{
			printf("There was a syntactical error: %s\n", response.String());
		}

	}
	return 0;
} 
