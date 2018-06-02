#include "mainwindow.h"
#include <QMenuBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QScrollBar>
#include <QScrollArea>
#include <QImageReader>
#include <algorithm>

#include <QDebug>

#define WIDTH 500
#define HEIGHT 500

MainWindow::MainWindow()
{
    createActions();
    createMenus();
    createMainWidget();

    initWidgets();
}

MainWindow::~MainWindow()
{
    delete image;
    delete currentOriginalImage;
}


void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::createMenus()
{
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = new QMenu(tr("&File"), this);

    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar->addMenu(fileMenu);

    setMenuBar(menuBar);
}

void MainWindow::addItemComboBoxLayer()
{
    comboBoxLayer->addItem("0", 1);
    comboBoxLayer->addItem("1", 2);
    comboBoxLayer->addItem("2", 4);
    comboBoxLayer->addItem("3", 8);
    comboBoxLayer->addItem("4", 16);
    comboBoxLayer->addItem("own variant", 0);

    connect(comboBoxLayer, SIGNAL(activated(int)), this, SLOT(selectLayer()));
}

void MainWindow::createMainWidget()
{
    QWidget *mainWidget    = new QWidget(this);
    QVBoxLayout *mainBox   = new QVBoxLayout;
    QHBoxLayout *topBox    = new QHBoxLayout;
    QHBoxLayout *faktorBox = new QHBoxLayout;
    QScrollArea *scrArea   = new QScrollArea;
    QLabel *fileLabel      = new QLabel(tr("File:"), this);
    QLabel *layerLabel     = new QLabel(tr("Layer:"), this);
    QLabel *sizeLabel      = new QLabel(tr("Size:"), this);
    QHBoxLayout *fileBox   = new QHBoxLayout;
    QHBoxLayout *layerBox  = new QHBoxLayout;
    QHBoxLayout *sizeBox   = new QHBoxLayout;
    sizeImage              = new QLabel(tr(""), this);
    comboBoxFileName       = new QComboBox(this);
    comboBoxLayer          = new QComboBox(this);
    imageLabel             = new QLabel(this);
    image                  = new QImage(WIDTH, HEIGHT, QImage::Format_RGB888);
    currentOriginalImage   = new QImage();
    factorSpinBox          = new QDoubleSpinBox(this);
    okFactor               = new QPushButton("Set the value", this);

    image->fill(QColor(Qt::white).rgb());
    imageLabel->setPixmap(QPixmap::fromImage(*image));

    fileBox->addWidget(fileLabel);
    fileBox->addWidget(comboBoxFileName);

    layerBox->addWidget(layerLabel);
    layerBox->addWidget(comboBoxLayer);

    addItemComboBoxLayer();

    sizeBox->addWidget(sizeLabel);
    sizeBox->addWidget(sizeImage);

    topBox->addLayout(fileBox);
    topBox->addLayout(layerBox);
    topBox->addLayout(sizeBox);

    faktorBox->addWidget(factorSpinBox);
    faktorBox->addWidget(okFactor);

    mainBox->addLayout(topBox);
    mainBox->addLayout(faktorBox);
    scrArea->setWidget(imageLabel);
    mainBox->addWidget(scrArea);
    mainWidget->setLayout(mainBox);

    mainBox->setSizeConstraint(QLayout::SetFixedSize);

    scrArea->setMinimumSize(WIDTH + 2, HEIGHT + 2);
    scrArea->setMaximumSize(WIDTH + 2, HEIGHT + 2);

    imageLabel->setScaledContents(true);

    factorSpinBox->setMinimum(1.0);
    factorSpinBox->setSingleStep(0.05);

    setCentralWidget(mainWidget);

    setWindowTitle(tr("Pyramid Image Viewer"));

    connect(comboBoxFileName, SIGNAL(activated(int)), this, SLOT(selectFileName()));
    connect(okFactor, SIGNAL(clicked(bool)), this, SLOT(clickedOkFaktor()));
}

