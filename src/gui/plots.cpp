#include "mainwindow.h"
#include "qcustomplot/qcustomplot.h"
#include "plots.h"

void MainWindow::setupPlot(PlotsType_t plottype, QCustomPlot *plot){

    if(plottype == RNTI_HIST){
        plot->addGraph();
        plot->graph(0)->setPen(QPen(QColor(40,110,255)));
        plot->xAxis->setRange(0,65536);
        plot->yAxis->setRange(0,10);
        plot->axisRect()->setupFullAxesBox();

        connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
        connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));

    }

    if(plottype == RB_OCCUPATION || plottype == CELL_THROUGHPUT){

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
    }

}

void MainWindow::addData(PlotsType_t plottype, QCustomPlot *plot, const ScanLineLegacy *data){


    if(plottype == RNTI_HIST){
        static QTime time(QTime::currentTime());
        double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
        static double lastPointKey = 0;

        if (key - lastPointKey > 0.1){ // at most add point every 100ms

            plot->removeGraph(0);
            plot->addGraph();
            plot->graph(0)->setPen(QPen(QColor(40, 110, 255)));

            for(int i = 0; i <= 65536; i++){
                plot->graph(0)->addData(i,data->rnti_hist[i]);
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

