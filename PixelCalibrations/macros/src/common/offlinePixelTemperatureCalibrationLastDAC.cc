
#include <iostream>
#include <iomanip>
#include <fstream>
#include <list>
#include <map>
#include <string>

#include <TFile.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TH1.h>
#include <TLegend.h>

#include "PixelCalibrations/macros/include/PixelReadOutChipData_dcsInfo.h"
#include "PixelCalibrations/macros/include/PixelReadOutChipData_temperatureInfo.h"
#include "PixelCalibrations/macros/include/PixelReadOutChipData_powerInfo.h"
#include "PixelCalibrations/macros/include/PixelReadOutChipData_fixedPower.h"
#include "PixelCalibrations/macros/include/PixelReadOutChipData_varyingPower.h"

//--- declare data-types specific to last DAC temperature calibration
typedef std::list<std::pair<std::map<unsigned int, std::list<unsigned int> >, PixelReadOutChipData_dcsInfo> > rawDataType_oneReadOutChip;
typedef std::map<std::string, rawDataType_oneReadOutChip> rawDataType_fixedPower;

typedef std::list<PixelReadOutChipData_dcsInfo> dcsConditionsDataSetType;

//--- declare auxiliary functions
void readRawData_XML(const char* inputFileName,
		     rawDataType_fixedPower& rawDataSet_reducedPower,
		     rawDataType_fixedPower& rawDataSet_nominalPower,
		     dcsConditionsDataSetType& dcsConditionsDataSet);

void writeCalibrationConstants_XML(const char* outputFileName,
				   const std::list<std::pair<std::string, PixelReadOutChipData_varyingPower> >& calibratedReadOutChips);

TDirectory* changeDirectory(TFile* outputFile, const std::string& directoryName);

void fillTemperatureCycleGraphs(TGraph* graph_CoolingCycles, TGraph* graph_HeatingCycles,
				const dcsConditionsDataSetType& dcsConditionsDataSet);
void drawTemperatureCycleGraphs(TGraph* graph_CoolingCycles, TGraph* graph_HeatingCycles);

//
//---------------------------------------------------------------------------------------------------
//

