#ifndef MAIN_FUNCTION_H
#define MAIN_FUNCTION_H
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
    void printParameter(std::map<std::string, std::string> &parameterTable);

    //印出每一個衛星的四個鄰居衛星編號
    void printAllSatNeighborId(std::map<int, satellite::satellite> &satellites);

    //印出編號observerId衛星觀察編號otherId衛星一天中的AER數值到sattrack/output.txt
    void printAERfile(int observerId, int otherId, std::map<int, satellite::satellite> &satellites);

    //印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態(單向)到sattrack/output.txt中
    void printRightConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出編號observerId衛星一天中對飛行方向左方衛星的連線狀態(單向)到sattrack/output.txt中
    void printLeftConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出每顆衛星在一天中，左右ISL的一天total可建立連線秒數(雙向皆通才可建立連線)到./outputFile/資料夾中，檔名會是acceptableAzimuthDif_acceptableElevationDif_acceptableRange.txt
    void printAllIslConnectionInfoFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出從方位角誤差80、85、90、...、170、175，所有衛星的左方與右方ISL平均可連線總時間到sattrack/output.txt中
    void compareDifferentAcceptableAzimuthDif(long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable);

    //印出從方PAT_time從5、10、15、...、到60，所有衛星的左方與右方ISL平均可連線總時間到sattrack/output.txt中
    void compareDifferentPAT_time(long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable);

    // 印出整個衛星群的所有inter ISL的breaking及connecting time到sattrack/output.txt中，output每列為"time (sat1Id, sat2Id) 1或0"，1或0表connecting或breaking
    void printBreakingConnectingStatus(std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable);

    //印出某個特定時刻，行星群的連線狀態(totalSatCount*totalSatCount)到sattrack/output.txt中
    void printConstellationStateFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出所有衛星(相鄰的會排在一起)，一天86400秒的ISL設置state(ex: 0-0-0-0X1-1-1-1X0-0-0)
    void adjustableISLdevice_printSatellitesDeviceStateOfDay(std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable);
    
    //印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中
    void printConstellationHopCountFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);
}

#endif