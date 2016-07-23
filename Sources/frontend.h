#ifndef FRONTEND_H
#define FRONTEND_H

#include <Application.h>
#include <InterfaceKit.h>
#include <Path.h>
#include <File.h>
#include <FindDirectory.h>

#include "calculator.h"
#include "zlayout.h"

#define APP_SIG "application/x-vnd.gaz.gigocalc"
#define PREF_FILE "gigocalc_settings"


#define MSG_TEXT_IN 'IN'
#define MSG_TEXT_OUT 'OUT'
#define MSG_SELECT_ANSWER 'SA'
#define MSG_TRIG_DEGREES 'DEGR'
#define MSG_TRIG_RADIANS 'RAD'
#define MSG_BASE_DEC 'DEC'
#define MSG_BASE_HEX 'HEX'
#define MSG_BASE_BIN 'BIN'
#define MSG_BASE_OCT 'OCT'




class CalcApp: public BApplication{
	public:
		CalcApp();
		virtual ~CalcApp();
		
		void AboutRequested();
		
};

class CalcView: public BView{
	private:
		BMessenger *_messenger;
	
		Calculator *_theCalc;

		BTextControl *_inputText, *_outputText;
				
		BMenuField *_settingsMenuField;
		BMenu *_settingsMenu, *_baseMenu, *_trigMenu;
		BMenuItem *_selectAnswerItem, *_binItem, *_hexItem, *_decItem, *_octItem, *_degItem, *_radItem, *_aboutItem, *_quitItem;
			
		bool _highlightAnswer;

	public:
		CalcView(BRect frame, const char * name, int32 resizingmode, int32 flags);
		virtual ~CalcView();
		virtual void AttachedToWindow();
		virtual void MessageReceived(BMessage* message);
		virtual void Draw(BRect update);
		
		void LoadPrefs();
		void SavePrefs();

};

class CalcWindow: public BWindow{
	private:
		CalcView *_calcView;
		
	public:
		CalcWindow();
		virtual ~CalcWindow();
		virtual bool QuitRequested();
		virtual void MessageReceived(BMessage* message);
};

#endif