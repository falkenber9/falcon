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
#include "mainwindow.h"
#include "qcustomplot/qcustomplot.h"
#include "plots.h"

void MainWindow::setupPlot(PlotsType_t plottype, QCustomPlot *plot){

  if(plottype == RNTI_HIST){

    plot->addGraph(); //Blue line
    plot->graph(0)->setPen(QPen(QColor(1,1,200)));
    plot->graph(0)->setLineStyle(QCPGraph::lsImpulse);

//    plot->addGraph(); //blue line
//    plot->graph(1)->setPen(QPen(QColor(40,110,255)));
//    plot->graph(1)->setLineStyle(QCPGraph::lsImpulse);

    plot->xAxis->setRange(0,65536);
   // plot->xAxis->setLabel("RNTI");
    plot->xAxis2->setLabel("RNTI Histogram");
    //plot->yAxis->setRange(0,100000);
    plot->yAxis->setRange(1e1, 1e5);
    plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    plot->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->yAxis->setTicker(logTicker);
    plot->yAxis2->setTicker(logTicker);
    plot->axisRect()->setupFullAxesBox();

    //connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange))); //evt. for later
    //connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));

  }

  /*if(plottype == RB_OCCUPATION || plottype == CELL_THROUGHPUT){

    plot->addGraph(); // blue line
    plot->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    plot->xAxis->setTicker(timeTicker);
    plot->axisRect()->setupFullAxesBox();
    plot->yAxis->setRange(0, 30);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    //connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    //dataTimer.start(0); // Interval 0 means to refresh as fast as possible
  }*/

  if(plottype == MCS_TBS_PLOT){

    plot->addGraph(); //orange line
    plot->graph(0)->setName("Uplink");
    plot->graph(0)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));

    plot->addGraph(); //blue line
    plot->graph(1)->setName("Downlink");
    plot->graph(1)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));


    /*// Legend for Graph Names:
    plot->legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(8); // and make a bit smaller for legend
    plot->legend->setFont(legendFont);
    plot->legend->setBrush(QBrush(QColor(0,0,0,0)));
    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignLeft);*/

    // Settings for Axis:
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    plot->xAxis->setTicker(timeTicker);
    plot->xAxis2->setLabel("Transport-Block-Size");
    plot->yAxis->setRange(0, 80000);
    //plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    //plot->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    //QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    //plot->yAxis->setTicker(logTicker);
    //plot->yAxis2->setTicker(logTicker);
    plot->axisRect()->setupFullAxesBox();

  }

  if(plottype == MCS_IDX_PLOT){

    plot->addGraph(); //orange line
    plot->graph(0)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));

    plot->addGraph(); //blue line
    plot->graph(1)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    plot->xAxis->setTicker(timeTicker);
    plot->xAxis2->setLabel("IDX");
    plot->axisRect()->setupFullAxesBox();
    plot->yAxis->setRange(0, 100);

    // make left and bottom axes transfer their ranges to right and top axes:
    // connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    // connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));

  }

  if(plottype == PRB_PLOT){

    plot->addGraph(); //orange line
    plot->graph(0)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));

    plot->addGraph(); //blue line
    plot->graph(1)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    plot->xAxis->setTicker(timeTicker);
    plot->xAxis2->setLabel("Resourceblocks/Subframe");
    plot->axisRect()->setupFullAxesBox();
    plot->yAxis->setRange(0, glob_settings.glob_args.decoder_args.file_nof_prb);

    // make left and bottom axes transfer their ranges to right and top axes:
    // connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    // connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));

  }

}

