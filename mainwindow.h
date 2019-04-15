#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"drawwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_fileToolButton_clicked();

    void on_clearButton_clicked();

    void on_forePainter_clicked();

    void on_backPainter_clicked();

    void on_startButton_clicked();

private:
    Ui::MainWindow *ui;
    QString filePath;
    DrawWidget* drawWidget;

    void init();
};

#endif // MAINWINDOW_H
