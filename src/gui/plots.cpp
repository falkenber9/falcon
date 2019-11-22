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

#define HO_MARGIN_RESCALE 100
#define TA_SPACING 2
#define TIMEFORMAT "%h:%m:%s"
#define TA_DIGITS_PER_DISPLAY 80
#define OVERSCAN 1.15

void MainWindow::setupPlot(PlotsType_t plottype, QCustomPlot *plot){

  if(plottype == RNTI_HIST){

    plot->addGraph(); //Blue line
    plot->graph(UPLINK)->setPen(QPen(QColor(1,1,200)));
    plot->graph(UPLINK)->setLineStyle(QCPGraph::lsImpulse);

    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    plot->yAxis->setRange(1, 2000);
    plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    plot->yAxis2->setScaleType(QCPAxis::stLogarithmic);
    plot->yAxis->setTicker(logTicker);
    plot->yAxis2->setTicker(logTicker);

    plot->xAxis->setRange(0,65535);
    plot->xAxis2->setLabel("RNTI Histogram");
    xTicker = QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText);
    plot->xAxis->setTicker(xTicker);
    plot->xAxis2->setTicker(xTicker);
    xTicker->setTickStepStrategy(QCPAxisTicker::TickStepStrategy::tssReadability);
    xTicker->setTicks(xAT.getTicks(plot_rnti_hist->width()));

    plot->axisRect()->setupFullAxesBox();
  }

  if(plottype == MCS_TBS_PLOT){

    plot->addGraph(); //orange line
    plot->graph(UPLINK)->setName("Uplink");
    plot->graph(UPLINK)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));

    plot->addGraph(); //blue line
    plot->graph(DOWNLINK)->setName("Downlink");
    plot->graph(DOWNLINK)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));


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
    timeTicker->setTimeFormat(TIMEFORMAT);
    timeTicker->setTickStepStrategy(QCPAxisTicker::TickStepStrategy::tssReadability);
    plot->xAxis->setTicker(timeTicker);
    plot->xAxis2->setLabel("Cell Throughput (TBS) [Mbit/s]");
    plot->axisRect()->setupFullAxesBox();
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);

  }

  if(plottype == MCS_IDX_PLOT){

    plot->addGraph(); //orange line
    plot->graph(UPLINK)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));
    plot->graph(UPLINK)->setName("Uplink");

    plot->addGraph(); //blue line
    plot->graph(DOWNLINK)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));
    plot->graph(DOWNLINK)->setName("Downlink");

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat(TIMEFORMAT);
    timeTicker->setTickStepStrategy(QCPAxisTicker::TickStepStrategy::tssReadability);
    plot->xAxis->setTicker(timeTicker);
    plot->xAxis2->setLabel("MCS Index");
    plot->axisRect()->setupFullAxesBox();
    plot->yAxis->setRange(0, 100);
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);
  }

  if(plottype == PRB_PLOT){

    plot->addGraph(); //orange line
    plot->graph(UPLINK)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));
    plot->graph(UPLINK)->setName("Uplink");


    plot->addGraph(); //blue line
    plot->graph(DOWNLINK)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));
    plot->graph(DOWNLINK)->setName("Downlink");


    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat(TIMEFORMAT);
    timeTicker->setTickStepStrategy(QCPAxisTicker::TickStepStrategy::tssReadability);
    plot->xAxis->setTicker(timeTicker);
    plot->xAxis2->setLabel("Resourceblocks/Subframe");
    plot->axisRect()->setupFullAxesBox();
    plot->yAxis->setRange(0, SPECTROGRAM_MAX_LINE_WIDTH);
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);




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
    std::vector<double> rnti_hist_sum(65536);

    if(abs(xAT.getPrevW() - plot_rnti_hist->width()) > HO_MARGIN_RESCALE){
      xTicker->setTicks(xAT.getTicks(plot_rnti_hist->width()));
    }

    if ((key - lastPointKey)*1000 > plot_mean_slider_a->value()){ // Update RNTI according to slider

      std::for_each(data->rnti_active_set.begin(), data->rnti_active_set.end(), [&rnti_hist_sum](rnti_manager_active_set_t i){ rnti_hist_sum[i.rnti] = i.frequency;});
      plot->graph(UPLINK)->setData(QVector<double>::fromStdVector(rnti_x_axis), QVector<double>::fromStdVector(rnti_hist_sum));

      lastPointKey = key;
      //            plot->replot();
    }

  }
  if(plottype == RB_OCCUPATION || plottype == CELL_THROUGHPUT){
    // calculate two new data points:
    double key = QTime::currentTime().msecsSinceStartOfDay()*0.001; // time elapsed since start of demo, in seconds
    int rnti_counter = 0;
    for(int i = 0; i < 65000; i++)if(data->rnti_hist[i] > 10) rnti_counter++;

    plot->graph(UPLINK)->addData(key,rnti_counter);

    key++;

    // make key axis range scroll with the data (at a constant range size of 8):
    plot->xAxis->setRange(key, 8, Qt::AlignRight);
    plot->replot();
  }
}


