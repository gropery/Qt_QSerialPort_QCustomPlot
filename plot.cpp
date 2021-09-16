#include "plot.h"
#include "ui_plot.h"

Plot::Plot(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Plot)
{
    ui->setupUi(this);
}

Plot::~Plot()
{
    delete ui;
}