int main(int argc, char* argv[])
{
  std::cout << "<offlinePixelTemperatureCalibrationLastDAC>:" << std::endl;
  
  rawDataType_fixedPower rawDataSet_reducedPower;
  rawDataType_fixedPower rawDataSet_nominalPower;

  dcsConditionsDataSetType dcsConditionsDataSet;

  const char* rawDataFileName = "/home/pvss/TriDAS/pixel/PixelCalibrations/data/pixelTemperatureCalibrationData.xml";
  std::cout << "--> reading raw data file = " << rawDataFileName << std::endl;
  readRawData_XML(rawDataFileName, rawDataSet_reducedPower, rawDataSet_nominalPower, dcsConditionsDataSet);

  TFile* outputFile = new TFile("pixelTemperatureCalibrationLastDAC.root", "RECREATE");

//--- draw graphs showing deviation between actual and nominal temperatures
//    for cooling/heating cycles
  changeDirectory(outputFile, "TemperatureCycling");
  TCanvas* temperatureCycleCanvas = new TCanvas("temperatureCycleCanvas", "Variation of Cooling-Box Temperature", 800, 600);
  temperatureCycleCanvas->SetFillColor(10);
  temperatureCycleCanvas->SetBorderSize(2);
  temperatureCycleCanvas->cd();
  TGraph* graph_CoolingCycles = new TGraph();
  graph_CoolingCycles->SetName("graph_CoolingCycles");
  TGraph* graph_HeatingCycles = new TGraph();
  graph_HeatingCycles->SetName("graph_HeatingCycles");
  fillTemperatureCycleGraphs(graph_CoolingCycles, graph_HeatingCycles, dcsConditionsDataSet);
  drawTemperatureCycleGraphs(graph_CoolingCycles, graph_HeatingCycles);
  temperatureCycleCanvas->Write();

  std::list<std::pair<std::string, PixelReadOutChipData_varyingPower> > calibratedReadOutChips;

//--- compute calibration constants;
//    draw graphs showing control output
  for ( rawDataType_fixedPower::iterator rawDataEntry_nominalPower = rawDataSet_nominalPower.begin();
	rawDataEntry_nominalPower != rawDataSet_nominalPower.end(); ++rawDataEntry_nominalPower ) {
    const std::string& readOutChip_name = rawDataEntry_nominalPower->first;

    std::cout << "computing Calibration for Read-out Chip = " << readOutChip_name << std::endl;

    rawDataType_oneReadOutChip& rawData_nominalPower = rawDataEntry_nominalPower->second;
    rawDataType_oneReadOutChip& rawData_reducedPower = rawDataSet_reducedPower[readOutChip_name];

    std::string calibration_nominalPowerName = readOutChip_name + "_nominalPower";
    PixelReadOutChipData_fixedPower calibration_nominalPower(calibration_nominalPowerName, rawData_nominalPower);
    calibration_nominalPower.Calibrate();
    changeDirectory(outputFile, readOutChip_name + "/nominalPower");
    calibration_nominalPower.Write();

    std::string calibration_reducedPowerName = readOutChip_name + "_reducedPower";
    PixelReadOutChipData_fixedPower calibration_reducedPower(calibration_reducedPowerName, rawData_reducedPower);
    calibration_reducedPower.Calibrate();
    changeDirectory(outputFile, readOutChip_name + "/reducedPower");
    calibration_reducedPower.Write();

    std::string calibration_varyingPowerName = readOutChip_name + "_varyingPower";
    PixelReadOutChipData_varyingPower calibration_varyingPower(calibration_varyingPowerName, calibration_nominalPower, calibration_reducedPower);
    calibration_varyingPower.Calibrate();
    std::cout << " estimated Temperature difference between Read-out Chip and Cooling Reservoir = " << calibration_varyingPower.getTemperatureDifference() << std::endl;
    changeDirectory(outputFile, readOutChip_name + "/varyingPower");
    calibration_varyingPower.Write();

    calibratedReadOutChips.push_back(std::pair<std::string, PixelReadOutChipData_varyingPower>(readOutChip_name, calibration_varyingPower));
  }

  delete outputFile;

//--- write calibration constants into XML file
  const char* calibrationConstantsFileName = "pixelTemperatureCalibrationConstantsLastDAC.xml";
  writeCalibrationConstants_XML(calibrationConstantsFileName, calibratedReadOutChips);
}

//
//---------------------------------------------------------------------------------------------------
//

