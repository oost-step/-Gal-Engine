#pragma once

#include <QtWidgets/QWidget>
#include "ui_galengineqt.h"

class galengineqt : public QWidget
{
    Q_OBJECT

public:
    galengineqt(QWidget *parent = nullptr);
    ~galengineqt();

private:
    Ui::galengineqtClass ui;
};