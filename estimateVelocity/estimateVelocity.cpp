#include <iostream>
#include <vector>
#include <utility>
#include <fstream>
#include <sstream>
#include "TGraph.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TStyle.h"
#include "TGraphErrors.h"

std::vector< std::pair<int,double> > takeData(const std::string file, const char thresholdLabel);
std::pair<double,double> plotVelocity(std::vector<double> positions, std::vector<double> means, int strip, std::vector<double> errors);

struct positionData{
	double positionValue;
	double deltaT;
	double deltaTError;
};

struct thresholdData{
	char thresholdLabel;
	std::vector<positionData> positions;
};

struct scintData{
	int ID;
	std::vector<thresholdData> thrData;
};

std::vector<scintData> takeData(const std::string file);
void velocityCalc(std::vector<scintData>& data,const char thrLabel);
void parserTest(std::vector<scintData>& data, const char thrLabel);

int main(int argc, char** argv)
{
	gStyle->SetOptFit(1);
	
	std::vector<scintData> data;
	if(argc == 1)
	  data = takeData("results.txt");
	else if (argc == 2)
	  data = takeData(std::string(argv[1]));
	
	if(data.size() == 0 )
	{
	  std::cout<<"Error while opening input file"<<std::endl;
	  return 1;
	}
	
	std::cout << data.size() << std::endl;
	
	//~ parserTest(data, 'a');
	velocityCalc( data , 'a');

	return 0;
}

void parserTest(std::vector<scintData>& data, const char thrLabel)
{
	for(auto scintillator : data)
	{	
		std::cout << scintillator.ID;
		for(auto threshold : scintillator.thrData)
		{
			if( thrLabel == threshold.thresholdLabel )
			{
				std::cout << "\t" << threshold.thresholdLabel;
				for( auto position : threshold.positions )
				{
					std::cout << "\t" << position.positionValue << "\t" << position.deltaT << "\t" << position.deltaTError <<std::endl;
				}
			}
		}
	}

}


void velocityCalc(std::vector<scintData>& data, const char thrLabel)
{

	std::ofstream results;
	std::string title = "resultsForThreshold";
	title+= thrLabel;
	title+= ".txt";
	results.open( title.c_str() );
	//std::vector<double> positionsForFit;
	//std::vector<double> timesForFit;		
	for(auto scintillator : data)
	{	
		std::vector<double> positionsForFit;
		std::vector<double> timesForFit;	
		std::vector<double> errorsForFit;
		for(auto threshold : scintillator.thrData)
		{
			
			if( thrLabel == threshold.thresholdLabel )
			{			
				for( auto position : threshold.positions )
				{
					std::cout << "\t" << position.positionValue << "\t" << position.deltaT << "\t" << position.deltaTError <<std::endl;
					positionsForFit.push_back( position.positionValue );
					timesForFit.push_back( position.deltaT );
					errorsForFit.push_back( position.deltaTError);
					std::cout << positionsForFit.size() << std::endl;
					std::cout << timesForFit.size() << std::endl;
				}
			}
		}
		std::pair<double,double> velocity = plotVelocity( positionsForFit, timesForFit, scintillator.ID, errorsForFit);
		results << scintillator.ID << "\t" << velocity.first << "\t" << velocity.second << std::endl;
	}

	results.close();
}

std::pair<double,double> plotVelocity(std::vector<double> positions, std::vector<double> means, int strip, std::vector<double> errors)
{
	TCanvas* c = new TCanvas();
	//TGraph* vGraph = new TGraph(positions.size(), &means[0], &positions[0]);
	TGraphErrors* vGraph = new TGraphErrors(positions.size(), &means[0], &positions[0], 0, &errors[0] );
	vGraph->SetMarkerStyle(20);
	vGraph->SetMarkerSize(1);
	vGraph->Draw("AP");
	vGraph->Fit("pol1");
	TF1* fit = vGraph->GetFunction("pol1");
	std::stringstream buf;
	buf << strip;
	c->SaveAs( (buf.str() + ".png").c_str() );
	std::cout << fit->GetParameter(1);
	std::pair<double,double> resultOfFit = std::make_pair<double,double> ( (fit->GetParameter(1))*-0.2,  (fit->GetParError(1))*-0.2 );
	return resultOfFit;
}