void readRawData_XML(const char* inputFileName,
		     rawDataType_fixedPower& rawDataSet_reducedPower,
		     rawDataType_fixedPower& rawDataSet_nominalPower,
		     dcsConditionsDataSetType& dcsConditionsDataSet)
{
//--- open input file
  ifstream* inputFile = new ifstream(inputFileName);

  std::string lineBuffer;

//--- skip header information
  while ( lineBuffer.find("<calibrationData>") == std::string::npos ) {
    //std::cout << "--> skipping line = " << lineBuffer << std::endl;
    getline(*inputFile, lineBuffer);
  }

  std::string currentDataType;
  std::string currentTemperatureCycleType;
  std::string lastTemperatureCycleType;
  double currentNominalTemperature;
  double lastNominalTemperature;
  unsigned int currentTempRange;

  std::string currentReadOutChip_name;
  enum powerMode { kNominalPower, kReducedPower };
  int currentReadOutChip_powerMode;

  double currentActualTemperature;
  double currentD1_analogLV_vMon;
  double currentD1_analogLV_iMon;
  double currentD1_digitalLV_vMon;
  double currentD1_digitalLV_iMon;
  double currentD2_analogLV_vMon;
  double currentD2_analogLV_iMon;
  double currentD2_digitalLV_vMon;
  double currentD2_digitalLV_iMon;

  typedef std::map<unsigned int, std::list<unsigned int> > adcValueType;
  std::map<std::string, adcValueType> adcValueMap;

  while ( lineBuffer.find("</calibrationData>") == std::string::npos  ) {
    
    //std::cout << "--> reading line = " << lineBuffer << std::endl;

    if ( lineBuffer.find("<dataSet") != std::string::npos ) {
      int index_beginTag = lineBuffer.find('<');
      int index_firstSeparator = lineBuffer.find(' ', index_beginTag + 1);
      int index_secondSeparator = lineBuffer.find(' ', index_firstSeparator + 1);
      int index_thirdSeparator = lineBuffer.find(' ', index_secondSeparator + 1);
      int index_fourthSeparator = lineBuffer.find(' ', index_thirdSeparator + 1);
      //int index_endTag = lineBuffer.find('>');
      
      //std::cout << "index_firstSeparator = " << index_firstSeparator << std::endl;
      //std::cout << "index_secondSeparator = " << index_secondSeparator << std::endl;
      //std::cout << "index_thirdSeparator = " << index_thirdSeparator << std::endl;

      int index_beginDataType = lineBuffer.find('\"', index_firstSeparator);
      //std::cout << "index_beginDataType = " << index_beginDataType << std::endl;
      int index_endDataType = lineBuffer.find('\"', index_beginDataType + 1);
      std::string dataType(lineBuffer, index_beginDataType + 1, index_endDataType - (index_beginDataType + 1));

      int index_beginTemperatureCycleType = lineBuffer.find('\"', index_secondSeparator);
      //std::cout << "index_beginTemperatureCycleType = " << index_beginTemperatureCycleType << std::endl;
      int index_endTemperatureCycleType = lineBuffer.find('\"', index_beginTemperatureCycleType + 1);
      std::string temperatureCycleType(lineBuffer, index_beginTemperatureCycleType + 1, index_endTemperatureCycleType - (index_beginTemperatureCycleType + 1));

      int index_beginNominalTemperature = lineBuffer.find('\"', index_thirdSeparator);
      //std::cout << "index_beginNominalTemperature = " << index_beginNominalTemperature << std::endl;
      int index_endNominalTemperature = lineBuffer.find('\"', index_beginNominalTemperature + 1);
      std::string nominalTemperature_string(lineBuffer, index_beginNominalTemperature + 1, index_endNominalTemperature - (index_beginNominalTemperature + 1));
      //std::cout << "nominalTemperature_string = " << nominalTemperature_string << std::endl;
      double nominalTemperature = atof(nominalTemperature_string.data());

      int index_beginTempRange = lineBuffer.find('\"', index_fourthSeparator);
      int index_endTempRange = lineBuffer.find('\"', index_beginTempRange + 1);
      std::string TempRange_string(lineBuffer, index_beginTempRange + 1, index_endTempRange - (index_beginTempRange + 1));
      unsigned int TempRange = atoi(TempRange_string.data());

      //std::cout << "dataType = " << dataType << ","
      //	  << " temperatureCycleType = " << temperatureCycleType << ","
      //	  << " nominalTemperature = " << nominalTemperature << ","
      //	  << " TempRange = " << TempRange << std::endl;

      currentDataType = dataType;
      currentTemperatureCycleType = temperatureCycleType;
      currentNominalTemperature = nominalTemperature;
      currentTempRange = TempRange;

      if ( currentNominalTemperature != lastNominalTemperature ||
	   currentTemperatureCycleType != lastTemperatureCycleType) {
	std::cout << "--> processing data for nominal Temperature = " << nominalTemperature << ","
		  << " temperature Cycle type = " << temperatureCycleType << std::endl;
	lastNominalTemperature = currentNominalTemperature;
	lastTemperatureCycleType = currentTemperatureCycleType;
      }
    }

    if ( lineBuffer.find("<FPix_") != std::string::npos ) {
      int index_beginTag = lineBuffer.find('<');
      int index_firstSeparator = lineBuffer.find(' ', index_beginTag + 1);
      int index_secondSeparator = lineBuffer.find(' ', index_firstSeparator + 1);
      //int index_endTag = lineBuffer.find('>');

      //std::cout << "index_beginTag = " << index_beginTag << std::endl;
      //std::cout << "index_firstSeparator = " << index_firstSeparator << std::endl;

      std::string readOutChipName(lineBuffer, index_beginTag + 1, index_firstSeparator - (index_beginTag + 1));

      int index_beginVana = lineBuffer.find('\"', index_firstSeparator);
      int index_endVana = lineBuffer.find('\"', index_beginVana + 1);
      std::string Vana_string(lineBuffer, index_beginVana + 1, index_endVana - (index_beginVana + 1));
      unsigned Vana = atoi(Vana_string.data());

      int index_beginVsf = lineBuffer.find('\"', index_secondSeparator);
      int index_endVsf = lineBuffer.find('\"', index_beginVsf + 1);
      std::string Vsf_string(lineBuffer, index_beginVsf + 1, index_endVsf - (index_beginVsf + 1));
      unsigned Vsf = atoi(Vsf_string.data());

      //std::cout << "readOutChipName = " << readOutChipName << ","
      //	  << " Vana = " << Vana << ","
      //	  << " Vsf = " << Vsf << std::endl;

      currentReadOutChip_name = readOutChipName;
      if ( Vana == 0 && Vsf == 0 ) {
	currentReadOutChip_powerMode = kReducedPower;
      } else {
	currentReadOutChip_powerMode = kNominalPower;
      }
    }

    if ( lineBuffer.find("<adcCount") != std::string::npos ) {
      int index_beginOpeningTag = lineBuffer.find('<');
      int index_endOpeningTag = lineBuffer.find('>', index_beginOpeningTag + 1);
      int index_beginClosingTag = lineBuffer.find("</", index_endOpeningTag + 1);
      //int index_endClosingTag = lineBuffer.find('>', index_beginClosingTag + 1);

      std::string adcValue_string(lineBuffer, index_endOpeningTag + 1, index_beginClosingTag - (index_endOpeningTag + 1));
      unsigned int adcValue = atoi(adcValue_string.data());

      //std::cout << "adcValue = " << adcValue << std::endl;

      adcValueMap[currentReadOutChip_name][currentTempRange].push_back(adcValue);
    }

    if ( lineBuffer.find("</FPix_") != std::string::npos ) {
      currentReadOutChip_name = "";      
    }

    if ( lineBuffer.find("</dataSet_") != std::string::npos ) {
      currentDataType = "";
    }

    if ( lineBuffer.find("<dcsConditions") != std::string::npos ) {
      
    }

    if ( lineBuffer.find("<dist_1:") != std::string::npos ) {
      int index_beginOpeningTag = lineBuffer.find('<');
      int index_firstSeparator = lineBuffer.find(' ', index_beginOpeningTag + 1);
      int index_endOpeningTag = lineBuffer.find('>', index_beginOpeningTag + 1);
      int index_beginClosingTag = lineBuffer.find("</", index_endOpeningTag + 1);
      //int index_endClosingTag = lineBuffer.find('>', index_beginClosingTag + 1);

      std::string dpeName(lineBuffer, index_beginOpeningTag + 1, index_firstSeparator - (index_beginOpeningTag + 1));

      int index_beginAlias = lineBuffer.find('\"', index_firstSeparator);
      int index_endAlias = lineBuffer.find('\"', index_beginAlias + 1);
      std::string dpeAlias(lineBuffer, index_beginAlias + 1, index_endAlias - (index_beginAlias + 1));

      std::string dpeValue_string(lineBuffer, index_endOpeningTag + 1, index_beginClosingTag - (index_endOpeningTag + 1));
      double dpeValue = atof(dpeValue_string.data());

      //std::cout << "dpeName = " << dpeName << ","
      //	  << " dpeAlias = " << dpeAlias << ","
      //	  << " dpeValue = " << dpeValue << std::endl;

/*
      if ( dpeAlias == "FluroCarbonChiller_reading" ) currentActualTemperature = dpeValue;
      if ( dpeAlias == "PilotRunDetector_D1_analogLV_vMon" ) currentD1_analogLV_vMon = dpeValue;
      if ( dpeAlias == "PilotRunDetector_D1_analogLV_iMon" ) currentD1_analogLV_iMon = dpeValue;
      if ( dpeAlias == "PilotRunDetector_D1_digitalLV_vMon" ) currentD1_digitalLV_vMon = dpeValue;
      if ( dpeAlias == "PilotRunDetector_D1_digitalLV_iMon" ) currentD1_digitalLV_iMon = dpeValue;
      if ( dpeAlias == "PilotRunDetector_D2_analogLV_vMon" ) currentD2_analogLV_vMon = dpeValue;
      if ( dpeAlias == "PilotRunDetector_D2_analogLV_iMon" ) currentD2_analogLV_iMon = dpeValue;
      if ( dpeAlias == "PilotRunDetector_D2_digitalLV_vMon" ) currentD2_digitalLV_vMon = dpeValue;
      if ( dpeAlias == "PilotRunDetector_D2_digitalLV_iMon" ) currentD2_digitalLV_iMon = dpeValue;
 */
      if ( dpeName == "dist_1:OTHER/test_C6F14Chiller.readings.Temperature:_online.._value" ) currentActualTemperature = dpeValue;
/*
      if ( dpeName == "PilotRunDetector_D1_analogLV_vMon" ) currentD1_analogLV_vMon = dpeValue;
      if ( dpeName == "PilotRunDetector_D1_analogLV_iMon" ) currentD1_analogLV_iMon = dpeValue;
      if ( dpeName == "PilotRunDetector_D1_digitalLV_vMon" ) currentD1_digitalLV_vMon = dpeValue;
      if ( dpeName == "PilotRunDetector_D1_digitalLV_iMon" ) currentD1_digitalLV_iMon = dpeValue;
      if ( dpeName == "PilotRunDetector_D2_analogLV_vMon" ) currentD2_analogLV_vMon = dpeValue;
      if ( dpeName == "PilotRunDetector_D2_analogLV_iMon" ) currentD2_analogLV_iMon = dpeValue;
      if ( dpeName == "PilotRunDetector_D2_digitalLV_vMon" ) currentD2_digitalLV_vMon = dpeValue;
      if ( dpeName == "PilotRunDetector_D2_digitalLV_iMon" ) currentD2_digitalLV_iMon = dpeValue;
 */
    }

    if ( lineBuffer.find("</dcsConditions") != std::string::npos ) {
      PixelReadOutChipData_temperatureInfo temperatureInfo(currentNominalTemperature, currentActualTemperature, currentTemperatureCycleType);
      PixelReadOutChipData_powerInfo powerInfo(0.5*(currentD1_analogLV_vMon + currentD2_analogLV_vMon), 
					       0.5*(currentD1_analogLV_iMon + currentD2_analogLV_iMon), 
					       0.5*(currentD1_digitalLV_vMon + currentD2_digitalLV_vMon), 
					       0.5*(currentD1_digitalLV_iMon + currentD2_digitalLV_iMon));
      PixelReadOutChipData_dcsInfo dcsInfo(temperatureInfo, powerInfo);

      rawDataType_fixedPower* rawDataSet = NULL;
      if ( currentReadOutChip_powerMode == kReducedPower ) rawDataSet = &rawDataSet_reducedPower;
      if ( currentReadOutChip_powerMode == kNominalPower ) rawDataSet = &rawDataSet_nominalPower;
      
      if ( rawDataSet != NULL ) {
	for ( std::map<std::string, adcValueType>::iterator adcValueEntry = adcValueMap.begin();
	      adcValueEntry != adcValueMap.end(); ++adcValueEntry ) {
	  const std::string readOutChip_name = adcValueEntry->first;

	  std::map<unsigned int, std::list<unsigned int> > adcValues = adcValueEntry->second;

	  (*rawDataSet)[readOutChip_name].push_back(std::pair<adcValueType, PixelReadOutChipData_dcsInfo>(adcValues, dcsInfo));
	}
      }

      dcsConditionsDataSet.push_back(dcsInfo);

      adcValueMap.clear();
    }

    getline(*inputFile, lineBuffer);
  }

//--- close input file
  delete inputFile;
}

