/*
 * Copyright (c) 2019 Robert Falkenberg.
 *
 * This file is part of FALCON 
 * (see https://github.com/falkenber9/falcon).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */
#pragma once
#define SPACING 5
#define THRESHOLD_XXKMODE 400
#define PX_PER_DIGIT 10
#define MAX_RANGE_RNTI 60000 // Should be compatible to xxk presentation: 60000 --> 60k

typedef enum{

    RNTI_HIST          = 0,
    RB_OCCUPATION      = 1,
    CELL_THROUGHPUT    = 2,
    PRB_PLOT           = 3,
    MCS_IDX_PLOT       = 4,
    MCS_TBS_PLOT       = 5

}PlotsType_t;

struct xAxisTicks{
public:
  xAxisTicks(){

    for(int mode = 0; mode < 2; mode++){
    for(int numTicks = 0; numTicks < 7; numTicks++){
      QMap<double, QString> result;
      for(int i = 0; i <= numTicks; i++){
          if(!mode){
            result.insert(MAX_RANGE_RNTI/(std::max(1,numTicks))*i, QString::fromStdString(std::to_string(MAX_RANGE_RNTI/(std::max(1,numTicks))*i)));
          }
          else{
            result.insert(MAX_RANGE_RNTI/(std::max(1,numTicks))*i, QString::fromStdString(std::to_string(MAX_RANGE_RNTI/(std::max(1,numTicks))*i/1000) + "k"));
          }
          savedTicks[mode][numTicks] = result;
        }
      }
    }
  }
  QMap<double, QString> getFull(){return full;}
  QMap<double, QString> getHalf(){return half;}
  QMap<double, QString> getLess(){return less;}
  QMap<double, QString> getTicks(int n){
    prevWidth = n;

    int mode = (n < THRESHOLD_XXKMODE) ? (1) : (0); // xxk (3) or xx000 (5) presentation + #spacing whitespace
    int numTicks = std::min(7, (n/((mode) ? (5+SPACING) : (3+SPACING))/PX_PER_DIGIT)); // Maximum of 7 ticks
    numTicks = std::max(1, numTicks); // At least one tick
    return savedTicks[mode][numTicks - 1];
  }

  int getPrevW(){return prevWidth;}
private:
  int prevWidth = 0;
  QMap<double, QString> savedTicks[2][7];
  QMap<double, QString> full = {{0, "0"},
                                {10000, "10000"},
                                {20000, "20000"},
                                {30000, "30000"},
                                {40000, "40000"},
                                {50000, "50000"},
                                {60000, "60000"}
                               };
  QMap<double, QString> half= {{0, "0"},
                               {10000, "10k"},
                               {20000, "20k"},
                               {30000, "30k"},
                               {40000, "40k"},
                               {50000, "50k"},
                               {60000, "60k"}
                              };
  QMap<double, QString> less = {{0, "0"},
                                {20000, "20k"},
                                {40000, "40k"},
                                {60000, "60k"}
                               };
};

typedef enum{

    UPLINK             = 0,
    DOWNLINK           = 1

}LINKTYPE;
