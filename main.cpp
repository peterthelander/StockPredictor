#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <map>
#include "dirent.h"
using namespace std;

#pragma warning(disable:4786) 

/*
		US data goes: 1-Jan-1968  ... 30-sep-2002
		ASX data goes: Tue 3-Jan-1984 ... 26-Jun-2002
		Common Range: US Mon 2-Jan-1984 ... Thu 20-Jun-2002
		             ASX Tue 3-Jan-1984 ... Fri 21-Jun-2002
*/

typedef int DateLong;

const DateLong DATE_START = 19840102; // Monday 02-Jan-1984
const DateLong DATE_END   = 20020621; // Friday 21-Jun-2002

const char* PATH_ASX_Text = "C:\\Peter\\Pisa\\Data\\ASX_Text";
const char* PATH_US_Text  = "C:\\Peter\\Pisa\\Data\\US_Text\\index";
const char* PATH_ASX_Data = "C:\\Peter\\Pisa\\Data\\ASX_Data";
const char* PATH_US_Data  = "C:\\Peter\\Pisa\\Data\\US_Data";

const int numASXStocks = 69;
const int numUSStocks = 813;

// A  US  stock array stores values in sequence, MON-TUE-WED-THU-FRI-MON-TUE...
// An ASX stock array stores values in sequence, TUE-WED-THU-FRI-MON-TUE-WED...

const int NUM_DATES = 3856 * 5/4; // number of days from start end (inclusive), taking four week days per week

DateLong  US_DateGuide[NUM_DATES];
DateLong ASX_DateGuide[NUM_DATES];

//                        J   F   M   A   M   J   J   A   S   O   N   D;
int daysInMonths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


bool isLeap(int year)
{
	// data only covering ~ 1984 to 2002, so don't care about other centuries like 1900
	return (year % 4 == 0);
}

void incrementDateLong(DateLong& d)
{
	int year = d/10000;
	int month = (d%10000) / 100;
	int day = (d%100);

	day++;

	int days = daysInMonths[month-1];
	if (month == 2 && isLeap(year))
		days = 29;

	if (day > days)
	{
		day = 1;
		month ++;
		if (month == 13)
		{
			month = 1;
			year ++;
		}
	}

	d = (year * 100 + month)*100 + day;
}

	int US_index = 0;
	int ASX_index = 0;

void generateDateGuides()
{
	DateLong d = DATE_START; // first monday, for us stocks

	while (d < DATE_END)
	{
		// d is on a monday
		US_DateGuide[US_index++] = d; // take a monday at 0
		incrementDateLong(d);
		US_DateGuide[US_index++] = d; // take a tuesday at 1
		ASX_DateGuide[ASX_index++] = d; // take a tuesday at 0
		incrementDateLong(d);
		US_DateGuide[US_index++] = d; // take a wednesday at 2
		ASX_DateGuide[ASX_index++] = d; // take a wednesday at 1
		incrementDateLong(d);
		US_DateGuide[US_index++] = d; // take a thursday at 3
		ASX_DateGuide[ASX_index++] = d; // take a thursday at 2
		incrementDateLong(d);
		US_DateGuide[US_index++] = d; // take a friday at 4
		ASX_DateGuide[ASX_index++] = d; // take a friday at 3
		incrementDateLong(d); // d now on saturday
		incrementDateLong(d); // d now on sunday
		incrementDateLong(d); // d now on monday
		ASX_DateGuide[ASX_index++] = d; // take a monday at 4
	}
}

typedef float Daily;

typedef Daily Stock[NUM_DATES];

Daily undefined = 99999.0;

enum StockType { ASX, US };

int loadCount;

void loadDataStock(const char* stockName, Stock& stockData)
{
	FILE *fp = fopen(stockName, "rb");
	fread(&stockData[0], NUM_DATES, sizeof(Daily), fp);
	fclose(fp);
}