void writeCalibrationConstants_XML(const char* outputFileName,
				   const std::list<std::pair<std::string, PixelReadOutChipData_varyingPower> >& calibratedReadOutChips)
{
//--- check that output file does not yet exist
  std::ifstream testFile;
  testFile.open(outputFileName, std::ifstream::in);
  testFile.close();
  
  if ( !testFile.fail() ) {
//--- in case XML outout file does already exists,
//    print an error message and return
    std::cerr << "Error in <writeCalibrationConstants_XML>: outputFile = " << outputFileName << " does already exist !!!" << std::endl;
    return;
  }

  testFile.clear(std::ios::failbit);
  
//--- now create output file 
//    (in case it does not yet exist)
  std::ofstream* outputFile = new std::ofstream(outputFileName, std::ofstream::out);

//--- write header
  *outputFile << "<pixelTemperatureCalibrationConstants>" << std::endl;

//--- iterate over read-out chips
//    and write calibration constants into output file
  for ( std::list<std::pair<std::string, PixelReadOutChipData_varyingPower> >::const_iterator calibratedReadOutChipEntry = calibratedReadOutChips.begin();
	calibratedReadOutChipEntry != calibratedReadOutChips.end(); ++calibratedReadOutChipEntry ) {
    const std::string& readOutChip_name = calibratedReadOutChipEntry->first;

    const PixelReadOutChipData_fixedPower& calibratedReadOutChip = calibratedReadOutChipEntry->second.getDataSet_nominalPower();

    std::map<std::string, double> readOutChip_calibrationConstants = calibratedReadOutChip.getCalibrationConstants();

    *outputFile << " <" << readOutChip_name << ">" << std::endl;

    for ( std::map<std::string, double>::const_iterator calibrationConstant = readOutChip_calibrationConstants.begin();
	  calibrationConstant != readOutChip_calibrationConstants.end(); ++calibrationConstant ) {
      const std::string& calibrationConstant_name = calibrationConstant->first;
      double calibrationConstant_value = calibrationConstant->second;
/*
      const std::string& calibrationConstant_beginTag = calibrationConstant_name;
      std::string calibrationConstant_endTag;
      unsigned int attribute_index = calibrationConstant_name.find(' ');
      if ( attribute_index != std::string::npos ) {
//--- do not include attributes in end-tag
	calibrationConstant_endTag = std::string(calibrationConstant_name, 0, attribute_index);
      } else {
	calibrationConstant_endTag = calibrationConstant_name;
      }

      *outputFile << "  <" << calibrationConstant_beginTag << ">"
		  << calibrationConstant_value
		  << "</" << calibrationConstant_endTag << ">" << std::endl;
 */
      
      *outputFile << "  <" << calibrationConstant_name << ">"
		  << calibrationConstant_value
		  << "</" << calibrationConstant_name << ">" << std::endl;
    }

    *outputFile << "  <deltaT>" 
		<< calibratedReadOutChipEntry->second.getTemperatureDifference()
		<< "</deltaT>" << std::endl;
    
    *outputFile << " </" << readOutChip_name << ">" << std::endl;
  }

//--- write trailer
  *outputFile << "</pixelTemperatureCalibrationConstants>" << std::endl;

//--- close output file
  delete outputFile;
}

