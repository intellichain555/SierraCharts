#include "sierrachart.h"
#include <string>

SCDLLName("WTT Target Levels")
/*----------------------------------------------------------------------*/
int RequestData(SCStudyGraphRef sc, SCString url)
{
	SCString UrlFileData;

	int& RequestState = sc.GetPersistentInt(8);

	if (RequestState == HTTP_REQUEST_NOT_SENT)
	{
		const char* p_Username = NULL;
		
		UrlFileData.Format("http://85.239.53.96/getdata/url=%s", url.GetChars());
		
		if (!sc.MakeHTTPRequest(UrlFileData))
		{
			sc.AddMessageToLog("Error Making HTTP Request.", true);
			RequestState = HTTP_REQUEST_ERROR;
		}
		else
			RequestState = HTTP_REQUEST_MADE;

		return 0;
	}

	if (RequestState == HTTP_REQUEST_MADE && sc.HTTPResponse != "")
	{
		RequestState = HTTP_REQUEST_RECEIVED;
		return 1;
	}
	else if (RequestState == HTTP_REQUEST_MADE && sc.HTTPResponse == "")
	{
		return 0;
	}

	if (RequestState != HTTP_REQUEST_RECEIVED)
		return 0;
	
	return 1;
}

/*----------------------------------------------------------------------*/

void ClearRequest(SCStudyGraphRef sc)
{
	int& RequestState = sc.GetPersistentInt(8);
	SCDateTime& RequestDateTime = sc.GetPersistentSCDateTime(2);

	RequestState = HTTP_REQUEST_NOT_SENT;
	RequestDateTime = sc.CurrentSystemDateTime;
}

/*----------------------------------------------------------------------*/

void GetItemArrayFromString(const SCString& Line, SCString* DataArray, int ArraySize, int Zonid)
{
	char FieldsDelimitter = '\t';

	int find_delim = Line.IndexOf(FieldsDelimitter);
	unsigned int pos = 0;

	int CurrentItem = 0;
	
	while (find_delim > 0 && CurrentItem < ArraySize)
	{
		SCString NextStr = Line.GetSubString(find_delim - pos, pos);

		pos = find_delim + 1;
		find_delim = Line.IndexOf(FieldsDelimitter, find_delim + 1);
		DataArray[CurrentItem+ Zonid*6] = NextStr;
		CurrentItem++;
	}
	if (find_delim < 0)
		DataArray[CurrentItem+ Zonid*6] = Line.GetSubString(Line.GetLength(), pos);
}

/*----------------------------------------------------------------------*/

