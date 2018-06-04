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
#include <QtMath>
#include <QMessageBox>
#include <QGuiApplication>
#include <algorithm>

#define WIDTH 500
#define HEIGHT 500

MainWindow::MainWindow(const QString pathName, const double factor)
{
    listInfoImage = new QList <InfoImage>;

    createActions();
    createMenus();
    createMainWidget();

    fileBoxSetEnabled(false);
    factorBoxSetHidden(true);

    if (pathName != NULL) {
        if (loadFile(pathName)) {
            addItemComboBoxFileName(pathName);
            comboBoxLayer->setCurrentIndex(0);
            fileBoxSetEnabled(true);
            setMaxFactorSpinBox();

            /* Проверяем коэффициент сжатия на валидность */
            if (factor > 1 && factor < factorSpinBox->maximum()) {
                setFactorForImage(factor);
                comboBoxLayer->setCurrentIndex(5);
                factorSpinBox->setValue(factor);

                factorBoxSetHidden(false);
            }
        }
    }

}

MainWindow::~MainWindow()
{
    delete image;
    delete currentOriginalImage;
    delete listInfoImage;
}

/**
 * Определяет действия открытия файла и завершение программы.
 */
void MainWindow::createActions(void)
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

/**
 * Создает меню для QMainWindow
 * Добавляются действия открытия файла и завершение программы.
 */
void MainWindow::createMenus(void)
{
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *fileMenu = new QMenu(tr("&File"), this);

    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar->addMenu(fileMenu);

    setMenuBar(menuBar);
}

/**
 * Создает основной виджет в который
 * упаковываваются все остальные элементы
 * (QLabel(imageLabel) для изображения, QComboBox для файлов и уровня сжатия,
 * QDoubleSpinBox и QPushButton для установки своего коэффициента сжатия и др.)
 * и устанавливает данный виджет в QMainWindow
 */
void MainWindow::createMainWidget(void)
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

    layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(comboBoxFileName, SIGNAL(activated(int)), this, SLOT(selectFileName()));
    connect(okFactor, SIGNAL(clicked(bool)), this, SLOT(clickedOkFaktor()));
}

/**
 * Добавляет элементы в comboBoxLayer
 * определяющие уровень сжатия изображения
 */
void MainWindow::addItemComboBoxLayer(void)
{
    comboBoxLayer->addItem("0", 1);
    comboBoxLayer->addItem("1", 2);
    comboBoxLayer->addItem("2", 4);
    comboBoxLayer->addItem("3", 8);
    comboBoxLayer->addItem("4", 16);
    comboBoxLayer->addItem("own variant", 0);

    connect(comboBoxLayer, SIGNAL(activated(int)), this, SLOT(selectLayer()));
}

/**
 * Устанавлиет возможность взаимодествия с элементами
 * comboBoxFileName и comboBoxLayer
 *
 * @param enabled значение взаимодествия (true - активный / false - неактивный)
 */
void MainWindow::fileBoxSetEnabled(bool enabled)
{
    comboBoxFileName->setEnabled(enabled);
    comboBoxLayer->setEnabled(enabled);
}

/**
 * Устанавлиет скрытие элементов factorSpinBox и okFactor
 *
 * @param hidden значение скрытия (true - невидимый / false - видимый)
 */
void MainWindow::factorBoxSetHidden(bool hidden)
{
    factorSpinBox->setHidden(hidden);
    okFactor->setHidden(hidden);
}

/**
 * Загружает изображение и устанавливает в imageLabel
 * в currentOriginalImage сохраняется оригинал изображения
 *
 * @param pathName Полный путь к файлу изображения
 *
 * @retval true  Файл загружен
 * @retval false Файл не загружен
 */

bool MainWindow::loadFile(const QString &pathName)
{
    QImageReader reader(pathName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();

    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(pathName), reader.errorString()));
        return false;
    }

    imageLabel->setPixmap(QPixmap::fromImage(newImage));
    imageLabel->resize(newImage.size());

    *currentOriginalImage = newImage;

    sizeImage->setText(QString::number(newImage.width()) + "x" + QString::number(newImage.height()));

    return true;
}

/**
 * Добавление файла в listInfoImage и comboBoxFileName
 *
 * @param pathName полный путь к файлу изображения
 */
void MainWindow::addItemComboBoxFileName(const QString &pathName)
{
    int index = getIndexListInfoImage(pathName);

    /* Данный файл отсуствует в списке listInfoImage (сomboBoxFileName) */
    if (index == -1) {
        InfoImage newInfoImage;
        QFile file;
        QFileInfo fileInfo;

        newInfoImage.diagonal = calcDiagonalImage();
        newInfoImage.pathName.append(pathName);

        file.setFileName(pathName);
        fileInfo.setFile(file.fileName());

        listInfoImage->append(newInfoImage);

        sortItemsComboBox();

        index = getIndexListInfoImage(file.fileName());

        /* Добавляем новый итем и усатанвилем текущее отображаемое значение QComboBox */
        comboBoxFileName->insertItem(index, fileInfo.fileName(), file.fileName());
        comboBoxFileName->setCurrentIndex(index);
    } else {
        /* Файл ранее был загружен, выбираем из списка */
        comboBoxFileName->setCurrentIndex(index);
    }
}