//
//---------------------------------------------------------------------------------------------------
//

TDirectory* changeDirectory(TFile* outputFile, const std::string& directoryName)
{
  std::cout << "<changeDirectory>:" << std::endl;
  std::cout << " directoryName = " << directoryName << std::endl;

//--- change into root directory
//    if requested to do so
  if ( directoryName == "" || directoryName == "/" ) {
    std::cout << "--> changing to root Directory" << std::endl;
    outputFile->cd();
    return outputFile;
  } 

//--- in case the name of a read-out chip 
//     (name according to DocDB #901)
//    has been given as second function argument,
//    the directory structure shall match the detector hierarchy
//   --> replace all underscores by slashes
  std::string directoryName_string = directoryName;

  for ( unsigned int directoryName_index = 0; directoryName_index < directoryName_string.length(); ++directoryName_index ) {
    if ( directoryName_string[directoryName_index] == '_' ) {
      if ( directoryName_index < (directoryName_string.length() - 1) ) {
	directoryName_string.replace(directoryName_index, 1, "/");
      } else {
//--- remove trailing slashes
	directoryName_string.erase(directoryName_index, 1);
      }
    }
  }

  std::cout << "directoryName_string = " << directoryName_string << ","
	    << " length = " << directoryName_string.length() << std::endl;

//--- break full directory name into name of most deeply nested sub-directory 
//    and its parent directory
  TDirectory* parentdirectory = NULL;

  std::string subdirectoryName;

  int lastSlash_index = directoryName_string.find_last_of('/');
  if ( lastSlash_index >= 0 && lastSlash_index < (int)directoryName_string.length() ) {
//--- parent directory is a subdirectory;
//    recursively create and navigate through parent directories
    subdirectoryName = std::string(directoryName_string, lastSlash_index + 1, directoryName_string.length() - (lastSlash_index + 1));
    std::string parentdirectoryName(directoryName_string, 0, lastSlash_index);

    std::cout << " subdirectoryName = " << subdirectoryName << std::endl;
    std::cout << " parentdirectoryName = " << parentdirectoryName << std::endl;

    parentdirectory = changeDirectory(outputFile, parentdirectoryName);
  } else {
//--- parent directory is the root directory
    subdirectoryName = std::string(directoryName_string, 0, directoryName_string.length());

    outputFile->cd();
    parentdirectory = outputFile;
  }

//--- create sub-directory within parent directory
//    (in case it does not yet exist)
  //TDirectory* subdirectory = parentdirectory->GetDirectory(subdirectoryName.data());
  TDirectory* subdirectory = dynamic_cast<TDirectory*>(parentdirectory->FindObject(subdirectoryName.data()));
  std::cout << "parentdirectory->FindObject(" << subdirectoryName.data() << ") = " << subdirectory << std::endl;
  if ( subdirectory == NULL ) {
    if ( parentdirectory != outputFile ) {
      std::cout << "--> creating new sub-Directory = " << subdirectoryName << " within Directory = " << parentdirectory->GetName() << std::endl;
    } else {
      std::cout << "--> creating new sub-Directory = " << subdirectoryName << " within File = " << parentdirectory->GetName() << std::endl;
    }

    subdirectory = parentdirectory->mkdir(subdirectoryName.data());    
  }
    
//--- change into sub-directory
  bool isError = (!subdirectory->cd());

  if ( isError ) {
    std::cerr << "Error in <changeDirectory>:"
              << " failed to change into Directory = " << directoryName << " !!!" << std::endl;
    return NULL;
  }

  return subdirectory;
}

