#include "plot.h"
#include "ui_plot.h"

Plot::Plot(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Plot),

    /* Populate colors */
    line_colors{
      /* For channel data (gruvbox palette) */
      /* Light */
      QColor ("#fb4934"),
      QColor ("#b8bb26"),
      QColor ("#fabd2f"),
      QColor ("#83a598"),
      QColor ("#d3869b"),
      QColor ("#8ec07c"),
      QColor ("#fe8019"),
      /* Light */
      QColor ("#cc241d"),
      QColor ("#98971a"),
      QColor ("#d79921"),
      QColor ("#458588"),
      QColor ("#b16286"),
      QColor ("#689d6a"),
      QColor ("#d65d0e"),
    },
    gui_colors {
      /* Monochromatic for axes and ui */
      QColor (48,  47,  47,  255), /**<  0: qdark ui dark/background color */
      QColor (80,  80,  80,  255), /**<  1: qdark ui medium/grid color */
      QColor (170, 170, 170, 255), /**<  2: qdark ui light/text color */
      QColor (48,  47,  47,  200)  /**<  3: qdark ui dark/background color w/transparency */
    },

    /* Main vars */
    isPlotting(false),
    dataPointNumber (0),
    channelNumber(0)
{
    ui->setupUi(this);
    setWindowTitle("Plot");

    /* Setup plot area and connect controls slots */
    setupPlot();

    /* Wheel over plot when flagPlotting */
    connect (ui->plot, SIGNAL (mouseWheel (QWheelEvent*)), this, SLOT (slot_plot_mouseWheel(QWheelEvent*)));
    /* Slot for printing coordinates */
    connect (ui->plot, SIGNAL (mouseMove (QMouseEvent*)), this, SLOT (slot_plot_mouseMove(QMouseEvent*)));
    /* Channel selection */
    connect (ui->plot, SIGNAL(selectionChangedByUser()), this, SLOT(slot_plot_selectionChangedByUser()));
    connect (ui->plot, SIGNAL(legendDoubleClick (QCPLegend*, QCPAbstractLegendItem*, QMouseEvent*)), this, SLOT(slot_plot_legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*, QMouseEvent*)));
    /* Connect update timer to replot slot */
    connect (&timerUpdatePlot, SIGNAL (timeout()), this, SLOT (slot_timerUpdatePlotr_timeout()));


}

Plot::~Plot()
{
    delete ui;
}

/**
 * @brief Setup the plot area
 */
