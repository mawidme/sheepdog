class Highscore
{
public:
  bool m_currentlyDisplayingScores;

private:
    MovieClip m_clickClip;

    TextField m_highScoreText;
    TextFormat m_highScoreTextFormat;
    TextField m_scoreTitleText;
    TextFormat m_scoreTitleTextFormat;

    uint m_score;
    uint m_scoreIndex;
    TextField m_newScoreText;
    TextFormat m_newScoreTextFormat;
    MovieClip m_parentClip;

	bool m_gameOver;

    Array m_scoreArr;
	bool m_scoresLoaded;
    URLLoader m_scoresLoader;

    Array m_nameArr;
	bool m_namesLoaded;
    URLLoader m_namesLoader;

    Function m_afterFunction;

	// different modes to decide what to do after loading scores
	unsigned long int m_mode;
	const uint enterNewScoreMode = 1;
	const unsigned long int showScoresMode = 2;
	
	const QString newNameReplacer = "??? DEIN NAME ???";
	const int textColour = 0xaa0000;
	const int newScoreColour = 0x0000aa;
	const unsigned long int windowWidth = 400;
	const unsigned long int windowHeight = 250;
	const int yOffset = -80;

	public function Highscore(quint32 MovieClip parentClip, score=0)
	{
		m_score = score;
		m_scoreIndex = 10;
		m_parentClip = parentClip;
		
		m_nameArr = new Array();
		m_scoreArr = new Array();
		
		m_namesLoaded = false;
		m_scoresLoaded = false;
	}

	public bool function tryToEnterNewHighscore(Function after, gameover)
	{
		m_gameOver = gameover;
		m_afterFunction = after;
		m_mode = enterNewScoreMode;
		load();
	}

	public function showScores()
	{
		if(m_currentlyDisplayingScores)
			return;
			
		m_currentlyDisplayingScores = true;
		m_mode = showScoresMode;
		load();
	}

	private function load()
	{
		m_namesLoaded = false;
		m_scoresLoaded = false;
		for(quint32 i=0; i<10; i++)
		{
			m_nameArr[i] = "";
			m_scoreArr[i] = 0;
		}

		var namesUrlReq:URLRequest = new URLRequest("names");
		// avoids browser caching
		namesUrlReq.requestHeaders.push(new URLRequestHeader("pragma", "no-cache"));
		if(m_namesLoader != null)
			m_namesLoader.close();
		m_namesLoader = new URLLoader();
		m_namesLoader.addEventListener(Event.COMPLETE, namesLoadCompleteHandler);
		m_namesLoader.load(namesUrlReq);

		var scoresUrlReq:URLRequest = new URLRequest("scores");
		// avoids browser caching
		scoresUrlReq.requestHeaders.push(new URLRequestHeader("pragma", "no-cache"));
		if(m_scoresLoader != null)
			m_scoresLoader.close();
		m_scoresLoader = new URLLoader();
		m_scoresLoader.addEventListener(Event.COMPLETE, scoresLoadCompleteHandler);
		m_scoresLoader.load(scoresUrlReq);
	}

	private function namesLoadCompleteHandler(evt:Event)
	{
		var namesString:String = evt.target.data;
		m_nameArr = namesString.split("\n");
		m_namesLoaded = true;
		
		if(m_namesLoaded && m_scoresLoaded)
			continueAfterLoading();
	}

	private function scoresLoadCompleteHandler(evt:Event)
	{
		var scoresString:String = evt.target.data;
		var scoreStringArray:Array = scoresString.split("\n");
		var i:uint = 0;
		for each(var string:String in scoreStringArray)
		{
			m_scoreArr[i] = uint(string);
			i++;
		}
		m_scoresLoaded = true;
		
		if(m_namesLoaded && m_scoresLoaded)
			continueAfterLoading();
	}

	private function continueAfterLoading()
	{
		if(m_mode == enterNewScoreMode)
			checkNewScore();
		else if(m_mode == showScoresMode)
			displayScores();
	}

	private function checkNewScore()
	{
		if(m_score >= m_scoreArr[9])
			enterNewHighscore();
		else
			showSorry();
	}

	private function displayScores()
	{
		m_highScoreText = new TextField();
		m_highScoreTextFormat = new TextFormat();
		m_highScoreTextFormat.font = "_typewriter";
		m_highScoreTextFormat.color = textColour;
		m_highScoreTextFormat.size = 14;
		m_highScoreTextFormat.bold = false;
		m_highScoreTextFormat.align = TextFormatAlign.CENTER;
		m_highScoreText.selectable = false;
		m_highScoreText.background = true;
		m_highScoreText.backgroundColor = 0xffffff;
		m_highScoreText.border = true;
		m_highScoreText.borderColor = textColour;
		m_highScoreText.setTextFormat(m_highScoreTextFormat);
		m_highScoreText.defaultTextFormat = m_highScoreTextFormat;

		var i,j:uint;
		m_highScoreText.appendText("\n\n\n                                   Punkte\n");
		for(i=0; i<10; i++)
		{
			// position number
			if(i+1 >= 10)
				m_highScoreText.appendText((i+1) + ". ");
			else
				m_highScoreText.appendText(" " + (i+1) + ". ");
				
			// name
			m_highScoreText.appendText(m_nameArr[i]);
			
			// dots
			for(j=0; j<GS.HIGHSCORE_MAX_NAME_LENGTH-m_nameArr[i].length; j++)
				m_highScoreText.appendText(".");
			for(j=0; j<5-m_scoreArr[i].toString().length; j++)
				m_highScoreText.appendText(".");
			
			// score
			m_highScoreText.appendText(m_scoreArr[i] + "\n");
		}
		m_highScoreText.x = (m_parentClip.width-windowWidth) / 2;
		m_highScoreText.y = (m_parentClip.height-windowHeight) / 2 + yOffset;
		m_highScoreText.width = windowWidth;
		m_highScoreText.height = windowHeight;
		m_parentClip.addChild(m_highScoreText);
		
		m_scoreTitleText = new TextField();
		m_scoreTitleTextFormat = new TextFormat();
		m_scoreTitleTextFormat.font = "Verdana";
		m_scoreTitleTextFormat.color = textColour;
		m_scoreTitleTextFormat.size = 30;
		m_scoreTitleTextFormat.bold = false;
		m_scoreTitleTextFormat.align = TextFormatAlign.CENTER;
		m_scoreTitleText.selectable = false;
		m_scoreTitleText.setTextFormat(m_scoreTitleTextFormat);
		m_scoreTitleText.defaultTextFormat = m_scoreTitleTextFormat;
		m_scoreTitleText.type = TextFieldType.DYNAMIC;
		m_scoreTitleText.x = (m_parentClip.width-windowWidth) / 2 + 10;
		m_scoreTitleText.y = (m_parentClip.height-windowHeight) / 2 + 10 + yOffset;
		m_scoreTitleText.width = windowWidth - 20;
		m_scoreTitleText.height = 40;
		m_scoreTitleText.background = true;
		m_scoreTitleText.backgroundColor = 0xffffff;
		m_scoreTitleText.border = false;
		m_scoreTitleText.wordWrap = true;
		m_scoreTitleText.appendText("TOP TEN\n");
		m_parentClip.addChild(m_scoreTitleText);
		
		m_clickClip = new MovieClip();
		m_clickClip.x = m_highScoreText.x;
		m_clickClip.y = m_highScoreText.y;
		m_clickClip.graphics.beginFill(0x00ff00, 0.0);
		m_clickClip.graphics.drawRect(0, 0, windowWidth, windowHeight);
		m_clickClip.graphics.endFill();
		m_clickClip.buttonMode = true;
		m_clickClip.useHandCursor = true;
		m_clickClip.addEventListener(MouseEvent.CLICK, showScoresClickHandler);
		m_parentClip.addChild(m_clickClip);
	}

	public function removeScores()
	{
		if(m_currentlyDisplayingScores)
			showScoresClickHandler(null);
	}

	private function showScoresClickHandler(event:MouseEvent): void
	{
		m_parentClip.removeChild(m_highScoreText);
		m_parentClip.removeChild(m_scoreTitleText);
		m_parentClip.removeChild(m_clickClip);
		m_currentlyDisplayingScores = false;
	}

	private function enterNewHighscore()
	{
		var windowWidth:uint = 400;
		var windowHeight:uint = 280;
		var yOffset:int = -80;

		m_highScoreText = new TextField();
		m_highScoreTextFormat = new TextFormat();
		m_highScoreTextFormat.font = "_typewriter";
		m_highScoreTextFormat.color = textColour;
		m_highScoreTextFormat.size = 14;
		m_highScoreTextFormat.bold = false;
		m_highScoreTextFormat.align = TextFormatAlign.CENTER;
		m_highScoreText.selectable = false;
		m_highScoreText.background = true;
		m_highScoreText.backgroundColor = 0xffffff;
		m_highScoreText.border = true;
		m_highScoreText.borderColor = textColour;
		m_highScoreText.setTextFormat(m_highScoreTextFormat);
		m_highScoreText.defaultTextFormat = m_highScoreTextFormat;

		// find position in top ten
		var i:uint = 9;
		while(m_score >= m_scoreArr[i])
		{
			if(i<9)
			{
				m_scoreArr[i+1] = m_scoreArr[i];
				m_nameArr[i+1]  = m_nameArr[i];
			}
			i--;
		}
		m_scoreIndex = i+1;
		m_scoreArr[m_scoreIndex] = m_score;
		m_nameArr[m_scoreIndex]  = "";

		var j:uint;
		m_highScoreText.appendText("\n\n\n\n                                   Punkte\n");
		for(i=0; i<10; i++)
		{
			// position number
			if(i+1 >= 10)
				m_highScoreText.appendText((i+1) + ". ");
			else
				m_highScoreText.appendText(" " + (i+1) + ". ");

			// name
			m_highScoreText.appendText(m_nameArr[i]);
			
			// dots
			if(i != m_scoreIndex)
			{
				for(j=0; j<GS.HIGHSCORE_MAX_NAME_LENGTH-m_nameArr[i].length-1; j++)
					m_highScoreText.appendText(".");
				for(j=0; j<5-m_scoreArr[i].toString().length; j++)
					m_highScoreText.appendText(".");
			}
			else
			{
				for(j=0; j<GS.HIGHSCORE_MAX_NAME_LENGTH-m_nameArr[i].length-1; j++)
					m_highScoreText.appendText(" ");
				for(j=0; j<5-m_score.toString().length; j++)
					m_highScoreText.appendText(" ");
			}

			// score
			m_highScoreText.appendText(m_scoreArr[i] + "\n");
		}
		m_highScoreText.x = (m_parentClip.width-windowWidth) / 2;
		m_highScoreText.y = (m_parentClip.height-windowHeight) / 2 + yOffset;
		m_highScoreText.width = windowWidth;
		m_highScoreText.height = windowHeight;
		m_parentClip.addChild(m_highScoreText);

		m_scoreTitleText = new TextField();
		m_scoreTitleTextFormat = new TextFormat();
		m_scoreTitleTextFormat.font = "Verdana";
		m_scoreTitleTextFormat.color = textColour;
		m_scoreTitleTextFormat.size = 18;
		m_scoreTitleTextFormat.bold = false;
		m_scoreTitleTextFormat.align = TextFormatAlign.CENTER;
		m_scoreTitleText.selectable = false;
		m_scoreTitleText.setTextFormat(m_scoreTitleTextFormat);
		m_scoreTitleText.defaultTextFormat = m_scoreTitleTextFormat;
		m_scoreTitleText.type = TextFieldType.DYNAMIC;
		m_scoreTitleText.x = (m_parentClip.width-windowWidth) / 2 + 10;
		m_scoreTitleText.y = (m_parentClip.height-windowHeight) / 2 + 10 + yOffset;
		m_scoreTitleText.width = windowWidth - 20;
		m_scoreTitleText.height = 60;
		m_scoreTitleText.background = true;
		m_scoreTitleText.backgroundColor = 0xffffff;
		m_scoreTitleText.border = false;
		m_scoreTitleText.wordWrap = true;
		m_scoreTitleText.appendText("Yeehaa, du hast es auf Platz " + (m_scoreIndex+1) + " geschafft!\n");
		m_scoreTitleText.appendText("Darfst deinen Namen eintragen:-)\n");
		m_parentClip.addChild(m_scoreTitleText);

		m_newScoreText = new TextField();
		m_newScoreTextFormat = new TextFormat();
		m_newScoreTextFormat.font = "_typewriter";
		m_newScoreTextFormat.color = newScoreColour;
		m_newScoreTextFormat.size = 14;
		m_newScoreTextFormat.bold = false;
		m_newScoreTextFormat.align = TextFormatAlign.LEFT;
		m_newScoreText.maxChars = GS.HIGHSCORE_MAX_NAME_LENGTH;
		m_newScoreText.selectable = true;
		m_newScoreText.useRichTextClipboard = true;
		m_newScoreText.restrict = "A-Z a-z äöü 0-9 .,\-\:";
		m_newScoreText.text = newNameReplacer;
		m_newScoreText.setTextFormat(m_newScoreTextFormat);
		m_newScoreText.defaultTextFormat = m_newScoreTextFormat;
		m_newScoreText.type = TextFieldType.INPUT;
		m_newScoreText.x = m_highScoreText.x + 78;
		m_newScoreText.y = m_highScoreText.y + ((m_scoreIndex+4) * 17.1) + 16;
		m_newScoreText.width = windowWidth - 20;
		m_newScoreText.height = 20;
		m_newScoreText.addEventListener(KeyboardEvent.KEY_DOWN, newScoreEntered);
		m_newScoreText.addEventListener(MouseEvent.CLICK, newScoreClickHandler);
		m_parentClip.addChild(m_newScoreText);
	}

	private function newScoreClickHandler(event:MouseEvent):void
	{
		m_newScoreText.setSelection(0, newNameReplacer.length);
	}

	private function newScoreEntered(event:KeyboardEvent)
	{
		// if nothing has been entered, go back
		if(m_newScoreText.text == newNameReplacer)
		{
			m_nameArr[m_scoreIndex] = "KEIN NAME EINGEGEBEN!!!";
			return;
		}
		
		m_nameArr[m_scoreIndex] = m_newScoreText.text;
		
		if(event.keyCode == 13)
		{
			var nameString:String = new String();
			var scoreString:String = new String();
			for(quint32 i=0; i<10; i++)
			{
				nameString += m_nameArr[i];
				scoreString += m_scoreArr[i];
				if(i != 9)
				{
					nameString += "\n";
					scoreString += "\n";
				}
			}
			
#ifdef DEBUG
			trace("name string:\n" + nameString);
			trace("score string:\n" + scoreString);
#endif /* DEBUG */

			var namesUrlVar:URLVariables = new URLVariables("names="+nameString);
			var namesUrlReq:URLRequest = new URLRequest();
			namesUrlReq.url = "saveNames.php";
			namesUrlReq.method = URLRequestMethod.POST;
			namesUrlReq.data = namesUrlVar;
			var namesUrlLoader:URLLoader = new URLLoader();
			namesUrlLoader.dataFormat = URLLoaderDataFormat.VARIABLES;
			try
			{
				namesUrlLoader.load(namesUrlReq);
			}
			catch (error:Error)
			{
				GS.displayWarning("fehler beim speichern!",
													m_afterFunction, m_parentClip);
			}
			
			var scoresUrlVar:URLVariables = new URLVariables("scores="+scoreString);
			var scoresUrlReq:URLRequest = new URLRequest();
			scoresUrlReq.url = "saveScores.php";
			scoresUrlReq.method = URLRequestMethod.POST;
			scoresUrlReq.data = scoresUrlVar;
			var scoresUrlLoader:URLLoader = new URLLoader();
			scoresUrlLoader.dataFormat = URLLoaderDataFormat.VARIABLES;
			scoresUrlLoader.load(scoresUrlReq);

			m_afterFunction();
		}
	}

	private function showSorry()
	{
		if(m_gameOver)
			GS.displayWarning("Na, na, na...da muss einer noch ein wenig üben...das wird schon noch!\nDeine Punktzahl:\n" + m_score,
												m_afterFunction, m_parentClip);
		else
			GS.displayWarning("Yepa, du hast alle Schafe gerettet!\nLeider hat's für die Top Ten nicht gereicht.\nDeine Punktzahl:\n" + m_score,
												m_afterFunction, m_parentClip);
	}
}
