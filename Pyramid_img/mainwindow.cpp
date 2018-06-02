#include "mainwindow.h"
#include <QMenuBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
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
    QWidget *mainWidget   = new QWidget(this);
    QVBoxLayout *mainBox  = new QVBoxLayout;
    QHBoxLayout *topBox   = new QHBoxLayout;
    QScrollArea *scrArea  = new QScrollArea;
    QLabel *fileLabel     = new QLabel(tr("File:"), this);
    QLabel *layerLabel    = new QLabel(tr("Layer:"), this);
    QLabel *sizeLabel     = new QLabel(tr("Size:"), this);
    QHBoxLayout *fileBox  = new QHBoxLayout;
    QHBoxLayout *layerBox = new QHBoxLayout;
    QHBoxLayout *sizeBox  = new QHBoxLayout;
    sizeImage             = new QLabel(tr(""), this);
    comboBoxFileName      = new QComboBox;
    comboBoxLayer         = new QComboBox;
    imageLabel            = new QLabel;
    image                 = new QImage(WIDTH, HEIGHT, QImage::Format_RGB888);
    currentOriginalImage  = new QImage;

    //imageLabel->setBackgroundRole(QPalette::Base);
    //imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    image->fill(QColor(Qt::white).rgb());

    imageLabel->setPixmap(QPixmap::fromImage(*image));

    createActions();
    createMenus();
    createComboBoxLayer();

    fileBox->addWidget(fileLabel);
    fileBox->addWidget(comboBoxFileName);

    layerBox->addWidget(layerLabel);
    layerBox->addWidget(comboBoxLayer);

    sizeBox->addWidget(sizeLabel);
    sizeBox->addWidget(sizeImage);

    topBox->addLayout(fileBox);
    topBox->addLayout(layerBox);
    topBox->addLayout(sizeBox);

    mainBox->addLayout(topBox);
    scrArea->setWidget(imageLabel);
    mainBox->addWidget(scrArea);
    mainWidget->setLayout(mainBox);

    qDebug() << scrArea->size();

    scrArea->setMinimumSize(WIDTH + 2, HEIGHT + 2);
    scrArea->setMaximumSize(WIDTH + 2, HEIGHT + 2);

    comboBoxFileName->setEnabled(false);
    comboBoxLayer->setEnabled(false);

    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setCentralWidget(mainWidget);

    setWindowTitle(tr("Pyramid Image Viewer"));

    connect(comboBoxFileName, SIGNAL(activated(int)), this, SLOT(selectFileName()));
}

MainWindow::~MainWindow()
{
    delete comboBoxFileName;
    delete comboBoxLayer;
    delete image;
}

void MainWindow::open()
{
    QFileDialog dialog;

    dialog.setNameFilter(tr("Images (*.jpg)"));

    while (dialog.exec() == QDialog::Accepted && !setLoadFile(dialog.selectedFiles().first())) {}

    map.insert(dialog.selectedFiles().first(), currentOriginalImage->height());

    sortItemsComboBox(dialog.selectedFiles().first());

    comboBoxLayer->setEnabled(true);
    comboBoxFileName->setEnabled(true);
}

void MainWindow::save()
{
    qDebug() << "Press 'save'";
}

void MainWindow::selectFileName()
{
    if (setLoadFile(comboBoxFileName->currentData().toString()))
        qDebug() << "File Loaded";
    comboBoxLayer->setCurrentIndex(0);
}

void MainWindow::selectLayer()
{
    int factor = comboBoxLayer->currentData().toInt();

    if (factor != 0) {
        const QImage newImage = currentOriginalImage->scaled(currentOriginalImage->width() / factor, currentOriginalImage->height() / factor, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        imageLabel->setPixmap(QPixmap::fromImage(newImage));
        sizeImage->setText(QString::number(newImage.width()) + "x" + QString::number(newImage.height()));
    }
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
        qDebug() << tmpMap.key(*i) << " value = " << *i;
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

void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);

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
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar->addMenu(fileMenu);

    setMenuBar(menuBar);
}

void MainWindow::createComboBoxLayer()
{
    comboBoxLayer->addItem("0", 1);
    comboBoxLayer->addItem("1", 2);
    comboBoxLayer->addItem("2", 4);
    comboBoxLayer->addItem("3", 8);
    comboBoxLayer->addItem("4", 16);
    comboBoxLayer->addItem("own variant", 0);

    connect(comboBoxLayer, SIGNAL(activated(int)), this, SLOT(selectLayer()));
}
