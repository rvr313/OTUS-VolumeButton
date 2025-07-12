#include "widget.h"
#include "volumebutton.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    VolumeButton *vb = new VolumeButton(this);
    ui->verticalLayout->addWidget(vb);
}

Widget::~Widget()
{
    delete ui;
}