void Plot::setupPlot()
{
    //清除图标中所有项目
    ui->plot->clearItems();

    //设置背景颜色
    ui->plot->setBackground (gui_colors[0]);

    //用于高性能模式，去除抗锯齿， (see QCustomPlot real time example)
    ui->plot->setNotAntialiasedElements (QCP::aeAll);
    QFont font;
    font.setStyleStrategy (QFont::NoAntialias);
    ui->plot->legend->setFont (font);

    //See QCustomPlot examples / styled demo
    /* X Axis: Style */
    ui->plot->xAxis->grid()->setPen (QPen(gui_colors[2], 1, Qt::DotLine));
    ui->plot->xAxis->grid()->setSubGridPen (QPen(gui_colors[1], 1, Qt::DotLine));
    ui->plot->xAxis->grid()->setSubGridVisible (true);
    ui->plot->xAxis->setBasePen (QPen (gui_colors[2]));
    ui->plot->xAxis->setTickPen (QPen (gui_colors[2]));
    ui->plot->xAxis->setSubTickPen (QPen (gui_colors[2]));
    ui->plot->xAxis->setUpperEnding (QCPLineEnding::esSpikeArrow);
    ui->plot->xAxis->setTickLabelColor (gui_colors[2]);
    ui->plot->xAxis->setTickLabelFont (font);
    // 范围
    ui->plot->xAxis->setRange (dataPointNumber - ui->spinBoxXPoints->value(), dataPointNumber);
    // 设置坐标轴名称
    ui->plot->xAxis->setLabel("X");

    /* Y Axis */
    ui->plot->yAxis->grid()->setPen (QPen(gui_colors[2], 1, Qt::DotLine));
    ui->plot->yAxis->grid()->setSubGridPen (QPen(gui_colors[1], 1, Qt::DotLine));
    ui->plot->yAxis->grid()->setSubGridVisible (true);
    ui->plot->yAxis->setBasePen (QPen (gui_colors[2]));
    ui->plot->yAxis->setTickPen (QPen (gui_colors[2]));
    ui->plot->yAxis->setSubTickPen (QPen (gui_colors[2]));
    ui->plot->yAxis->setUpperEnding (QCPLineEnding::esSpikeArrow);
    ui->plot->yAxis->setTickLabelColor (gui_colors[2]);
    ui->plot->yAxis->setTickLabelFont (font);
    // 范围
    ui->plot->yAxis->setRange (ui->spinBoxYMin->value(), ui->spinBoxYMax->value());
    // 设置坐标轴名称
    ui->plot->yAxis->setLabel("Y");

    // 图表交互
    ui->plot->setInteraction (QCP::iRangeDrag, true);           //可单击拖拽
    ui->plot->setInteraction (QCP::iRangeZoom, true);           //可滚轮缩放
    ui->plot->setInteraction (QCP::iSelectPlottables, true);    //图表内容可选择
    ui->plot->setInteraction (QCP::iSelectLegend, true);        //图例可选
    ui->plot->axisRect()->setRangeDrag (Qt::Horizontal);        //水平拖拽
    ui->plot->axisRect()->setRangeZoom (Qt::Horizontal);        //水平缩放

    // 图例设置
    QFont legendFont;
    legendFont.setPointSize (9);
    ui->plot->legend->setVisible (true);
    ui->plot->legend->setFont (legendFont);
    ui->plot->legend->setBrush (gui_colors[3]);
    ui->plot->legend->setBorderPen (gui_colors[2]);
    // 图例位置，右上角
    ui->plot->axisRect()->insetLayout()->setInsetAlignment (0, Qt::AlignTop|Qt::AlignRight);
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for new data from serial port . Data is comming in QStringList and needs to be parsed
 * @param newData
 */
void Plot::onNewDataArrived(QByteArray baRecvData)
{
    static int num = 0;
    static int channelIndex = 0;

    if (isPlotting){
        num = baRecvData.size();

        for (int i = 0; i < num; i++){
            if(ui->plot->plottableCount() <= channelIndex){
                /* Add new channel data */
                ui->plot->addGraph();
                ui->plot->graph()->setPen (line_colors[channelNumber % CUSTOM_LINE_COLORS]);
                ui->plot->graph()->setName (QString("Channel %1").arg(channelNumber));
                if(ui->plot->legend->item(channelNumber)){
                    ui->plot->legend->item (channelNumber)->setTextColor (line_colors[channelNumber % CUSTOM_LINE_COLORS]);
                }
                ui->listWidgetChannels->addItem(ui->plot->graph()->name());
                ui->listWidgetChannels->item(channelIndex)->setForeground(QBrush(line_colors[channelNumber % CUSTOM_LINE_COLORS]));
                channelNumber++;
            }

            /* Add data to Graph */
            ui->plot->graph(channelIndex)->addData (dataPointNumber, baRecvData[channelIndex]);
            /* Increment data number and channel */
            channelIndex++;
        }

        dataPointNumber++;
        channelIndex = 0;
      }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief 1.使用滚轮缩放曲线
 *        2.将滚轮缩放值同步至spin
 * @param event
 */
void Plot::slot_plot_mouseWheel (QWheelEvent *event)
{
  QWheelEvent inverted_event = QWheelEvent(event->posF(), event->globalPosF(),
                                           -event->pixelDelta(), -event->angleDelta(),
                                           0, Qt::Vertical, event->buttons(), event->modifiers());
  QApplication::sendEvent (ui->spinBoxXPoints, &inverted_event);
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief 在状态栏中显示鼠标在图表中的XY坐标值
 * @param event
 */
void Plot::slot_plot_mouseMove(QMouseEvent *event)
{
    int xPos = int(ui->plot->xAxis->pixelToCoord(event->x()));
    int yPos = int(ui->plot->yAxis->pixelToCoord(event->y()));
    QString coordinates = tr("X: %1 Y: %2").arg(xPos).arg(yPos);
    ui->statusBar->showMessage(coordinates);
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Select both line and legend (channel)
 * @param plottable
 * @param event
 */
void Plot::slot_plot_selectionChangedByUser (void)
{
    /* synchronize selection of graphs with selection of corresponding legend items */
     for (int i = 0; i < ui->plot->graphCount(); i++)
       {
         QCPGraph *graph = ui->plot->graph(i);
         QCPPlottableLegendItem *item = ui->plot->legend->itemWithPlottable (graph);
         if (item->selected())
           {
             item->setSelected (true);
   //          graph->set (true);
           }
         else
           {
             item->setSelected (false);
     //        graph->setSelected (false);
           }
       }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Rename a graph by double clicking on its legend item
 * @param legend
 * @param item
 */
void Plot::slot_plot_legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)
{
    Q_UNUSED (legend)
    Q_UNUSED(event)
    /* Only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0) */
    if (item)
      {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        bool ok;
        QString newName = QInputDialog::getText (this, "Change channel name", "New name:", QLineEdit::Normal, plItem->plottable()->name(), &ok, Qt::Popup);
        if (ok)
          {
            plItem->plottable()->setName(newName);
            for(int i=0; i<ui->plot->graphCount(); i++)
            {
                ui->listWidgetChannels->item(i)->setText(ui->plot->graph(i)->name());
            }
            ui->plot->replot();
          }
      }
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Replot
 */
void Plot::slot_timerUpdatePlotr_timeout()
{
  ui->plot->xAxis->setRange (dataPointNumber - ui->spinBoxXPoints->value(), dataPointNumber);
  ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void Plot::on_checkBoxXTrackAixs_stateChanged(int arg1)
{
    qDebug()<<"on_checkBoxXTrackAixs_stateChanged";
}

void Plot::on_checkBoxYAutoScale_stateChanged(int arg1)
{
    qDebug()<<"on_checkBoxYAutoScale_stateChanged";
}

void Plot::on_pushButtonYAutoScale_clicked()
{
    ui->plot->yAxis->rescale(true);
    ui->spinBoxYMax->setValue(int(ui->plot->yAxis->range().upper) * 1.1);
    ui->spinBoxYMin->setValue(int(ui->plot->yAxis->range().lower) * 1.1);
}


/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


/**
 * @brief Spin box controls where the x position displayed
 * @param arg1
 */
void Plot::on_spinBoxXCurPos_valueChanged(int arg1)
{
    ui->plot->xAxis->setRangeLower(arg1);
    ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Spin box controls how many data points are collected and displayed
 * @param arg1
 */
void Plot::on_spinBoxXPoints_valueChanged(int arg1)
{
    ui->plot->xAxis->setRange (dataPointNumber - arg1, dataPointNumber);
    ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for spin box for plot minimum value on y axis
 * @param arg1
 */
void Plot::on_spinBoxYMin_valueChanged(int arg1)
{
    ui->plot->yAxis->setRangeLower (arg1);
    ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Slot for spin box for plot maximum value on y axis
 * @param arg1
 */
void Plot::on_spinBoxYMax_valueChanged(int arg1)
{
    ui->plot->yAxis->setRangeUpper (arg1);
    ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Spin box for changing the X Tick step
 * @param arg1
 */
void Plot::on_spinBoxXTicks_valueChanged(int arg1)
{
    ui->plot->xAxis->ticker()->setTickCount(arg1);
    ui->plot->replot();
}
/** ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @brief Spin box for changing the Y Tick step
 * @param arg1
 */
void Plot::on_spinBoxYTicks_valueChanged(int arg1)
{
    ui->plot->yAxis->ticker()->setTickCount(arg1);
    ui->plot->replot();
}


void Plot::on_pushButtonShowAllCurve_clicked()
{
    for(int i=0; i<ui->plot->graphCount(); i++)
    {
        ui->plot->graph(i)->setVisible(true);
        ui->listWidgetChannels->item(i)->setBackground(Qt::NoBrush);
    }
}

void Plot::on_pushButtonClearAllCurve_clicked()
{
    ui->plot->clearPlottables();
    ui->listWidgetChannels->clear();
    channelNumber = 0;
    dataPointNumber = 0;
    setupPlot();
    ui->plot->replot();
}


void Plot::on_pushButtonStartPlot_clicked()
{
    if(ui->pushButtonStartPlot->isChecked()){
        //启动绘图
        timerUpdatePlot.start(20);
        isPlotting = true;
        ui->pushButtonStartPlot->setText("停止绘图");
    }
    else{
        //停止绘图
        timerUpdatePlot.stop();
        isPlotting = false;
        ui->pushButtonStartPlot->setText("开始绘图");
    }
}

void Plot::on_listWidgetChannels_itemDoubleClicked(QListWidgetItem *item)
{
    int graphIdx = ui->listWidgetChannels->currentRow();

    if(ui->plot->graph(graphIdx)->visible()){
        ui->plot->graph(graphIdx)->setVisible(false);
        item->setBackground(Qt::black);
    }
    else{
        ui->plot->graph(graphIdx)->setVisible(true);
        item->setBackground(Qt::NoBrush);
    }
}

void Plot::on_listWidgetChannels_currentRowChanged(int currentRow)
{
    //曲线可见
    if(ui->plot->graph(currentRow)->visible())
        ui->checkBoxCurveVisible->setCheckState(Qt::Checked);
    else
        ui->checkBoxCurveVisible->setCheckState(Qt::Unchecked);

    //曲线加粗
    if(ui->plot->graph(currentRow)->pen().width() == 3)
        ui->checkBoxCurveBold->setCheckState(Qt::Checked);
    else
        ui->checkBoxCurveBold->setCheckState(Qt::Unchecked);

    //颜色设置
    //获取当前颜色
    QColor curColor = ui->plot->graph(currentRow)->pen().color();
    //设置选择框颜色
    ui->pushButtonCurveColor->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(curColor.name()));



}

void Plot::on_checkBoxCurveVisible_stateChanged(int arg1)
{
    int graphIdx = ui->listWidgetChannels->currentRow();
    if(graphIdx<0 || graphIdx>channelNumber)
        return;

    if(arg1 == Qt::Checked){
        ui->plot->graph(graphIdx)->setVisible(true);
        ui->listWidgetChannels->item(graphIdx)->setBackground(Qt::NoBrush);
    }
    else{
        ui->plot->graph(graphIdx)->setVisible(false);
        ui->listWidgetChannels->item(graphIdx)->setBackground(Qt::black);
    }
}

void Plot::on_checkBoxCurveBold_stateChanged(int arg1)
{
    int graphIdx = ui->listWidgetChannels->currentRow();
    if(graphIdx<0 || graphIdx>channelNumber)
        return;

    // 预先读取曲线的颜色
    QPen pen = ui->plot->graph(graphIdx)->pen();

    if(arg1 == Qt::Checked)
        pen.setWidth(3);
    else
        pen.setWidth(1);

    ui->plot->graph(graphIdx)->setPen(pen);
}

void Plot::on_pushButtonCurveColor_clicked()
{
    int graphIdx = ui->listWidgetChannels->currentRow();
    if(graphIdx<0 || graphIdx>channelNumber)
        return;

    // 获取当前颜色
    QColor curColor = ui->plot->graph(graphIdx)->pen().color();// 由curve曲线获得颜色
    // 以当前颜色打开调色板，父对象，标题，颜色对话框设置项（显示Alpha透明度通道）
    QColor color = QColorDialog::getColor(curColor, this,
                                     tr("颜色对话框"),
                                     QColorDialog::ShowAlphaChannel);
    // 判断返回的颜色是否合法。若点击x关闭颜色对话框，会返回QColor(Invalid)无效值，直接使用会导致变为黑色。
    if(color.isValid()){
        // 设置选择框颜色
        ui->pushButtonCurveColor->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(color.name()));
        // 设置曲线颜色
        QPen pen = ui->plot->graph(graphIdx)->pen();
        pen.setBrush(color);
        ui->plot->graph(graphIdx)->setPen(pen);
    }
}

void Plot::on_comboBoxCurveLineStyle_currentIndexChanged(int index)
{

}

void Plot::on_comboBoxCurveScatterStyle_currentIndexChanged(int index)
{

}