DateLong StrToDateLong(char* s)
{
	char *dash = strstr(s, "-");
	char* p = dash+1;
	*dash = 0;

	char *dash2 = strstr(p, "-");
	*dash2 = 0;

	int day = ::atol(s);

	int mon;

		 if (strcmp(p, "Jan")==0) mon = 1;
	else if (strcmp(p, "Feb")==0) mon = 2;
	else if (strcmp(p, "Mar")==0) mon = 3;
	else if (strcmp(p, "Apr")==0) mon = 4;
	else if (strcmp(p, "May")==0) mon = 5;
	else if (strcmp(p, "Jun")==0) mon = 6;
	else if (strcmp(p, "Jul")==0) mon = 7;
	else if (strcmp(p, "Aug")==0) mon = 8;
	else if (strcmp(p, "Sep")==0) mon = 9;
	else if (strcmp(p, "Oct")==0) mon = 10;
	else if (strcmp(p, "Nov")==0) mon = 11;
	else if (strcmp(p, "Dec")==0) mon = 12;

	p = dash2+1;
	int year = ::atol(p);
	if (year < 50) year += 100;
	year += 1900;

	return (year * 100 + mon) * 100 + day;
}

void loadStock(StockType stockType, const char* stockName, Stock& stockData)
{
	if (strstr(stockName, ".DAT"))
	{
		loadDataStock(stockName, stockData);
		return;
	}

	const char* path;
	const char* ext;
	DateLong* dateGuide;

	loadCount = 0;

	if (stockType == ASX)
	{
		dateGuide = ASX_DateGuide;
		path = PATH_ASX_Text;
		ext = ".CSV";
	}
	else
	{
		dateGuide = US_DateGuide;
		path = PATH_US_Text;
		ext = ".ASC";
	}

	for (int i=0; i<NUM_DATES; i++)
		stockData[i] = undefined;

	string file;

	if (strstr(stockName, "\\"))
	{
		file = stockName; // full filename provided
	}
	else
	{
		file = path;
		file += "\\";
		file += stockName;
		file += ext;
	}

	const int buf_size = 500000;

	static char buf[buf_size];

	memset(buf, 0, buf_size);

	FILE* fp = fopen(file.c_str(), "r");

	size_t bytes = fread(buf, buf_size, 1, fp);

	fclose(fp);

	const char* delims = ",\n";

	char* p = strtok(buf, delims);

	if (strcmp(p, "Date")==0)
	{
		// p at "Date"
		p = strtok(0, "\n");// p at Open,High...
		p = strtok(0, "\n");// p at first data line, "17-Dec-04,123.45,..."

		// yahoo-format stock, need to read backwards
		int index = NUM_DATES - 1;

		while (index >=0)
		{
			char *dateStr = strtok(0, delims);

			if (dateStr == 0)
				break;

			DateLong d = StrToDateLong(dateStr);

			char * open = strtok(0, delims);
			char * high = strtok(0, delims);
			char * low = strtok(0, delims);
			char * close = strtok(0, delims);
			char * volume = strtok(0, delims);
			char * closeAdj = strtok(0, delims);

			if (d > dateGuide[index])
				continue;

			while (d < dateGuide[index] && index > 0)
				index--;

			if (index >= 0)
			{
				double openF = ::atof(open);
				double closeF = ::atof(close);

				double logReturn = log(closeF / openF);

				stockData[index--] = Daily(logReturn);

				if (logReturn != 0.0)
					loadCount++;
			}
		}
	}
	else
	{
		int index = 0;

		//while (p && ((int)(p - &buf[0]) < (int)bytes) && index < NUM_DATES)
		while (p && index < NUM_DATES)
		{
			// p at stock, eg "BHP" or at date for US
			if (stockType == ASX)
				p = strtok(0, delims);

			if (!p)
				break;

			// p at "19860421" or "01/03/1968" for US files
			DateLong d;
			if (stockType == ASX)
				d = ::atol(p);
			else
			{
				int year = ::atol(p+6);
				int month = ::atol(p);
				int day = ::atol(p+3);
				d = (year * 100 + month)*100 + day;
			}

			if (d < 19000000 || d > 20090000)
				break;

			while (dateGuide[index] < d)
				index++;

			p = strtok(0, delims);
			// p at open, eg 123.456
			double open = ::atof(p);

			p = strtok(0, delims);
			// p at high, eg 123.456
			p = strtok(0, delims);
			// p at low, eg 123.456
			p = strtok(0, delims);
			// p at close, eg 123.456
			double close = ::atof(p);

			if (*(p + strlen(p) + 1) == '\n')
			{
				// no volume. Just skip ahead to next line
				p = strtok(0, delims);
			}
			else
			{
				p = strtok(0, delims);
				// p at volume, eg 123.456
				p = strtok(0, delims);
			}

			if (dateGuide[index] > d)
				continue;

			double logReturn = log(close / open);

			// typical logReturns are from -0.1 to 0.1, equiv to -10% to +10%
			// scale around this to a value from 1..255

			// normalize logReturn to -127 .. 127
			//logReturn *= (127/maxLogReturn);
			//logReturn = max(logReturn, -127.0);
			//logReturn = min(logReturn,  127.0);

			stockData[index++] = Daily(logReturn);

			if (logReturn != 0.0)
				loadCount++;
		}
	}
}

