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
#include <map>
#include <algorithm>

std::vector< std::pair<int,double> > takeData(const std::string file, const char thresholdLabel);
std::pair<double,double> plotVelocity(std::vector<double> positions, std::vector<double> means, int strip, std::vector<double> errors, char thresholdLabel);



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
void velocityCalc(std::vector<scintData>& data, const std::string& outPath);
void parserTest(std::vector<scintData>& data, const char thrLabel);

int main(int argc, char** argv)
{
	gStyle->SetOptFit(1);
	
	std::vector<scintData> data;
	std::string outPath = "";
	if(argc == 1)
	{
	  data = takeData("results.txt");
	}
	else if (argc == 2)
	{
	  data = takeData(std::string(argv[1]));
	}
	else if (argc == 3)
	{
	  data = takeData(std::string(argv[1]));
	  outPath = argv[2];
	}
	
	if(data.size() == 0 )
	{
	  std::cout<<"Error while opening input file"<<std::endl;
	  return 1;
	}
	
	velocityCalc( data , outPath);
	
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


void velocityCalc(std::vector<scintData>& data, const std::string& outPath)
{
	std::map<char, int> thresholdLabelToInt = {{'a', 1}, {'b',2}, {'c',3}, {'d',4}};
	std::ofstream results;
	std::string title = outPath+"EffVelocities";
	title+= ".txt";
	results.open( title.c_str() );
	for(const auto& scintillator : data)
	{	
		for(const auto& threshold : scintillator.thrData)
		{
			std::vector<double> positionsForFit;
			std::vector<double> timesForFit;	
			std::vector<double> errorsForFit;
			for(const auto& position : threshold.positions )
			{
				positionsForFit.push_back( position.positionValue );
				timesForFit.push_back( position.deltaT );
				errorsForFit.push_back( position.deltaTError);
			}
			std::pair<double,double> velocity = plotVelocity( positionsForFit, timesForFit, scintillator.ID, errorsForFit, threshold.thresholdLabel);
			int layer = 0, layerReset = 0;
			if( scintillator.ID < 49 )
			{
			  layer = 1;
			}
			else if( scintillator.ID > 48 && scintillator.ID < 97 )
			{
			  layer = 2; 
			  layerReset = 48;
			}
			else 
			{
			  layer = 3;
			  layerReset = 96;
			}
			
			results << layer << "\t" << scintillator.ID - layerReset << "\tA\t" << thresholdLabelToInt[threshold.thresholdLabel];
			results << "\t" << velocity.first << "\t" << velocity.second;
			results << "\t0\t0\t0\t0\t0\t0" << std::endl;
			results << layer << "\t" << scintillator.ID - layerReset << "\tB\t" << thresholdLabelToInt[threshold.thresholdLabel];
			results << "\t" << velocity.first << "\t" << velocity.second;
			results << "\t0\t0\t0\t0\t0\t0" << std::endl;
		}
	}

	results.close();
}

std::pair<double,double> plotVelocity(std::vector<double> positions, std::vector<double> means, int strip, std::vector<double> errors, char thresholdLabel)
{
	TCanvas* c = new TCanvas();
	TGraphErrors* vGraph = new TGraphErrors(positions.size(), &means[0], &positions[0], 0, &errors[0] );
	vGraph->SetMarkerStyle(20);
	vGraph->SetMarkerSize(1);
	vGraph->Draw("AP");
	vGraph->Fit("pol1");
	TF1* fit = vGraph->GetFunction("pol1");
	std::stringstream buf;
	buf << strip << "_" << thresholdLabel;
	c->SaveAs( (buf.str() + ".png").c_str() );
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

bool ifThresholdIsIn( scintData& data, char label)
{
	return std::any_of(data.thrData.begin(), data.thrData.end(), [label]( const thresholdData& thr) {return label == thr.thresholdLabel; });
}

int findThresholdIn( scintData& data, char label)
{
	for( unsigned int i = 0; i < data.thrData.size(); i++ )
	{
		if( label == data.thrData[i].thresholdLabel )
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
				
				//IF THR IS IN SCINTILLATOR
				if( ifThresholdIsIn( allData[scintIndex], thrLabel ) )
				{
				  int thrIndex = findThresholdIn( allData[scintIndex], thrLabel );
				  allData[ scintIndex ].thrData[thrIndex].positions.push_back(pos);
				}
				else
				{
				//ELSE MAKE A NEW THRESHOLD AND FILL
				thresholdData thr;
				thr.thresholdLabel = thrLabel;
				thr.positions.push_back(pos);
				
				allData[ scintIndex ].thrData.push_back(thr);
				}
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

