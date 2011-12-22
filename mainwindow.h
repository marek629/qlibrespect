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
    FormRespect* ploter;
};

#endif // MAINWINDOW_H
