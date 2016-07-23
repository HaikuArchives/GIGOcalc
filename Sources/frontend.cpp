#include "frontend.h"

CalcApp::CalcApp()
	: BApplication( APP_SIG )
{

	BWindow *myWindow = new CalcWindow;
	myWindow->Show();
	
	return;
}

CalcApp::~CalcApp()
{
	return;
}

void CalcApp::AboutRequested()
{
	(new BAlert("About", "A minimalist's expression calculator by Samuel Gutterman & Shamyl Zakariya", "Whoop-dee-doo.", NULL, NULL, B_WIDTH_FROM_LABEL, B_IDEA_ALERT))->Go();
}



//*******************************************************************


CalcView::CalcView(BRect frame, const char *name, int32 resizingmode, int32 flags)
	: BView(frame, name, resizingmode, flags | B_WILL_DRAW)
{
	_theCalc = new Calculator();	
	
	return;
}

CalcView::~CalcView()
{
	delete _theCalc;
	delete _messenger;
	
	return;
}

void CalcView::Draw(BRect update)
{
	rgb_color light = tint_color(ViewColor(), B_LIGHTEN_2_TINT);
	rgb_color dark = tint_color(ViewColor(), B_DARKEN_2_TINT);
	BRect b(Bounds());
	
	SetPenSize(0);
	SetHighColor(light);
	StrokeLine(b.LeftTop(), b.RightTop());
	StrokeLine(b.LeftTop(), b.LeftBottom());
	
	SetHighColor(dark);
	StrokeLine(b.RightTop(), b.RightBottom());
	StrokeLine(b.LeftBottom(), b.RightBottom());

}

void CalcView::AttachedToWindow()
{
	SetViewColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT));
	float margin = 4;

	ZLayoutView *top = new ZLayoutView(Bounds(), "top", new ZBorderLayout(27), B_FOLLOW_ALL_SIDES);
	top->DrawLayout(true);
	AddChild(top);

	ZLayoutView *center = new ZLayoutView(Z_NULLRECT, "center", new ZVerticalLayout());
	top->AddChildAt(center, ZBorderLayout::CENTER);

	ZLayoutView *east = new ZLayoutView(Z_NULLRECT, "east", new ZVerticalLayout());
	top->AddChildAt(east, ZBorderLayout::EAST);
	
	_inputText = new BTextControl(Z_NULLRECT, "_inputText", ">>", "", new BMessage(MSG_TEXT_IN));
	_inputText->SetDivider(be_plain_font->StringWidth(">>") + margin);
	_inputText->SetTarget(this);
		
	_outputText = new BTextControl(Z_NULLRECT, "_outputText", "<<", "", new BMessage(MSG_TEXT_OUT), B_FOLLOW_LEFT_RIGHT | B_FOLLOW_V_CENTER, B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
	_outputText->SetDivider(be_plain_font->StringWidth("<<") + margin);
	_outputText->SetTarget(this);

	center->AddChild(_inputText);
	center->AddChild(_outputText);		

	_settingsMenu = new BMenu("");

	_settingsMenuField = new BMenuField(Z_NULLRECT, "settingsMenuField", NULL, _settingsMenu);
	east->AddChild(_settingsMenuField);


	_selectAnswerItem = new BMenuItem("Select Answer", new BMessage(MSG_SELECT_ANSWER));
	_binItem = new BMenuItem("BIN", new BMessage(MSG_BASE_BIN));
	_octItem = new BMenuItem("OCT", new BMessage(MSG_BASE_OCT));
	_decItem = new BMenuItem("DEC", new BMessage(MSG_BASE_DEC));
	_hexItem = new BMenuItem("HEX", new BMessage(MSG_BASE_HEX));
	_degItem = new BMenuItem ("Degrees", new BMessage(MSG_TRIG_DEGREES));
	_radItem = new BMenuItem ("Radians", new BMessage(MSG_TRIG_RADIANS));
	_aboutItem = new BMenuItem("About...", new BMessage(B_ABOUT_REQUESTED));
	_quitItem = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED));
	
	_baseMenu = new BMenu("Output in base");
	_trigMenu = new BMenu("Trig");
	
	_settingsMenu->AddItem(_selectAnswerItem);
	_settingsMenu->AddSeparatorItem();
	_settingsMenu->AddItem(_baseMenu);
	_settingsMenu->AddItem(_trigMenu);
	_settingsMenu->AddSeparatorItem();
	_settingsMenu->AddItem(_aboutItem);
	_settingsMenu->AddItem(_quitItem);
	
	_baseMenu->AddItem(_binItem);
	_baseMenu->AddItem(_octItem);
	_baseMenu->AddItem(_decItem);	
	_baseMenu->AddItem(_hexItem);

	_trigMenu->AddItem(_degItem);
	_trigMenu->AddItem(_radItem);
	
	_settingsMenu->SetTargetForItems(this);
	_trigMenu->SetTargetForItems(this);
	_baseMenu->SetTargetForItems(this);
	
	_baseMenu->SetRadioMode(true);
	_trigMenu->SetRadioMode(true);
	_settingsMenu->SetRadioMode(false);
	
	_aboutItem->SetTarget(be_app);
	_quitItem->SetTarget(be_app);
				
	top->ResizeToPreferred();
	float h, w;
	top->GetPreferredSize(&w, &h);
	Window()->SetSizeLimits(w, 1000, h, h);

	//I have to create the messenger in AttachedToWindow as it will not
	//initialize correctly until 'this' is registered by the parent looper as a valid handler.
	_messenger = new BMessenger(this);	
	LoadPrefs();
	
	//in case font settings have changed and saved window size is too small
	if (Window()->Bounds().Height() < h) Window()->ResizeTo(h, Window()->Bounds().Width());
}