void MainWindow::initWidgets()
{
    fileBoxSetEnabled(false);
    factorBoxSetHidden(true);
}

void MainWindow::factorBoxSetHidden(bool hidden)
{
    factorSpinBox->setHidden(hidden);
    okFactor->setHidden(hidden);
}

void MainWindow::fileBoxSetEnabled(bool enabled)
{
    comboBoxFileName->setEnabled(enabled);
    comboBoxLayer->setEnabled(enabled);
}

void MainWindow::open()
{
    QFileDialog dialog;
    QStringList mimeTypeFilters;
    mimeTypeFilters << "image/jpeg" // will show "JPEG image (*.jpeg *.jpg *.jpe)
                    << "image/png"; // will show "PNG image (*.png)"

    dialog.setMimeTypeFilters(mimeTypeFilters);

    while (dialog.exec() == QDialog::Accepted && !setLoadFile(dialog.selectedFiles().first())) {}

    map.insert(dialog.selectedFiles().first(), currentOriginalImage->height());

    sortItemsComboBox(dialog.selectedFiles().first());

    comboBoxLayer->setCurrentIndex(0);

    fileBoxSetEnabled(true);
}

void MainWindow::selectFileName()
{
    if (!setLoadFile(comboBoxFileName->currentData().toString())) {
        //File not found
        map.remove(comboBoxFileName->currentData().toString());

        if (map.size() > 0) {
            comboBoxFileName->setCurrentIndex(0);
            sortItemsComboBox(comboBoxFileName->currentData().toString());
            selectFileName();
        } else {
            comboBoxFileName->clear();
            fileBoxSetEnabled(false);
        }
    }

    comboBoxLayer->setCurrentIndex(0);

    factorBoxSetHidden(true);
}

void MainWindow::selectLayer()
{
    double factor = comboBoxLayer->currentData().toFloat();

    if (factor == 0) {
        double maxFactor = 1;
        //own variant
        factor = 1;

        if (currentOriginalImage->width() <= currentOriginalImage->height())
            maxFactor = currentOriginalImage->width();
        else maxFactor = currentOriginalImage->height();

        factorSpinBox->setMaximum(maxFactor);

        factorBoxSetHidden(false);
    } else factorBoxSetHidden(true);

    setFactorForImage(factor);
}

void MainWindow::setFactorForImage(double factor)
{
    const QImage newImage = currentOriginalImage->scaled(currentOriginalImage->width() / factor, currentOriginalImage->height() / factor, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    imageLabel->setPixmap(QPixmap::fromImage(newImage));
    sizeImage->setText(QString::number(newImage.width()) + "x" + QString::number(newImage.height()));
}

void MainWindow::clickedOkFaktor()
{
    setFactorForImage(factorSpinBox->value());
}

bool MainWindow::setLoadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    if (newImage.isNull())
        return false;

    imageLabel->setPixmap(QPixmap::fromImage(newImage));
    imageLabel->resize(newImage.size());

    *currentOriginalImage = newImage;

    sizeImage->setText(QString::number(newImage.width()) + "x" + QString::number(newImage.height()));

    return true;
}

void MainWindow::sortItemsComboBox(const QString &fileName)
{
    QList <int> list = map.values();
    //Sort by the size of the diagonal of the image
    std::stable_sort(list.begin(), list.end());

    QMap <QString, int> tmpMap = map;
    QList <int>::iterator i;

    QFile file;
    QFileInfo fileInfo;

    comboBoxFileName->clear();

    for (i = list.begin(); i != list.end(); ++i) {
        file.setFileName(tmpMap.key(*i));
        fileInfo.setFile(file.fileName());
        comboBoxFileName->addItem(fileInfo.fileName(), tmpMap.key(*i));

        //Set the name of the open file in QComboBox
        if (fileName == tmpMap.key(*i))
            comboBoxFileName->setCurrentText(fileInfo.fileName());

        //To avoid the case when there are images with equal diagonal
        tmpMap.remove(tmpMap.key(*i));
    }
}