void saveStock(Stock& stockData, const char* stockName)
{
	FILE* fp = fopen(stockName, "wb");

	fwrite(&stockData[0], NUM_DATES, sizeof(Daily), fp);

	fclose(fp);
}

int numMatched;

struct CacheData
{
	float sumx2;
	float sumy2;
	float sumxy;
	int N;
	int cutoff;

	CacheData() : sumx2(0.0), sumy2(0.0), sumxy(0.0), N(0), cutoff(0) {};
};

typedef pair<Stock*, Stock*> CacheKey;

typedef map<CacheKey, CacheData> Cache;

Cache cache;

void calcCorrelation(Stock& s1, Stock& s2, float& r, float& a, float& b, int cutoff, bool useCache)
{
	float sumx = 0.0;
	float sumy = 0.0;
	float sumx2 = 0.0;
	float sumy2 = 0.0;
	float sumxy = 0.0;
	int N = 0;
	int start = 0;
	int i;

	CacheKey cacheKey = make_pair(&s1, &s2);

	if (useCache)
	{
		CacheData d = cache[cacheKey];

		start = d.cutoff;
		sumx2 = d.sumx2;
		sumy2 = d.sumy2;
		sumxy = d.sumxy;
		N = d.N;
	}

	for (i=start; i< cutoff; i++)
	{
		if (s1[i] != undefined && s2[i] != undefined)
		{
			sumx += s1[i];
			sumy += s2[i];
			N++;
		}
	}

	float avgx = sumx/N;
	float avgy = sumy/N;
	N = 0;

	if (useCache)
	{
		CacheData d = cache[cacheKey];

		N = d.N;
	}

	for (i=start; i< cutoff; i++)
	{
		if (s1[i] != undefined && s2[i] != undefined)
		{
			float x = s1[i] - avgx;
			float y = s2[i] - avgy;

			sumx2 += x * x;
			sumy2 += y * y;
			sumxy += x * y;
			N++;
		}
	}

	if (useCache)
	{
		CacheData d;
		d.cutoff = cutoff;
		d.sumx2 = sumx2;
		d.sumy2 = sumy2;
		d.sumxy = sumxy;
		d.N = N;

		cache[cacheKey] = d;
	}

	float stddevx = (float)sqrt(sumx2 / (N-1));
	float stddevy = (float)sqrt(sumy2 / (N-1));

	r = sumxy / (stddevx * stddevy) / (N-1);
	numMatched = N;

	//r = r * (float)N/NUM_DATES;

	// "y = a + bx"

	b = r * stddevy / stddevx;

	a = avgy - b * avgx;

}

void convertFiles(StockType stockType)
{
	//string root = "F:\\";
	string root = PATH_US_Text;
	string outPath;
	
	if (stockType == ASX)
	{
		outPath = PATH_ASX_Data;
	}
	else
	{
		outPath = PATH_US_Data;
	}

//	DIR *dir = opendir(root.c_str());
//
//	struct dirent* dire = 0;
//	
//	while (dire = readdir(dir))
//	{
//		cout << "dire->d_name = " << dire->d_name << endl;
//
//		string path2 = root + dire->d_name;
		string path2 = root;
//
//		if (strcmp(dire->d_name, "Data")==0)
//			continue;

		DIR *dir2 = opendir(path2.c_str());

		struct dirent* dire2 = 0;
	
		while (dire2 = readdir(dir2))
		{
			if (dire2->d_name[0] != '.')
			{
				string inFile = path2;
				inFile += "\\";
				inFile += dire2->d_name;

				string outFile = outPath;
				outFile += "\\";

				char* ext = strstr(dire2->d_name, ".ASC");
				if (!ext)
					ext = strstr(dire2->d_name, ".CSV");
				if (!ext)
					ext = strstr(dire2->d_name, ".csv");
				if (ext)
					strcpy(ext, ".DAT");

				outFile += dire2->d_name;

				cout << inFile << " -> " << outFile << endl;

				Stock s;
				// (dire2->d_name[0] > 'S')
				{
					loadStock(stockType, inFile.c_str(), s);

					if (loadCount > NUM_DATES/2)
					{
						saveStock(s, outFile.c_str());
						cout << "loadCount=" << loadCount << endl;
					}
				}
			}
		}
//	}
}