void CalcView::MessageReceived(BMessage* message)
{

	switch(message->what)
	{
		case MSG_TEXT_IN:
		{
			BString expression, response;
			int selStart = 0, selStop = 0;

			expression.SetTo(_inputText->Text());
			response.SetTo(_outputText->Text());
			
			int error = _theCalc->calculate(&expression, &response, selStart, selStop);
			_outputText->SetText(response.String());			

			if (!error){
				 if (_highlightAnswer) _outputText->MakeFocus();
			}
			else{
				_inputText->SetText(expression.String());
				_inputText->MakeFocus();
				_inputText->TextView()->Select(selStart,selStop);
				}
										
			break;
		}
		
		case MSG_SELECT_ANSWER:
		{
			if (!_selectAnswerItem->IsMarked())
			{
				_selectAnswerItem->SetMarked(true);		
				_highlightAnswer = true;
				_outputText->SetEnabled(true);
				_outputText->TextView()->MakeSelectable(true);
			}
			else {
				_selectAnswerItem->SetMarked(false);
				_highlightAnswer = false;
				_outputText->SetEnabled(false);
				_outputText->TextView()->MakeSelectable(false);
			}
			break;
		}
		
		case MSG_BASE_BIN:
		{		
			_theCalc->setResponseBase(2);
			_messenger->SendMessage(MSG_TEXT_IN);
			break;
		}

		case MSG_BASE_OCT:
		{
			_theCalc->setResponseBase(8);
			_messenger->SendMessage(MSG_TEXT_IN);
			break;
		}

		case MSG_BASE_DEC:
		{
			_theCalc->setResponseBase(10);
			_messenger->SendMessage(MSG_TEXT_IN);
			break;
		}

		case MSG_BASE_HEX:
		{
			_theCalc->setResponseBase(16);
			_messenger->SendMessage(MSG_TEXT_IN);
			break;
		}
		
		case MSG_TRIG_RADIANS:
		{
			_theCalc->useRadians();
			_messenger->SendMessage(MSG_TEXT_IN);
			break;
		}

		case MSG_TRIG_DEGREES:
		{
			_theCalc->useDegrees();
			_messenger->SendMessage(MSG_TEXT_IN);
			break;
		}			
		
		default:
			BView::MessageReceived(message);
	}
}