//
//---------------------------------------------------------------------------------------------------
//

void fillTemperatureCycleGraphs(TGraph* graph_CoolingCycles, TGraph* graph_HeatingCycles,
				const dcsConditionsDataSetType& dcsConditionsDataSet)
{
  for ( dcsConditionsDataSetType::const_iterator dcsConditionsDataEntry = dcsConditionsDataSet.begin(); 
	dcsConditionsDataEntry != dcsConditionsDataSet.end(); ++dcsConditionsDataEntry ) {
    const PixelReadOutChipData_temperatureInfo& temperatureInfo = dcsConditionsDataEntry->getTemperatureInfo();

    double nominalTemperature = temperatureInfo.getNominalTemperature();
    double actualTemperature = temperatureInfo.getActualTemperature();

    unsigned int temperatureCycleType = temperatureInfo.getTemperatureCycleType();

    //std::cout << "nominalTemperature = " << nominalTemperature << std::endl;
    //std::cout << "actualTemperature = " << actualTemperature << std::endl;
    //std::cout << "temperatureCycleType = " << temperatureCycleType << std::endl;

    TGraph* graph;
    switch ( temperatureCycleType ) {
    case PixelReadOutChipData_temperatureInfo::kCoolingCycle :
      graph = graph_CoolingCycles;
      break;
    case PixelReadOutChipData_temperatureInfo::kHeatingCycle :
      graph = graph_HeatingCycles;
      break;
    default : 
      std::cerr << "Error in <fillTemperatureCycleGraph>:" 
		<< " undefined Temperature Cycle type = " << temperatureCycleType << " !!!" << std::endl;
      continue;
    }

    graph->SetPoint(graph->GetN(), nominalTemperature, actualTemperature);
  }
}

