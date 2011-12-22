#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "formrespect.h"
#include "librespect.h"
#include "wavefile.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ploter = ui->widget;

    QSettings settings("respect");
    ploter->setColorMin(settings.value("colorMin","#000000").toString());
    ploter->setColorMax(settings.value("colorMax","#52FFFF").toString());
    ploter->setColorOverflow(settings.value("colorOverflow","#DDCC47").toString());

    connect(ploter->view(), SIGNAL(selectedItemChanged(QString)),
            ui->plainTextEdit, SLOT(setPlainText(QString)));

    connect(ploter->view(), SIGNAL(selectedItemChanged(QString)),
            ui->plainTextEdit, SLOT(setPlainText(QString)));

    connect(ui->plainTextEdit, SIGNAL(textChanged()),
            ploter->view(), SLOT(selectedTextChanged()));

    connect(ploter, SIGNAL(setStatusMessage(QString)),
            statusBar(), SLOT(showMessage(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QDir::homePath(),
                                                    tr("WAV files")+" (*.wav)");
    ploter->setupView(filePath);
}
