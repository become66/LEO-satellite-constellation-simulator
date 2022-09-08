#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include "getFileData.h"
#include "satellite.h"
#include "AER.h"

#include <iostream>
#include <iomanip>
#include<map>
#include<fstream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <bitset>
#include <set>


namespace mainFunction
{
    //印出設定的parameter
    void printParameter(std::map<std::string, std::string> &parameterTable){
        std::cout<<"**********************************************\nparameters:\n----------------------------------------------\n";
        for(auto p:parameterTable){
            std::cout<<p.first<<": "<<p.second<<"\n";
        }
        std::cout<<"**********************************************\n";
    }

    //印出每一個衛星的四個鄰居衛星編號
    void printAllSatNeighborId(std::map<int, satellite::satellite> &satellites){
        for(auto &sat:satellites){
            sat.second.printNeighborId();
        }
    }

    //印出編號observerId衛星觀察編號otherId衛星一天中的AER數值到sattrack/output.txt
    void printAERfile(int observerId, int otherId, std::map<int, satellite::satellite> &satellites){
        satellite::satellite observer = satellites.at(observerId);
        satellite::satellite other = satellites.at(otherId);
        std::ofstream output("./output.txt");
        output << std::setprecision(8) << std::fixed;
        for (int i = 0; i < 86400; ++i)
        {
            AER curAER = observer.getAER(i, other);
            output<<"satellite"<<observer.getId()<<" observe satellite"<<otherId<<" at date "<<curAER.date <<":    A: "<<curAER.A<<",    E: "<<curAER.E<<",    R: "<<curAER.R<<"\n";
        }
        output.close();

    }