void drawTemperatureCycleGraphs(TGraph* graph_CoolingCycles, TGraph* graph_HeatingCycles)
{
  double xMin_CoolingCycles, xMax_CoolingCycles, yMin_CoolingCycles, yMax_CoolingCycles;
  getDimension(graph_CoolingCycles, xMin_CoolingCycles, xMax_CoolingCycles, yMin_CoolingCycles, yMax_CoolingCycles);

  double xMin_HeatingCycles, xMax_HeatingCycles, yMin_HeatingCycles, yMax_HeatingCycles;
  getDimension(graph_HeatingCycles, xMin_HeatingCycles, xMax_HeatingCycles, yMin_HeatingCycles, yMax_HeatingCycles);

  double xMin = TMath::Min(xMin_CoolingCycles, xMin_HeatingCycles);
  double xMax = TMath::Max(xMax_CoolingCycles, xMax_HeatingCycles);
  double yMin = TMath::Min(yMin_CoolingCycles, yMin_HeatingCycles);
  double yMax = TMath::Max(yMax_CoolingCycles, yMax_HeatingCycles);
  
//--- initialise dummy histogram
//    (neccessary for drawing graphs)
  TH1* dummyHistogram = new TH1D("dummyHistogram", "dummyHistogram", 10, xMin, xMax);
  dummyHistogram->SetTitle("");
  dummyHistogram->SetStats(false);
  dummyHistogram->GetXaxis()->SetTitle("T_{nominal} / degrees");
  dummyHistogram->GetXaxis()->SetTitleOffset(1.2);
  dummyHistogram->GetYaxis()->SetTitle("T_{actual} / degrees");
  dummyHistogram->GetYaxis()->SetTitleOffset(1.3);
  dummyHistogram->SetMinimum(1.2*yMin);
  dummyHistogram->SetMaximum(1.2*yMax);
  dummyHistogram->Draw();

  graph_CoolingCycles->SetMarkerStyle(2);
  graph_CoolingCycles->SetMarkerSize(2);
  graph_CoolingCycles->SetMarkerColor(4); // blue
  graph_CoolingCycles->Draw("P");

  graph_HeatingCycles->SetMarkerStyle(2);
  graph_HeatingCycles->SetMarkerSize(2);
  graph_HeatingCycles->SetMarkerColor(2); // red
  graph_HeatingCycles->Draw("P");

  TLegend* legend = new TLegend(0.59, 0.15, 0.86, 0.31, NULL, "brNDC");
  legend->SetFillColor(10);
  legend->SetLineColor(10);
  legend->AddEntry(graph_CoolingCycles, "Cooling Cycles", "P");
  legend->AddEntry(graph_HeatingCycles, "Heating Cycles", "P");
  legend->Draw();
}