/**
 * Сортируем содержимое списка listInfoImage по диагонали
 * Определяем индекс (i->index) для каждого listInfoImage в comboBoxFileName
 */
void MainWindow::sortItemsComboBox(void)
{
    std::stable_sort(listInfoImage->begin(), listInfoImage->end());

    int count = 0;

    for (QList <InfoImage>::iterator i = listInfoImage->begin(); i != listInfoImage->end(); i++)
        i->index = count++;
}

/**
 * Получает позицию вставки в comboBoxFileName
 *
 * @param pathName Полный путь к файлу изображения
 *
 * @retval i->index Позиция вставки в comboBoxFileName
 * @retval -1       Файл отсуствует в списке listInfoImage
 */
int MainWindow::getIndexListInfoImage(const QString &pathName)
{
    for (QList <InfoImage>::iterator i = listInfoImage->begin(); i != listInfoImage->end(); i++)
         if (QString::compare(i->pathName, pathName) == 0)
            return i->index;
    return -1;
}

/**
 * Устанавливает коэффициет сжатия изображения на текущее
 * оригинальное изображение currentOriginalImage
 *
 * @param factor Коэффициет сжатия изображения
 */
void MainWindow::setFactorForImage(double factor)
{
    const QImage newImage = currentOriginalImage->scaled(currentOriginalImage->width() / factor, currentOriginalImage->height() / factor, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    imageLabel->setPixmap(QPixmap::fromImage(newImage));
    sizeImage->setText(QString::number(newImage.width()) + "x" + QString::number(newImage.height()));
}

/**
 * Вычисяет диагональ изображения
 *
 * @retval qSqrt(d) Диагональ изображения currentOriginalImage
 */
int MainWindow::calcDiagonalImage(void)
{
    qreal h = currentOriginalImage->height();
    qreal w = currentOriginalImage->width();
    qreal d = (h * h) + (w * w);

    return qSqrt(d);
}

/**
 * Устанавливает максимальное допустимое значение
 * сжатия для выбранного изображения currentOriginalImage
 */
void MainWindow::setMaxFactorSpinBox(void)
{
    /* Выбран "свой вариант" уровня сжатия изображения */
    double maxFactor = 1;

    /* Определяем максимальный коэффициент сжатия */
    if (currentOriginalImage->width() <= currentOriginalImage->height())
        maxFactor = currentOriginalImage->width();
    else maxFactor = currentOriginalImage->height();

    factorSpinBox->setMaximum(maxFactor);
}

/**
 * Запускает диалоговое окно для открытия файла
 * с устанавлинными фильтрами JPEG image (*.jpeg *.jpg *.jpe)
 * и PNG image (*.png)
 */
void MainWindow::open()
{
    QFileDialog dialog;
    QStringList mimeTypeFilters;

    mimeTypeFilters << "image/jpeg"
                    << "image/png";

    dialog.setMimeTypeFilters(mimeTypeFilters);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}

    addItemComboBoxFileName(dialog.selectedFiles().first());

    comboBoxLayer->setCurrentIndex(0);

    fileBoxSetEnabled(true);
}

/**
 * Слот, запускается при выборе файла из QComboBox comboBoxFileName
 */
void MainWindow::selectFileName()
{
    if (!loadFile(comboBoxFileName->currentData().toString())) {
        /* Файл не найден */
        /* Удаляем из списка listInfoImage и запускаем рекурсивно selectFileName() */
        if (listInfoImage->count() > 0) {
            comboBoxFileName->removeItem(comboBoxFileName->currentIndex());
            listInfoImage->removeAt(comboBoxFileName->currentIndex());
            comboBoxFileName->setCurrentIndex(0);
            sortItemsComboBox();
            selectFileName();
        } else {
            comboBoxFileName->clear();
            fileBoxSetEnabled(false);
        }
    }

    comboBoxLayer->setCurrentIndex(0);

    factorBoxSetHidden(true);
}

/**
 * Слот, запускается при выборе уровня сжатия из QComboBox comboBoxLayer
 */
void MainWindow::selectLayer()
{
    double factor = comboBoxLayer->currentData().toFloat();

    if (factor == 0) {
        /* Устанавливаем коэффициент сжатия для оригинальнального размера изображения */
        factor = 1;

        setMaxFactorSpinBox();

        factorSpinBox->setValue(1.0);

        factorBoxSetHidden(false);
    } else factorBoxSetHidden(true);

    setFactorForImage(factor);
}

/**
 * Слот, запускается при клике на QPushButton okFactor
 * устанавливает коэффицент сжатия изображения (свой вариант)
 */
void MainWindow::clickedOkFaktor()
{
    setFactorForImage(factorSpinBox->value());
}