void MainWindow::addData(PlotsType_t plottype, QCustomPlot *plot, const ScanLineLegacy *data){

  if(plottype == RNTI_HIST){
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    static uint32_t rnti_hist_sum[65536];

    if (key - lastPointKey > 0.1){ // at most add point every 100ms

      /*plot->removeGraph(0);
    //plot->addGraph();
    plot->graph(0)->setPen(QPen(QColor(40, 110, 255)));*/

      for(int i = 0; i <= 65535; i++){
        if(data->rnti_hist[i] >= 10){
          rnti_hist_sum[i] += data->rnti_hist[i]; //Sum rnti_hist up.
          //if(rnti_hist_sum[i] > 4000000000) rnti_hist_sum[i] = 4000000000;  //evtl. overflow protection
          plot->graph(0)->addData(i,rnti_hist_sum[i]);
        }
      }
      lastPointKey = key;
      plot->replot();
    }

  }
  if(plottype == RB_OCCUPATION || plottype == CELL_THROUGHPUT){
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    /*static double lastPointKey = 0;
    if (key-lastPointKey > 0.002) // at most add point every 2 ms
    {
        // add data to lines:
        plot->graph(0)->addData(key, qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
        // rescale value (vertical) axis to fit the current data:
        //ui->customPlot->graph(0)->rescaleValueAxis();
        //ui->customPlot->graph(1)->rescaleValueAxis(true);
        lastPointKey = key;
    }*/
    int rnti_counter = 0;
    for(int i = 0; i < 65000; i++)if(data->rnti_hist[i] > 10) rnti_counter++;

    plot->graph(0)->addData(key,rnti_counter);

    key++;

    // make key axis range scroll with the data (at a constant range size of 8):
    plot->xAxis->setRange(key, 8, Qt::AlignRight);
    plot->replot();
  }
}