    //印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態(單向)到sattrack/output.txt中
    void printRightConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        satellite::satellite sat = satellites.at(observerId);
        std::ofstream output("./output.txt");
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        for (int time = 0; time < 86400; ++time){
            AER rightSatAER;
            std::bitset<3> connectionState;
            bool result = sat.judgeRightConnectability(time, acceptableAER_diff, connectionState, rightSatAER);
            output<<rightSatAER.date<<":"<<std::setw(10)<<rightSatAER.A<<","<<std::setw(10)<<rightSatAER.E<<","<<std::setw(10)<<rightSatAER.R;
            output<<",   connectability of A: "<<connectionState[0]<<",   connectability of E: "<<connectionState[1]<<",   connectability of R: "<<connectionState[2]<<"  ---->  ";
            if(result)  
                output<<"OK\n";
            else    
                output<<"can't connect\n";
        }
        output.close();
    }

    //印出編號observerId衛星一天中對飛行方向左方衛星的連線狀態(單向)到sattrack/output.txt中
    void printLeftConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        satellite::satellite sat = satellites.at(observerId);
        std::ofstream output("./output.txt");
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        for (int time = 0; time < 86400; ++time){
            AER leftSatAER;
            std::bitset<3> connectionState;
            bool result = sat.judgeLeftConnectability(time, acceptableAER_diff, connectionState, leftSatAER);
            output<<leftSatAER.date<<":"<<std::setw(10)<<leftSatAER.A<<","<<std::setw(10)<<leftSatAER.E<<","<<std::setw(10)<<leftSatAER.R;
            output<<",   connectability of A: "<<connectionState[0]<<",   connectability of E: "<<connectionState[1]<<",   connectability of R: "<<connectionState[2]<<"  ---->  ";
            if(result)  
                output<<"OK\n";
            else    
                output<<"can't connect\n";
        }
        output.close();
    }

    //印出每顆衛星在一天中，左右ISL的一天total可建立連線秒數(雙向皆通才可建立連線)到./outputFile/資料夾中，檔名會是acceptableAzimuthDif_acceptableElevationDif_acceptableRange.txt
    void printAllIslConnectionInfoFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::string outputDir = "./outputFile/" + parameterTable.at("acceptableAzimuthDif") + "_" + parameterTable.at("acceptableElevationDif") + "_" + parameterTable.at("acceptableRange") + ".txt";
        std::ofstream output(outputDir);
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        output << std::setprecision(5) << std::fixed;    
        for(auto &sat: satellites){
            int rightAvailableTime = 0;
            int leftAvailableTime = 0;       
            output<<"sat"<<sat.first<<": ";
            for (int time = 0; time < 86400; ++time){
                if(sat.second.judgeRightISL(time, acceptableAER_diff)) ++rightAvailableTime;
                if(sat.second.judgeLeftISL(time, acceptableAER_diff)) ++leftAvailableTime;
            }
            output<<"rightAvailableTime: "<<rightAvailableTime<<", leftAvailableTime: "<<leftAvailableTime;
            double avgUtilization = (double)(172800+rightAvailableTime+leftAvailableTime)/345600;//86400*2=172800(同軌道前後的衛星永遠可以連線得上), 86400*4=345600
            output<<", average Utilization: "<<avgUtilization<<"\n";                
        }
        output.close();    
    }

    //印出從方位角誤差80、85、90、...、170、175，所有衛星的左方與右方ISL平均可連線總時間到sattrack/output.txt中
    void compareDifferentAcceptableAzimuthDif(long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./output.txt");
        output << std::setprecision(5) << std::fixed; 
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        int PATtime = std::stoi(parameterTable.at("PAT_time"));
        for(int acceptableAzimuthDif = 80; acceptableAzimuthDif <= 175; acceptableAzimuthDif+=5){
            satellite::resetAllISL(ISLtable);
            satellite::resetAllSat(satellites);
            AER acceptableAER_diff("acceptableAER_diff", (double)acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
            double rightAvailableTimeOfAllSat = 0;
            double leftAvailableTimeOfAllSat = 0;
            /*----------scenario2----------*/          
            // satellite::adjustableISLdeviceSetupAllISLstateOfDay(-1, acceptableAER_diff, satellites, ISLtable);
            /*-----------------------------*/                              
            for(auto &sat: satellites){
                /*----------不考慮PAT----------*/
                // int rightAvailableTime = 0;
                // int leftAvailableTime = 0;   
                // for (int time = 0; time < 86400; ++time){
                //     if(sat.second.judgeRightISL(time, acceptableAER_diff)) ++rightAvailableTime;
                //     if(sat.second.judgeLeftISL(time, acceptableAER_diff)) ++leftAvailableTime;
                // }
                // rightAvailableTimeOfAllSat += rightAvailableTime;
                // leftAvailableTimeOfAllSat += leftAvailableTime;

                /*----------考慮PAT----------*/
                std::bitset<86400> rightISLstateOfDay = sat.second.getRightISLstateOfDay(PATtime, acceptableAER_diff);  
                std::bitset<86400> leftISLstateOfDay = sat.second.getLeftISLstateOfDay(PATtime, acceptableAER_diff);
                rightAvailableTimeOfAllSat += rightISLstateOfDay.count(); 
                leftAvailableTimeOfAllSat += leftISLstateOfDay.count();  

                /*----------scenario2--記得註解掉上方scenario2的adjustableISLdeviceSetupAllISLstateOfDay----------*/ 
                // std::bitset<86400> rightISLstateOfDay = sat.second.getRightISL().getStateOfDay();  
                // std::bitset<86400> leftISLstateOfDay = sat.second.getLeftISL().getStateOfDay(); 
                // rightAvailableTimeOfAllSat += rightISLstateOfDay.count();
                // leftAvailableTimeOfAllSat += leftISLstateOfDay.count();                     
            }   
            double rightAvgAvailableTime = rightAvailableTimeOfAllSat / totalSatCount;
            double leftAvgAvailableTime = leftAvailableTimeOfAllSat / totalSatCount;
            double interLinkAvgAvailableTime = (rightAvgAvailableTime+leftAvgAvailableTime)/2;
            // output<<"acceptableAzimuthDif = "<<std::setw(3)<<acceptableAzimuthDif<<" --> "<<", interplane link avg available time: "<<interLinkAvgAvailableTime<<", right link avg available time: "<<rightAvgAvailableTime<<", left link avg available time: "<<leftAvgAvailableTime<<"\n";    
            output<<std::setw(3)<<acceptableAzimuthDif<<","<<std::setw(14)<<interLinkAvgAvailableTime<<","<<std::setw(14)<<rightAvgAvailableTime<<","<<std::setw(14)<<leftAvgAvailableTime<<"\n";
        }
        output.close();    
    }

    //印出從方PAT_time從5、10、15、...、到60，所有衛星的左方與右方ISL平均可連線總時間到sattrack/output.txt中
    void compareDifferentPAT_time(long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./output.txt");
        output << std::setprecision(5) << std::fixed; 
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        // std::map<int, std::bitset<86400>> alreadyCalculate;
        for(int PATtime = 5; PATtime <= 60; PATtime+=5){
            satellite::resetAllISL(ISLtable);
            double rightAvailableTimeOfAllSat = 0;
            double leftAvailableTimeOfAllSat = 0;
            for(auto &sat: satellites){
                std::bitset<86400> rightISLstateOfDay = sat.second.getRightISLstateOfDay(PATtime, acceptableAER_diff);  
                std::bitset<86400> leftISLstateOfDay = sat.second.getLeftISLstateOfDay(PATtime, acceptableAER_diff);                        
                rightAvailableTimeOfAllSat += rightISLstateOfDay.count(); 
                leftAvailableTimeOfAllSat += leftISLstateOfDay.count();
            }   
            double rightAvgAvailableTime = rightAvailableTimeOfAllSat / totalSatCount;
            double leftAvgAvailableTime = leftAvailableTimeOfAllSat / totalSatCount;
            double interLinkAvgAvailableTime = (rightAvgAvailableTime+leftAvgAvailableTime)/2;
            // output<<"PATtime = "<<std::setw(3)<<PATtime<<" --> "<<", interplane link avg available time: "<<interLinkAvgAvailableTime<<", right link avg available time: "<<rightAvgAvailableTime<<", left link avg available time: "<<leftAvgAvailableTime<<"\n";    
            output<<std::setw(3)<<PATtime<<","<<std::setw(14)<<interLinkAvgAvailableTime<<","<<std::setw(14)<<rightAvgAvailableTime<<","<<std::setw(14)<<leftAvgAvailableTime<<"\n";
        }
        output.close();    
    }

    // 印出整個衛星群的所有inter ISL的breaking及connecting time到sattrack/output.txt中，output每列為"time (sat1Id, sat2Id) 1或0"，1或0表connecting或breaking
    void printBreakingConnectingStatus(std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable){
            /*int是時間*/        /*set<int>是ISL的兩顆衛星編號*/   /*bool代表on or off*/
        std::map<int, std::set< std::pair<std::set<int>,  bool>>> breakingConnectingStatus;
        std::ofstream output("./output.txt");
        output << std::setprecision(5) << std::fixed; 
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        int PATtime = std::stoi(parameterTable.at("PAT_time"));
        satellite::setupAllISLstateOfDay(PATtime, acceptableAER_diff, satellites);//將每一個ISL的stateOfDay計算出來
        for(auto &ISL: ISLtable){//ISL.first是ISL的一對衛星id
            std::vector<std::pair<int, bool>> stateChangeInfo = ISL.second.getStateChangeInfo();
            for(const auto &changingPoint: stateChangeInfo){ //changingPoint的第一個elemet是秒數，第二個element是標示改變是由0->1還是1->0
                breakingConnectingStatus[changingPoint.first].insert(std::make_pair(ISL.first, changingPoint.second));
            }
        }
        for(auto &pair: breakingConnectingStatus){
            for(auto &p: pair.second){ //p.first是ISL的一對衛星id, p.second是標示改變是由0->1還是1->0
                output<<std::setw(5)<<pair.first<<" ("<<*p.first.begin()<<","<<*p.first.rbegin()<<") "<<p.second<<"\n";
            }
        }
        output.close();
    }

    //印出某個特定時刻，行星群的連線狀態(totalSatCount*totalSatCount)到sattrack/output.txt中
    void printConstellationStateFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./output.txt");
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
        int time = std::stoi(parameterTable.at("time"));
        int PAT_time = std::stoi(parameterTable.at("PAT_time"));
        std::vector<std::vector<int>> constellationState = satellite::getConstellationState(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
        //印出totalSatCount*totalSatCount二維陣列
        for(auto row: constellationState){
            for(auto i: row){
                output<<std::setw(5)<<i;
            }
            output<<"\n";
        }
        output.close();
    }

    //印出所有衛星(相鄰的會排在一起)，一天86400秒的ISL設置state(ex: 0-0-0-0X1-1-1-1X0-0-0)
    void adjustableISLdevice_printSatellitesDeviceStateOfDay(std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./output.txt");


        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
        satellite::adjustableISLdeviceSetupAllISLstateOfDay(-1, acceptableAER_diff, satellites, ISLtable);   
        if(parameterTable.at("TLE_inputFileName") == "TLE_7P_16Sats.txt"){
            for(size_t t = 0; t < 86400; ++t){
                satellite::satellite sat = satellites.at(101);
                output<<sat.getCertainTimeISLdeviceState(t);
                if(sat.getRightISL().getSecondState(t)) output<<"-";
                else    output<<"X";
                sat = sat.getRightSat();
                while(sat.getId() != 101){
                    output<<sat.getCertainTimeISLdeviceState(t);
                    if(sat.getRightISL().getSecondState(t)) output<<"-";
                    else    output<<"X";
                    sat = sat.getRightSat();
                }
                output<<"  ";
                sat = satellites.at(102);
                output<<sat.getCertainTimeISLdeviceState(t);
                if(sat.getRightISL().getSecondState(t)) output<<"-";
                else    output<<"X";            
                sat = sat.getRightSat();
                while(sat.getId() != 102){
                    output<<sat.getCertainTimeISLdeviceState(t);
                    if(sat.getRightISL().getSecondState(t)) output<<"-";
                    else    output<<"X";                
                    sat = sat.getRightSat();
                }
                output<<"  ";
                sat = satellites.at(103);
                output<<sat.getCertainTimeISLdeviceState(t);
                if(sat.getRightISL().getSecondState(t)) output<<"-";
                else    output<<"X";            
                sat = sat.getRightSat();
                while(sat.getId() != 103){
                    output<<sat.getCertainTimeISLdeviceState(t);
                    if(sat.getRightISL().getSecondState(t)) output<<"-";
                    else    output<<"X";                
                    sat = sat.getRightSat();
                }
                output<<"  ";
                sat = satellites.at(104);
                if(sat.getRightISL().getSecondState(t)) output<<"-";
                else    output<<"X";            
                output<<sat.getCertainTimeISLdeviceState(t);
                sat = sat.getRightSat();
                while(sat.getId() != 104){
                    output<<sat.getCertainTimeISLdeviceState(t);
                    if(sat.getRightISL().getSecondState(t)) output<<"-";
                    else    output<<"X";                
                    sat = sat.getRightSat();
                }                                  
                output<<"\n";
            }
        }        
        else if(parameterTable.at("TLE_inputFileName") == "TLE_6P_22Sats.txt"){
            for(size_t t = 0; t < 86400; ++t){
                satellite::satellite sat = satellites.at(101);
                output<<sat.getCertainTimeISLdeviceState(t);
                if(sat.getRightISL().getSecondState(t)) output<<"-";
                else    output<<"X";                
                sat = sat.getRightSat();
                while(sat.getId() != 101){
                    output<<sat.getCertainTimeISLdeviceState(t);
                    if(sat.getRightISL().getSecondState(t)) output<<"-";
                    else    output<<"X";                    
                    sat = sat.getRightSat();
                }
                output<<"\n";
            }        
        } 
        output.close();    
    }

    //印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中
    void printConstellationHopCountFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./output.txt");
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
        int time = std::stoi(parameterTable.at("time"));
        int PAT_time = std::stoi(parameterTable.at("PAT_time"));
        std::vector<std::vector<int>> constellationHopCount = satellite::getConstellationHopCount(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
        for(auto row: constellationHopCount){
            for(auto i: row){
                output<<std::setw(4)<<i;
            }
            output<<"\n";
        }
        output.close(); 
    }
}