void loadStocks(StockType stockType, int num, Stock* dest, string* names)
{
	string root;

	if (stockType == ASX)
	{
		root = PATH_ASX_Data;
	}
	else
	{
		root = PATH_US_Data;
	}

	DIR *dir = opendir(root.c_str());

	struct dirent* dire = 0;

	int i = 0;

	while (dire = readdir(dir))
	{
		if (dire->d_name[0] != '.')
		{
			cout << ".";

			names[i] = dire->d_name;
			string stockName = root;
			stockName += "\\";
			stockName += dire->d_name;

			loadDataStock(stockName.c_str(), dest[i]);

			i++;
		}
	}

	cout << endl;
}

void Stock_SetUndefined(Stock& stock)
{
	for (int i=0; i<NUM_DATES; i++)
		stock[i] = undefined;
}

void Stock_SetZero(Stock& stock)
{
	for (int i=0; i<NUM_DATES; i++)
		stock[i] = 0.0;
}

void Stock_Accumulate(Stock& dest, float a, float b, Stock& src)
{
	for (int i=0; i<NUM_DATES; i++)
	{
		if (src[i] == undefined)
		{
			; // ignore undefined src
		}
		else
		{
			if (dest[i] == undefined)
				dest[i] = 0.0;

			dest[i] += a + b * src[i];
		}
	}
}

void Stock_ToMovingAverage(Stock& dest, Stock&src, int days)
{
	for (int i=0; i<NUM_DATES; i++)
	{
		dest[i] = undefined;

		if (i >= days-1)
		{
			float x = 0;
			int N = 0;
			for (int w=0; w<days; w++)
			{
				if (src[i-w] == undefined)
				{
					;
				}
				else
				{
					if (w == 0)
					{
						x += src[i-w]*4;
						N += 4;
					}
					else if (w == 1)
					{
						x += src[i-w]*2;
						N += 2;
					}
					else
					{
						x += src[i-w];
						N += 1;
					}
				}
			}

			if (N > 0)
			{
				dest[i] = x/N;
			}
		}
	}
}

bool shown = false;

void calcModels(Stock* usStock, Stock* asxStock, Stock* asxModel, int cutoff, ostream& out)
{
	const int numLevels = 20;
	const float topR = (float)1.0;
	int histogram[numLevels]; // 0.00-0.05, 0.05-0.10, etc
	for (int i=0; i<numLevels; i++)
		histogram[i] = 0;
	float step = (float)topR / numLevels;

	for (int j=0; j<numASXStocks; j++)
	{
		Stock_SetZero(asxModel[j]);

		for (int i=0; i<numUSStocks; i++)
		{
			float r, a, b;

			calcCorrelation(usStock[i], asxStock[j], r, a, b, cutoff, true);

			/*
			for (int hl=0; hl<numLevels; hl++)
			{
				float lower = hl*step;
				float upper = (hl+1)*step;

				if (lower <= r && r < upper)
					histogram[hl] ++;
			}
			*/

			//if (fabs(r) > 0.1 && numMatched > 100)
			{
				//cout << usName << " <-> " << asxNames[i] << " = " << r << " (numMatched=" << numMatched << ")" << endl;
				//out << usName << " <-> " << asxNames[i] << " = " << r << " (numMatched=" << numMatched << ")" << endl;
				//char outChar = '0' + (char)(fabs(r)*20);
				//out << outChar;
			}

			//a *= (r*r);
			b *= (r*r);

			//if (fabs(r) > 0.01)
			{
				Stock_Accumulate(asxModel[j], 0.0, b, usStock[i]);
			}
		}

		if (!shown)
		{
			out << "   Model-Stock correlation: ";
			cout << "   Model-Stock correlation: ";
			float r, a, b;
			calcCorrelation(asxModel[j], asxStock[j], r, a, b, cutoff, false);
			out << "r=" << r << ", a=" << a << ", b = " << b << endl;
			cout << "r=" << r << ", a=" << a << ", b = " << b << endl;

			for (int hl=0; hl<numLevels; hl++)
			{
				float lower = hl*step;
				float upper = (hl+1)*step;

				if (lower <= r && r < upper)
					histogram[hl] ++;
			}
		}
	}

	if (!shown)
	{
		for (int l=0; l<numLevels; l++)
		{
			float lower = l*step;
			float upper = (l+1)*step;

			char buf[20];
			sprintf(buf, "%5.2f...%5.2f: ", lower, upper);
			out << buf;
			cout << buf;
			for (int x=0; x<histogram[l]; x+=1)
			{
				out << "*";
				cout << "*";
			}
			out << endl;
			cout << endl;
		}
		shown = true;
	}
}

