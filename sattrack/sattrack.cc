/*
 * Copyright 2013 Daniel Warner <contact@danrw.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include "getFileData.h"
#include "rectifyAzimuth.h"
#include "satellite.h"
#include "AER.h"
#include "mainFunction.h"

#include <iostream>
#include <iomanip>
#include<map>
#include<fstream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <bitset>
#include <set>

//for using std::string type in switch statement
constexpr unsigned int str2int(const char* str, int h = 0){
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ (unsigned int)str[h];
}



int main()
{
    // std::cout<<sizeof(satellite::satellite)<<"\n";
    clock_t start, End;
    double cpu_time_used;
    start = clock();    
    /*---------------------------------------*/
    
    std::map<std::string, std::string> parameterTable  = getFileData::getParameterdata("parameter.txt");
    int ISLfrontAngle = std::stoi(parameterTable.at("ISLfrontAngle"));
    int ISLrightAngle = std::stoi(parameterTable.at("ISLrightAngle"));
    int ISLbackAngle = std::stoi(parameterTable.at("ISLbackAngle"));
    int ISLleftAngle = std::stoi(parameterTable.at("ISLleftAngle"));
    std::string TLE_inputFileName = parameterTable.at("TLE_inputFileName");
    long unsigned int totalSatCount = 0, satCountPerOrbit = 0;
    if(TLE_inputFileName == "TLE_7P_16Sats.txt"){
        totalSatCount = 112;
        satCountPerOrbit = 16;
    }
    else if(TLE_inputFileName == "TLE_6P_22Sats.txt"){
        totalSatCount = 132;
        satCountPerOrbit = 22;
    }
    std::map<int, satellite::satellite> satellites = getFileData::getSatellitesTable(TLE_inputFileName, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
    std::map<std::set<int>, satellite::ISL> ISLtable = satellite::getISLtable(satellites);

    //讓衛星物件知道自己的鄰居及ISL是誰(指標指到鄰居衛星及ISL)
    for(auto &sat:satellites){
        sat.second.buildNeighborSatsAndISLs(satellites, ISLtable);
    }
    mainFunction::printParameter(parameterTable);
    std::cout<<"running function "<<parameterTable["execute_function"]<<"...\n";

    switch (str2int(parameterTable["execute_function"].c_str()))
    {
        case str2int("printAllSatNeighborId"):
            mainFunction::printAllSatNeighborId(satellites);
            break;
        case str2int("printAERfile"):
            mainFunction::printAERfile(std::stoi(parameterTable.at("observerId")), std::stoi(parameterTable.at("otherId")),satellites);
            break;
        case str2int("printRightConnectabilityFile"):
            mainFunction::printRightConnectabilityFile(std::stoi(parameterTable.at("observerId")),satellites,parameterTable);
            break;
        case str2int("printLeftConnectabilityFile"):
            mainFunction::printLeftConnectabilityFile(std::stoi(parameterTable.at("observerId")),satellites,parameterTable);
            break;
        case str2int("printAllIslConnectionInfoFile"):
            mainFunction::printAllIslConnectionInfoFile(satellites,parameterTable);
            break;
        case str2int("compareDifferentAcceptableAzimuthDif"):
            mainFunction::compareDifferentAcceptableAzimuthDif(totalSatCount, satellites, ISLtable ,parameterTable);
            break;   
        case str2int("compareDifferentPAT_time"):
            mainFunction::compareDifferentPAT_time(totalSatCount, satellites, ISLtable ,parameterTable);            
            break;  
        case str2int("printBreakingConnectingStatus"):
            mainFunction::printBreakingConnectingStatus(satellites,ISLtable, parameterTable);
            break;                                      
        case str2int("printConstellationStateFile"):
            mainFunction::printConstellationStateFile(satCountPerOrbit, totalSatCount, satellites,parameterTable);
            break; 
        case str2int("adjustableISLdevice_printSatellitesDeviceStateOfDay"):
            mainFunction::adjustableISLdevice_printSatellitesDeviceStateOfDay(satellites, ISLtable ,parameterTable);
            break;                  
        default:
            std::cout<<"running test!"<<"\n";
            /*-------------test-------------*/
            std::ofstream output("./output.txt");
            double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
            double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
            double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
            AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
            satellite::adjustableISLdeviceSetupAllISLstateOfDay(-1, acceptableAER_diff, satellites, ISLtable); 
            // int rightAvailableTimeOfAllSat = 0;
            // int leftAvailableTimeOfAllSat = 0;   
            std::vector<int> connectionCount(86400,0);
            std::vector<std::vector<std::set<int>>> connectionFailPairs(86400);         
            for(auto &ISLpair: ISLtable){
                /*----------scenario2--記得註解掉上方scenario2的adjustableISLdeviceSetupAllISLstateOfDay----------*/ 
                std::bitset<86400> rightISLstateOfDay = ISLpair.second.getStateOfDay();  
                for(size_t i = 0; i < 86400; ++i){
                    if(rightISLstateOfDay[i]){
                        connectionCount[i]++;
                    }
                    else{
                        connectionFailPairs[i].push_back(ISLpair.first);
                    }
                }           
            }   
            for(size_t i = 0; i < 86400; ++i){
                output<<connectionCount[i];
                for(auto satPair:connectionFailPairs[i]){
                    output<<", "<<" ("<<*satPair.begin()<<","<<*satPair.rbegin()<<")";
                }
                output<<"\n";
            }

            output.close();
            /*------------end test---------*/





            break;
    }




    /*-------------------------------------------*/
    End = clock();
    cpu_time_used = ((double) (End - start)) / CLOCKS_PER_SEC;
    std::cout<<"cpu_time_used: "<<cpu_time_used<<"\n";
    return 0;
}



//測試function satellite::judgeAzimuth，印出acceptableAngle介於0~180，連線裝置設在角度ISLdirAngle，觀測衛星位在角度otherSatAngle時，可否連線
void testJudgeAzimuthFunction(int ISLdirAngle, int otherSatAngle){
    for(int acceptableAngle = 0; acceptableAngle < 180; ++acceptableAngle){
        if(satellite::judgeAzimuth(ISLdirAngle, acceptableAngle, otherSatAngle)){
            std::cout<<"acceptableAngle: "<<acceptableAngle<<"->"<<"can connect"<<"\n";
        }
        else{
            std::cout<<"acceptableAngle: "<<acceptableAngle<<"->"<<"can not connect"<<"\n";
        }
    }    
}