bool ifScintillatorIsIn( std::vector<scintData>& data, int ID)
{
	for( auto scintillator : data )
	{
		if( ID == scintillator.ID )
			return true;
	}
	return false;
}

int findScintillatorIn( std::vector<scintData>& data, int ID)
{
	for( unsigned int i = 0; i < data.size(); i++ )
	{
		if( ID == data[i].ID )
			return i;
	}
	return -1;
}

std::vector<scintData> takeData(const std::string file){
	std::ifstream dataFile;
	dataFile.open(file.c_str());

	if( !dataFile.good() )
	{
		std::exit(1);
	}

	std::vector< scintData > allData;
	
	double position = 999;
	char thrLabel = 'z';
	double chi2 = 99;
	int degrees = 0;
	int ID = 0;
	double mean = 999;
	double meanError = 0;
		while( dataFile >> ID >> position >> thrLabel >>  mean >> meanError >> chi2 >> degrees )
		{	
			//IF ID IS IN ALLDATA
			if ( ifScintillatorIsIn(allData, ID) )
			{
				//FILL THIS SCINTILLATOR WITH ADDITIONAL INFO
				int scintIndex = findScintillatorIn(allData, ID);
				
				positionData pos;
				pos.positionValue = position;
				pos.deltaT = mean;
				pos.deltaTError = meanError;
				
				thresholdData thr;
				thr.thresholdLabel = thrLabel;
				thr.positions.push_back(pos);
				
				allData[ scintIndex ].thrData.push_back(thr);
			}
			else{
				//ELSE MAKE NEW SCINTILLATOR AND FILL
				positionData pos;
				pos.positionValue = position;
				pos.deltaT = mean;
				pos.deltaTError = meanError;
				
				thresholdData thr;
				thr.thresholdLabel = thrLabel;
				thr.positions.push_back(pos);
				
				scintData scintillator;
				scintillator.ID = ID;
				scintillator.thrData.push_back(thr);
				
				// FILL THRESHOLD AND POSITION
				allData.push_back(scintillator);
			}
		}
	
	return allData;	
}

//~ 
//~ std::vector< std::pair<int,double> > takeData(const std::string file, const char thresholdLabel){
	//~ std::ifstream dataFile;
	//~ dataFile.open(file.c_str());
//~ 
	//~ std::vector< std::pair<int, double> > allData;
	//~ std::vector< std::pair<int, double> > thresholdA, thresholdB, thresholdC, thresholdD;
	//~ int ID = 0;
	//~ double mean = 0;
		//~ while( dataFile >> ID >> mean )
		//~ {
			//~ allData.push_back(std::make_pair<int,double> (ID, mean) );			
		//~ }
//~ 
	//~ for(unsigned i = 0; i < allData.size(); i++)
	//~ {
		//~ switch(i%4){
			//~ case 0:
				//~ thresholdA.push_back(allData[i]);
				//~ break;
			//~ case 1:
				//~ thresholdB.push_back(allData[i]);
				//~ break;
			//~ case 2:
				//~ thresholdC.push_back(allData[i]);
				//~ break;
			//~ case 3:
				//~ thresholdD.push_back(allData[i]);
				//~ break;
		//~ }
	//~ }
//~ 
	//~ dataFile.close();
	//~ 
	//~ switch(thresholdLabel){
		//~ case 'a':
			//~ return thresholdA;
		//~ case 'b':
			//~ return thresholdB;
		//~ case 'c':
			//~ return thresholdC;
		//~ case 'd':
			//~ return thresholdD;
	//~ }
	//~ return allData;	
//~ }
