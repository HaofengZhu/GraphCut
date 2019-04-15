#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFileDialog>
#include<QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->fileEdit->setReadOnly(true);
    drawWidget=new DrawWidget(720,720);
    drawWidget->setLineWidth(5);
    ui->verticalLayout->insertWidget(2,drawWidget);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_fileToolButton_clicked()
{
    filePath=QFileDialog::getOpenFileName(this,tr("select file"),QDir::currentPath(),tr("Image (*.jpg *.png)"));
    if(filePath.isEmpty())
    {
       return;
    }
    ui->fileEdit->setText(filePath);

    QImage img(filePath);
    if(img.isNull())
    {
        qDebug()<<"Can not open file.";
        return;
    }
    drawWidget->setImage(img);
}

void MainWindow::on_clearButton_clicked()
{
    drawWidget->clear();
    init();
}

void MainWindow::on_forePainter_clicked()
{
    drawWidget->setForeOrBack(true);
    ui->forePainter->setDisabled(true);
    ui->backPainter->setDisabled(false);
}

void MainWindow::on_backPainter_clicked()
{
    drawWidget->setForeOrBack(false);
    ui->forePainter->setDisabled(false);
    ui->backPainter->setDisabled(true);
}

void MainWindow::on_startButton_clicked()
{
    drawWidget->compute();
}

void MainWindow::init()
{
    ui->forePainter->setDisabled(true);
    ui->backPainter->setDisabled(false);
    drawWidget->setForeOrBack(true);
}
