#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class FormRespect;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_File_triggered();

private:
    Ui::MainWindow *ui;
    FormRespect* plot;

    void setPixel(QImage *image, double i, int x, int y);
};

#endif // MAINWINDOW_H