void MainWindow::calc_performance_data(const ScanLineLegacy *line){
  perf_mutex.lock();
  // Fix axes
  if(plot_mcs_idx->width()/TA_DIGITS_PER_DISPLAY - TA_SPACING != plot_mcs_idx->xAxis->ticker()->tickCount()){
    plot_mcs_idx->xAxis->ticker()->setTickCount(std::max(plot_mcs_idx->width()/TA_DIGITS_PER_DISPLAY - TA_SPACING, 1));
  }
  if(plot_throughput->width()/TA_DIGITS_PER_DISPLAY - TA_SPACING != plot_throughput->xAxis->ticker()->tickCount()){
    plot_throughput->xAxis->ticker()->setTickCount(std::max(plot_throughput->width()/TA_DIGITS_PER_DISPLAY - TA_SPACING, 1));
  }
  if(plot_prb    ->width()/TA_DIGITS_PER_DISPLAY - TA_SPACING != plot_prb    ->xAxis->ticker()->tickCount()){
    plot_prb->xAxis->ticker()->setTickCount(std::max(plot_prb->width()/TA_DIGITS_PER_DISPLAY - TA_SPACING, 1));
  }

  /*  Assign pointer set according to uplink/downlink case:
     *
     *  There are variables for uplink/downlink calculation each.
     *  Depending on the link type a set of pointers is set to the corresponding set of variables,
     *  the calculation is then operated based on the pointer set.
     */

  if(line->type == SCAN_LINE_PERF_PLOT_A){
    mcs_tbs = &mcs_tbs_sum_uplink;
    l_prb = &l_prb_sum_uplink;
    mcs_idx = &mcs_idx_uplink;
    nof_allocations = &nof_allocations_uplink;
    nof_received_sf = &nof_received_sf_uplink;
    last_timestamp = &last_timestamp_uplink;
    sf_idx_old = &sf_idx_old_uplink;
    graph_mcs_idx = plot_mcs_idx->graph(UPLINK);
    graph_throughput   =  plot_throughput->graph(UPLINK);
    graph_prb =  plot_prb->graph(UPLINK);
  }else
  {
    mcs_tbs = &mcs_tbs_sum_downlink;
    l_prb = &l_prb_sum_downlink;
    mcs_idx = &mcs_idx_sum_downlink;
    nof_allocations = &nof_allocations_downlink;
    nof_received_sf = &nof_received_sf_downlink;
    last_timestamp = &last_timestamp_downlink;
    sf_idx_old = &sf_idx_old_downlink;
    graph_mcs_idx = plot_mcs_idx->graph(DOWNLINK);
    graph_throughput   =  plot_throughput->graph(DOWNLINK);
    graph_prb =  plot_prb->graph(DOWNLINK);
  }

  // Sum everything up
  (*mcs_tbs) += line->mcs_tbs;
  (*l_prb)   += line->l_prb;
  (*mcs_idx) += line->mcs_idx;
  (*nof_allocations)++;

  // Update sf_idx if new subframe has started
  if(line->sf_idx != *sf_idx_old){
    (*nof_received_sf)++;
    *sf_idx_old = line->sf_idx;
  }
  perf_mutex.unlock();
  delete line;
}

void MainWindow::draw_rnti_hist(const ScanLineLegacy *line){
  addData(RNTI_HIST, plot_rnti_hist, line);
  delete line;
}

void MainWindow::update_plot_color(){

  plot_mcs_idx->graph(UPLINK)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));
  plot_mcs_idx->graph(DOWNLINK)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));

  plot_throughput->graph(UPLINK)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));
  plot_throughput->graph(DOWNLINK)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));

  plot_prb->graph(UPLINK)->setPen(QPen(glob_settings.glob_args.gui_args.uplink_plot_color));
  plot_prb->graph(DOWNLINK)->setPen(QPen(glob_settings.glob_args.gui_args.downlink_plot_color));

}