/*

*/
SCSFExport scsf_ToTheTickStudy(SCStudyGraphRef sc)
{
	const int ZonesCount = 200;

	SCSubgraphRef AroonOscillator = sc.Subgraph[ZonesCount * 2];
	
	SCInputRef LevelsOpacity = sc.Input[0];
	SCInputRef LISColor = sc.Input[1];
	SCInputRef BBZColor = sc.Input[2];
	SCInputRef BullsGateColor = sc.Input[3];
	SCInputRef BearsGateColor = sc.Input[4];
	SCInputRef KeyLevelColor = sc.Input[5];
	SCInputRef LTFColor = sc.Input[6];
	SCInputRef TargetColor = sc.Input[7];
	SCInputRef OtherColor = sc.Input[8];
	SCInputRef TextColor = sc.Input[9];
	SCString FileURL = "https://www.dropbox.com/s/3sefkusgd9p653m/6E_LVL.azr?dl=1";
	int UpdateTime = 1;//5min
	if (sc.SetDefaults)
	{
		sc.GraphName = "WTT Target Levels";

		sc.GraphRegion = 0;
		sc.ScaleRangeType = SCALE_SAMEASREGION;
		sc.ValueFormat = VALUEFORMAT_INHERITED;
		sc.AutoLoop = 0; //Not using automatic looping.
		sc.FreeDLL = 0;
		sc.DrawStudyUnderneathMainPriceGraph = true;
		sc.DisplayStudyInputValues = false;
		sc.GlobalDisplayStudySubgraphsNameAndValue = false;
		SCString SubgraphName = " ";
		for (int i = 0; i < 2 * ZonesCount; i += 2)
		{

			sc.Subgraph[i ].Name = SubgraphName;
			sc.Subgraph[i ].DrawStyle = DRAWSTYLE_TRANSPARENT_TEXT;
			sc.Subgraph[i ].PrimaryColor = TextColor.GetColor(); // Light blue
			sc.Subgraph[i ].DrawZeros = 0;
			sc.Subgraph[i ].LineLabel = LL_DISPLAY_NAME + LL_NAME_ALIGN_ABOVE;
			sc.Subgraph[i ].DisplayNameValueInDataLine = SNV_DISPLAY_IN_DATA_LINE;
			sc.Subgraph[i ].UseTransparentLabelBackground = 1;

			sc.Subgraph[i + 1].Name = SubgraphName;
			sc.Subgraph[i + 1].DrawStyle = DRAWSTYLE_TRANSPARENT_TEXT;
			sc.Subgraph[i + 1].PrimaryColor = TextColor.GetColor(); // Light blue
			sc.Subgraph[i + 1].DrawZeros = 0;
			sc.Subgraph[i + 1].LineLabel = LL_DISPLAY_NAME + LL_NAME_ALIGN_ABOVE + LL_NAME_ALIGN_RIGHT;
			sc.Subgraph[i + 1].DisplayNameValueInDataLine = SNV_DISPLAY_IN_DATA_LINE;
			sc.Subgraph[i + 1].UseTransparentLabelBackground = 1;

		}



		LevelsOpacity.Name = "Levels Opacity(0-100)";
		LevelsOpacity.SetInt(75);

		LISColor.Name = "LIS Color";
		LISColor.SetColor(RGB(255, 5, 224));
		BBZColor.Name = "BBZ Color";
		BBZColor.SetColor(RGB(92, 164, 224));
		BullsGateColor.Name = "Bulls Gate Color";
		BullsGateColor.SetColor(RGB(255, 212, 2));
		BearsGateColor.Name = "Bears Gate Color";
		BearsGateColor.SetColor(RGB(255, 212, 2));
		KeyLevelColor.Name = "Key Level Color";
		KeyLevelColor.SetColor(RGB(92, 164, 224));
		LTFColor.Name = "LTF Zones Color";
		LTFColor.SetColor(RGB(22, 247, 246));
		TargetColor.Name = "Target Color";
		TargetColor.SetColor(RGB(203, 33, 70));
		
		OtherColor.Name = "Other Zones Color";
		OtherColor.SetColor(RGB(106, 149, 233));
		
		TextColor.Name = "Text Color";
		TextColor.SetColor(RGB(255, 255, 255));

		return;
	}

	SCString ShortSymbolName = sc.Symbol.GetSubString(2);

	SCDateTime RemoteFileUpdateTime = sc.CurrentSystemDateTime;

	int& RequestState = sc.GetPersistentInt(8);
	int PriorRequestState = RequestState;
	SCDateTime& RequestDateTime = sc.GetPersistentSCDateTime(2);

	//if (sc.UpdateStartIndex == 0 && RequestState == HTTP_REQUEST_MADE)
		//ClearRequest(sc);  // This is to solve a condition when Apply is used from the Chart Studies window, and the initial state of the RequestState is remembered
	//else 
	SCDateTime TimeInterval = SCDateTime::MINUTES(UpdateTime);
	if (RequestDateTime.GetTime() == 0
		|| sc.UpdateStartIndex == 0
		|| ((sc.CurrentSystemDateTime.GetTime() - RequestDateTime.GetTime() >= TimeInterval.GetTime())
			&& RequestDateTime.GetTime() != sc.CurrentSystemDateTime.GetTime())
		)
		ClearRequest(sc);

	if (!RequestData(sc, FileURL))
		return;

	SCString& Response = sc.HTTPResponse;

	bool RequestStateChanged = PriorRequestState != RequestState;

	if (atof(Response.GetSubString(2)) > 0)
	{
		if (RequestStateChanged)
			sc.AddMessageToLog("Received valid data.", 0);
	}
	else
	{
		if (RequestStateChanged)
			sc.AddMessageToLog("Unknown server error.", 1);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Parsing Data
	//////////////////////////////////////////////////////////////////////////

	char LinesDelimitter = '\n';
	char FieldsDelimitter = '\t';
	SCString NextLine;

	int find_delim = Response.IndexOf(LinesDelimitter);
	unsigned int pos = 0;

	int Index = sc.ArraySize - 1;
	SCDateTime LastChartDateTime = sc.BaseDateTimeIn[Index];
	int ZoneId = 0;
	SCString CurrentZonesData[ZonesCount * 6] = { " " };
	while (find_delim > 0)
	{
		NextLine = Response.GetSubString(find_delim - pos, pos);
		pos = find_delim + 1;
		find_delim = Response.IndexOf(LinesDelimitter, find_delim + 1);


		NextLine = NextLine.GetSubString(NextLine.GetLength() - 1); // remove '\r' character
		int find_last = NextLine.LastIndexOf(FieldsDelimitter, NextLine.GetLength() - 1);
		if (find_last < 0)
			continue;

		GetItemArrayFromString(NextLine, CurrentZonesData, ZonesCount, ZoneId);
		ZoneId++;
	}
	s_UseTool Tool;
	 
	for (int i = 0; RequestState != HTTP_REQUEST_NOT_SENT && i < ZonesCount; i += 1)
	{
		Tool.Clear();
		Tool.ChartNumber = sc.ChartNumber;
		Tool.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
		int& r_LineNumber = sc.GetPersistentInt(i+10);
		if (r_LineNumber != 0)
			Tool.LineNumber = r_LineNumber;
		Tool.BeginDateTime = sc.BaseDateTimeIn[0];

		Tool.EndValue = (float)atof(CurrentZonesData[6 * i]) + (float)atof(CurrentZonesData[6 * i + 1]) * sc.TickSize;
		Tool.BeginValue = (float)atof(CurrentZonesData[6 * i]);
		Tool.Color = RGB(255, 0, 0);  // Red
		Tool.LineWidth = 0; //To see the outline this must be 1 or greater.
//		Tool.SecondaryColor = LTFColor.GetColor();
		Tool.TransparencyLevel = LevelsOpacity.GetInt();
		Tool.AddMethod = UTAM_ADD_OR_ADJUST;
		Tool.AddAsUserDrawnDrawing = 0;
		
		
		if (CurrentZonesData[6 * i + 5].Compare("LTF") == 0)
		{
			Tool.SecondaryColor = LTFColor.GetColor();
			
		}
		else if (CurrentZonesData[6 * i + 5].Compare("LIS") == 0)
		{
			Tool.SecondaryColor = LISColor.GetColor();
		}
		else if (CurrentZonesData[6 * i + 5].Compare("BUG") == 0)
		{
			Tool.SecondaryColor = BullsGateColor.GetColor();
		}
		else if (CurrentZonesData[6 * i + 5].Compare("BEG") == 0)
		{
			Tool.SecondaryColor = BearsGateColor.GetColor();
		}
		else if (CurrentZonesData[6 * i + 5].Compare("KLV") == 0)
		{
			Tool.SecondaryColor = KeyLevelColor.GetColor();
		}
		else if (CurrentZonesData[6 * i + 5].Compare("LVL") == 0)
		{
			Tool.SecondaryColor = OtherColor.GetColor();
		}
		else if (CurrentZonesData[6 * i + 5].Compare("BBZ") == 0)
		{
			Tool.SecondaryColor = BBZColor.GetColor();
		}
		else if (CurrentZonesData[6 * i + 5].Compare("TGT") == 0)
		{
			Tool.SecondaryColor = TargetColor.GetColor();
		}
		else
		{
			Tool.SecondaryColor = OtherColor.GetColor();
		}


		sc.UseTool(Tool);
		r_LineNumber = Tool.LineNumber;//Remember line number which has been automatically set

		SCString tmp = "";
		float tickdigit = sc.TickSize;
		int nDigits = 0;
		float tmpVal1, tmpVal2;
		while (tickdigit < 1 && (float)((int)tickdigit) != tickdigit)
		{
			tickdigit = tickdigit * 10;
			nDigits++;
			//int Return = sc.FormattedEvaluate(tickdigit, "%0f", EQUAL_OPERATOR, tickdigit, "%1f");
		}
		
		SCString tmp1 = "";
		SCString tmp3 = "%.";
		SCString tmp2 = "";
		tmp2.Format("%d", nDigits);
		tmp3.Append(tmp2);
		tmp3.Append("f");
		tmpVal1 = (float)atof(CurrentZonesData[6 * i]);
		tmpVal2 = (float)atof(CurrentZonesData[6 * i]) + (float)atof(CurrentZonesData[6 * i + 1]) * sc.TickSize;
		tmp1.Format(tmp3.GetChars(), tmpVal1);
		tmp = tmp.Append(tmp1);
		tmp = tmp.Append(" - ");
		tmp1.Format(tmp3.GetChars(), tmpVal2);
		tmp = tmp.Append(tmp1);

		
		sc.Subgraph[i * 2 + 1].Name = tmp;
		sc.Subgraph[i * 2 + 1].PrimaryColor = TextColor.GetColor();

		sc.Subgraph[i * 2].Name = CurrentZonesData[6 * i + 4];
		sc.Subgraph[i * 2].PrimaryColor = TextColor.GetColor();

	}


	sc.TransparencyLevel = LevelsOpacity.GetInt();
	
	while (true)
	{
		if (Index < 0)
			break;
		for (int i = 0; i < ZonesCount; i += 1)
		{
			sc.Subgraph[i * 2][Index] = (float)atof(CurrentZonesData[6 * i]);
			sc.Subgraph[i * 2 + 1][Index] = (float)atof(CurrentZonesData[6 * i]);
		}

		Index--;
	}

}