float doStrategy(int strat, Stock* asxStock, Stock* asxModel, int day)
{
	float result = 0.0;

	if (strat == 1)
	{
		int numDefined = 0;
		for (int s=0; s<numASXStocks; s++)
		{
			if (asxStock[s][day] != undefined)
			{
				result += asxStock[s][day];
				numDefined ++;
			}
		}
		result = numDefined ? (float)(result / numDefined) : (float)0.0;
	}
	else
	{
		float highestPrediction = 0.0;
		int highestChoice;

		float lowestPrediction = 0.0;
		int lowestChoice;

		for (int s=0; s<numASXStocks; s++)
		{
			if (asxStock[s][day] != undefined)
			{
				float prediction = asxModel[s][day];

				if (prediction > highestPrediction)
				{
					highestPrediction = prediction;
					highestChoice = s;
				}

				if (prediction < lowestPrediction)
				{
					lowestPrediction = prediction;
					lowestChoice = s;
				}
			}
		}

		if (strat == 0) // worst
		{
			if (lowestPrediction < 0.0)
				result = asxStock[lowestChoice][day];
		}
		else if (strat == 2) // best
		{
			if (highestPrediction > 0.0)
				result = asxStock[highestChoice][day];
		}
	}

	return result;
}

int main()
{
	generateDateGuides();

//	convertFiles(US);

	Stock* asxStock = new Stock[numASXStocks];
	Stock* asxStock5d = new Stock[numASXStocks];
	string* asxNames = new string[numASXStocks];

	Stock* asxModel = new Stock[numASXStocks];
	Stock* asxModel5d = new Stock[numASXStocks];

	loadStocks(ASX, numASXStocks, asxStock, asxNames);

	Stock* usStock = new Stock[numUSStocks];
	Stock* usStock5d = new Stock[numUSStocks];
	string* usNames = new string[numUSStocks];

	loadStocks(US, numUSStocks, usStock, usNames);

	shown = false;
	int WINDOW = 3;

	int s;
	for (s = 0; s<numASXStocks; s++)
		Stock_ToMovingAverage(asxStock5d[s], asxStock[s], WINDOW);
	for (s = 0; s<numUSStocks; s++)
		Stock_ToMovingAverage(usStock5d[s], usStock[s], WINDOW);

	ofstream out("pisa.out");

	const float initial = 10000.0;

	float cumResults[3] = { initial, initial, initial };

	int startDay = (int)(NUM_DATES*0.6);
	int endDay = (int)(NUM_DATES*0.99);

	for (int day=startDay; day<endDay; day++)
	{
		char buf[10];
		sprintf(buf, "%04d", day);
		cout << buf << ": ";
		out << buf << ": ";

		calcModels(usStock5d, asxStock5d, asxModel5d, day, out);

		float time = (float)(day - startDay); // days, 5 days per week
		time = time * 7 / 5; // days, 7 days per week
		time = time / (float)365.25; // years

		for (int strat = 0; strat <3; strat++)
		{
			float logReturn = doStrategy(strat, asxStock, asxModel5d, day);

			float dayFactor = (float)exp(logReturn);

			cumResults[strat] *= dayFactor;

			char buf[50];

			float overallFactor = cumResults[strat] / initial;

			float annualizedFactor = (float)pow(overallFactor, 1.0/time);

			float annualizedPercent = (float)(annualizedFactor-1.0)*100;

			sprintf(buf, "$%8.2f (%+4.2f%%) ", cumResults[strat], annualizedPercent);
			cout << buf << " ";
			out << buf << " ";
		}

		cout << endl;
		out << endl;
	}

	cout << endl;

	return 0;
}