void MainWindow::draw_plot_uplink(){
  double timestamp = QTime::currentTime().msecsSinceStartOfDay()*0.001; // day time in milliseconds
  perf_mutex.lock();

  mcs_tbs = &mcs_tbs_sum_uplink;
  l_prb = &l_prb_sum_uplink;
  mcs_idx = &mcs_idx_uplink;
  nof_allocations = &nof_allocations_uplink;
  nof_received_sf = &nof_received_sf_uplink;
  graph_mcs_idx = plot_mcs_idx->graph(UPLINK);
  graph_throughput   =  plot_throughput->graph(UPLINK);
  graph_prb =  plot_prb->graph(UPLINK);

  if(*nof_received_sf != 0){    // Only plot if at least one subframe was received

    /*  CALCULATION:
     *  mcs_tbs_sum_sum_a/elapsed/1024/1024*1000 [bit/ms] --> /1024^2 --> [Mbit/ms] --> /1000 --> [Mbit/s]
     */
    graph_mcs_idx ->addData(timestamp,*mcs_idx / *nof_allocations);
    graph_throughput     ->addData(timestamp,*mcs_tbs / *nof_received_sf *1000/1024/1024);
    graph_prb   ->addData(timestamp,*l_prb  / *nof_received_sf);
  }
  else{

    graph_mcs_idx ->addData(timestamp, 0);
    graph_throughput     ->addData(timestamp, 0);
    graph_prb   ->addData(timestamp, 0);
  }
  *mcs_idx = 0;
  *mcs_tbs = 0;
  *l_prb   = 0;
  *nof_allocations = 0;
  *nof_received_sf = 0;

  // make timestamp axis range scroll with the data (at a constant range size of 10 sec):
  plot_mcs_idx->xAxis->setRange(timestamp, 10, Qt::AlignRight);
  plot_mcs_idx->yAxis->rescale(true);
  plot_mcs_idx->yAxis->scaleRange(OVERSCAN);
  plot_mcs_idx->yAxis->setRangeLower(0);

  plot_throughput->xAxis->setRange(timestamp, 10, Qt::AlignRight);
  plot_throughput->yAxis->rescale(true);
  plot_throughput->yAxis->scaleRange(OVERSCAN);
  plot_throughput->yAxis->setRangeLower(0);

  plot_prb    ->xAxis->setRange(timestamp, 10, Qt::AlignRight);
  plot_prb->yAxis->rescale(true);
  plot_prb->yAxis->scaleRange(OVERSCAN);
  plot_prb->yAxis->setRangeLower(0);

  perf_mutex.unlock();
}

void MainWindow::draw_plot_downlink(){
  double timestamp = QTime::currentTime().msecsSinceStartOfDay()*0.001; // day time in milliseconds
  perf_mutex.lock();

  mcs_tbs = &mcs_tbs_sum_downlink;
  l_prb = &l_prb_sum_downlink;
  mcs_idx = &mcs_idx_sum_downlink;
  nof_allocations = &nof_allocations_downlink;
  nof_received_sf = &nof_received_sf_downlink;
  graph_mcs_idx = plot_mcs_idx->graph(DOWNLINK);
  graph_throughput   =  plot_throughput->graph(DOWNLINK);
  graph_prb =  plot_prb->graph(DOWNLINK);

  if(*nof_received_sf != 0){    // Only plot if at least one subframe was received

    /*  CALCULATION:
     *  mcs_tbs_sum_sum_a/elapsed/1024/1024*1000 [bit/ms] --> /1024^2 --> [Mbit/ms] --> /1000 --> [Mbit/s]
     */
    graph_mcs_idx ->addData(timestamp,*mcs_idx / *nof_allocations);
    graph_throughput     ->addData(timestamp,*mcs_tbs / *nof_received_sf *1000/1024/1024);
    graph_prb   ->addData(timestamp,*l_prb  / *nof_received_sf);
  }
  else{

    graph_mcs_idx ->addData(timestamp, 0);
    graph_throughput     ->addData(timestamp, 0);
    graph_prb   ->addData(timestamp, 0);
  }

  *mcs_idx = 0;
  *mcs_tbs = 0;
  *l_prb   = 0;
  *nof_allocations = 0;
  *nof_received_sf = 0;

  // make timestamp axis range scroll with the data (at a constant range size of 10 sec):
  plot_mcs_idx->xAxis->setRange(timestamp, 10, Qt::AlignRight);
  plot_mcs_idx->yAxis->rescale(true);
  plot_mcs_idx->yAxis->scaleRange(OVERSCAN);
  plot_mcs_idx->yAxis->setRangeLower(0);

  plot_throughput->xAxis->setRange(timestamp, 10, Qt::AlignRight);
  plot_throughput->yAxis->rescale(true);
  plot_throughput->yAxis->scaleRange(OVERSCAN);
  plot_throughput->yAxis->setRangeLower(0);

  plot_prb    ->xAxis->setRange(timestamp, 10, Qt::AlignRight);
  plot_prb->yAxis->rescale(true);
  plot_prb->yAxis->scaleRange(OVERSCAN);
  plot_prb->yAxis->setRangeLower(0);

  perf_mutex.unlock();
}

void MainWindow::replot_perf(){
  perf_mutex.lock();
  plot_mcs_idx->replot();
  plot_throughput->replot();
  plot_prb->replot();
  plot_rnti_hist->replot();
  perf_mutex.unlock();
}