void MainWindow::draw_plot(const ScanLineLegacy *line){


  if(line->type == SCAN_LINE_PERF_PLOT_A){

    if(line->sfn != sfn_old_a){

      if(mcs_idx_sum_counter_a != 0){


        //qDebug() <<"\nMCS_IDX mean: "<< mcs_idx_sum / mcs_idx_sum_counter <<", TBS_SUM:" << mcs_tbs_sum << ", L_PRB_SUM:"<< l_prb_sum ;

        static QTime time(QTime::currentTime());
        double key = time.elapsed() * 0.001; // time elapsed since start of demo, in milliseconds
        static double last_key = 0;

        mcs_idx_sum_sum_a += mcs_idx_sum_a / mcs_idx_sum_counter_a;
        mcs_tbs_sum_sum_a += mcs_tbs_sum_a;
        l_prb_sum_sum_a   += l_prb_sum_a;

        if((key - last_key) * 1000 > plot_mean_slider_a->value()){

          mcs_idx_plot_a->graph(0)->addData(key,mcs_idx_sum_sum_a / sum_sum_counter_a);
          mcs_tbs_plot_a->graph(0)->addData(key,mcs_tbs_sum_sum_a / sum_sum_counter_a);
          prb_plot_a    ->graph(0)->addData(key,(l_prb_sum_sum_a  / (sum_sum_counter_a * 10)));

          mcs_idx_sum_sum_a = 0;
          mcs_tbs_sum_sum_a = 0;
          l_prb_sum_sum_a   = 0;
          sum_sum_counter_a = 0;

          mcs_idx_plot_a->yAxis->rescale(true);
          //mcs_tbs_plot_a->yAxis->rescale(true);
          //prb_plot_a->yAxis->    rescale(true);

          // make key axis range scroll with the data (at a constant range size of 10 sec):
          mcs_idx_plot_a->xAxis->setRange(key, 10, Qt::AlignRight);
          mcs_idx_plot_a       ->replot();
          mcs_tbs_plot_a->xAxis->setRange(key, 10, Qt::AlignRight);
          mcs_tbs_plot_a       ->replot();
          prb_plot_a    ->xAxis->setRange(key, 10, Qt::AlignRight);
          prb_plot_a           ->replot();

          last_key = key;


        }

        sum_sum_counter_a++;

      }

      mcs_idx_sum_a = line->mcs_idx; // Save new value for next round.
      mcs_idx_sum_counter_a = 1;

      mcs_tbs_sum_a = line->mcs_tbs; //Save Values for next round.
      l_prb_sum_a   = line->l_prb;

      //qDebug() <<"\n New Subframe: \n";

      sfn_old_a = line->sfn;
    }
    else{

      mcs_tbs_sum_a += line->mcs_tbs;   //Sum all values.
      l_prb_sum_a   += line->l_prb;
      mcs_idx_sum_a += line->mcs_idx;
      mcs_idx_sum_counter_a++;
    }

    //qDebug() << "SF_ID:"<< line->sf_idx << ", SFN:"<< line->sfn << ", MCS_IDX:"<< line->mcs_idx << ", MCS_TBS:"<< line->mcs_tbs << ", L_PRB:"<< line->l_prb;


  }
  if(line->type == SCAN_LINE_PERF_PLOT_B){

    if(line->sfn != sfn_old_b){

      if(mcs_idx_sum_counter_b != 0){


        //qDebug() <<"\nMCS_IDX mean: "<< mcs_idx_sum / mcs_idx_sum_counter <<", TBS_SUM:" << mcs_tbs_sum << ", L_PRB_SUM:"<< l_prb_sum ;

        static QTime time(QTime::currentTime());
        double key = time.elapsed() * 0.001 ; // time elapsed since start of demo, in milliseconds
        static double last_key = 0;

        mcs_idx_sum_sum_b += mcs_idx_sum_b / mcs_idx_sum_counter_b;
        mcs_tbs_sum_sum_b += mcs_tbs_sum_b;
        l_prb_sum_sum_b   += l_prb_sum_b;

        //qDebug() << "key: " << key << " last key: " << last_key << " Diff: " << key- last_key;

        if((key - last_key) * 1000 > plot_mean_slider_a->value()){

          //qDebug() << "Taken";

          mcs_idx_plot_a->graph(1)->addData(key,mcs_idx_sum_sum_b / sum_sum_counter_b);
          mcs_tbs_plot_a->graph(1)->addData(key,mcs_tbs_sum_sum_b / sum_sum_counter_b);
          prb_plot_a    ->graph(1)->addData(key,(l_prb_sum_sum_b  / (sum_sum_counter_b * 10)));

          mcs_idx_sum_sum_b = 0;
          mcs_tbs_sum_sum_b = 0;
          l_prb_sum_sum_b   = 0;
          sum_sum_counter_b = 0;

          // make key axis range scroll with the data (at a constant range size of 1000):
        /*  mcs_idx_plot_a->xAxis->setRange(key, 5000, Qt::AlignRight);
          mcs_idx_plot_a       ->replot();
          mcs_tbs_plot_a->xAxis->setRange(key, 5000, Qt::AlignRight);
          mcs_tbs_plot_a       ->replot();
          prb_plot_a    ->xAxis->setRange(key, 5000, Qt::AlignRight);
          prb_plot_a           ->replot();
*/
          last_key = key;


        }

        sum_sum_counter_b++;

      }

      mcs_idx_sum_b = line->mcs_idx; // Save new value for next round.
      mcs_idx_sum_counter_b = 1;

      mcs_tbs_sum_b = line->mcs_tbs; //Save Values for next round.
      l_prb_sum_b   = line->l_prb;

      //qDebug() <<"\n New Subframe: \n";

      sfn_old_b = line->sfn;
    }
    else{

      mcs_tbs_sum_b += line->mcs_tbs;   //Sum all values.
      l_prb_sum_b   += line->l_prb;
      mcs_idx_sum_b += line->mcs_idx;
      mcs_idx_sum_counter_b++;
    }

    //qDebug() << "SF_ID:"<< line->sf_idx << ", SFN:"<< line->sfn << ", MCS_IDX:"<< line->mcs_idx << ", MCS_TBS:"<< line->mcs_tbs << ", L_PRB:"<< line->l_prb;

  }


  delete line;
}

void MainWindow::draw_rnti_hist(const ScanLineLegacy *line){

  addData(RNTI_HIST, rnti_hist_plot_a, line);
  delete line;
}

void MainWindow::update_plot_color(){

  mcs_idx_plot_a->graph(0)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));
  mcs_idx_plot_a->graph(1)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));

  mcs_tbs_plot_a->graph(0)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));
  mcs_tbs_plot_a->graph(1)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));

  prb_plot_a->graph(0)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));
  prb_plot_a->graph(1)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));

}