void CalcView::LoadPrefs(){

	BMessage prefs;
	BFile prefFile;
	BPath prefPath;
	bool prefLoadError = false;
	
	bool selAns = false;
	int32 trigMode, answerBase;
	BRect windowRect;
	const char * lastAnswer;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefPath) != B_OK)
		printf("\tUnable to find default user settings directory\n");
		
	prefPath.Append(PREF_FILE);
	
	
	if (prefFile.SetTo(prefPath.Path(), B_READ_WRITE) == B_OK){
		if (prefs.Unflatten(&prefFile) == B_OK){
			prefLoadError = (prefs.FindBool("selectAnswer", &selAns) != B_OK);
			prefLoadError = (prefs.FindRect("windowFrame", &windowRect) != B_OK);
			prefLoadError = (prefs.FindInt32("trigMode", &trigMode) != B_OK);
			prefLoadError = (prefs.FindInt32("answerBase", &answerBase) != B_OK);
			prefLoadError = (prefs.FindString("lastAnswer", &lastAnswer) != B_OK);
		}
		else
			prefLoadError = true;
	}
	else
		prefLoadError = true;
	
	//load default settings
	if (prefLoadError){
		printf("\tUnable to open preferences file. Using default settings.\n");
		_selectAnswerItem->SetMarked(false);
		_degItem->SetMarked(true);		
		_decItem->SetMarked(true);
		
		_messenger->SendMessage(MSG_BASE_DEC);
		_messenger->SendMessage(MSG_TRIG_DEGREES);		
	}
	else{
		_selectAnswerItem->SetMarked(selAns);
		Window()->ResizeTo(windowRect.Width(), Frame().Height());
		Window()->MoveTo(windowRect.left, windowRect.top);

		_trigMenu->FindItem(trigMode)->SetMarked(true);
		_baseMenu->FindItem(answerBase)->SetMarked(true);		

		_messenger->SendMessage(answerBase);
		_messenger->SendMessage(trigMode);
		
		BString la(lastAnswer);
		_theCalc->setLastAnswer(la);
	}
	
	
	_messenger->SendMessage(MSG_SELECT_ANSWER);
	_inputText->MakeFocus();
}

void CalcView::SavePrefs(){
	//store prefs into a message and serialize it into ~/config/settings/gigocalc_settings
	BMessage prefs;
	prefs.AddBool("selectAnswer", _selectAnswerItem->IsMarked());
	prefs.AddRect("windowFrame", Window()->Frame());
	
	prefs.AddInt32("trigMode", _trigMenu->FindMarked()->Message()->what);
	prefs.AddInt32("answerBase", _baseMenu->FindMarked()->Message()->what);
	prefs.AddString("lastAnswer", _theCalc->getLastAnswer().String());

		
	BFile preffile;
	BPath prefPath;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefPath) != B_OK)
		printf("\tUnable to find default user settings directory\n");
		
	prefPath.Append(PREF_FILE);
	
	if (preffile.SetTo(prefPath.Path(), B_READ_WRITE | B_CREATE_FILE) != B_OK)
		printf("\tUnable to create the preferences file\n");
	
	if (prefs.Flatten(&preffile) != B_OK)
		printf("\tUnable to flatten preferences message\n");
}


//*******************************************************************


CalcWindow::CalcWindow()
	: BWindow( BRect( 100, 100, 400, 170 ), "GIGOcalc 2.0", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_ANCHORED_ON_ACTIVATE | B_NOT_V_RESIZABLE)
{
	_calcView = new CalcView(Bounds(), "calcView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	AddChild(_calcView);
		
	return;
}

CalcWindow::~CalcWindow()
{

	return;
}

bool CalcWindow::QuitRequested()
{

	_calcView->SavePrefs();	
	be_app->PostMessage( B_QUIT_REQUESTED );
	
	return true;
}

void CalcWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		
		default:
			BWindow::MessageReceived(message);
	}
}