#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QFileDialog;
class QLabel;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private slots:
    void open();
    void selectFileName();
    void selectLayer();
    void clickedOkFaktor();

private:
    void createActions(void);
    void createMenus(void);
    void createMainWidget(void);
    void addItemComboBoxLayer(void);
    void fileBoxSetEnabled(bool);
    void factorBoxSetHidden(bool);
    bool loadFile(const QString &);
    void addItemComboBoxFileName(const QString &);
    void sortItemsComboBox(void);
    int getIndexListInfoImage(const QString &);
    void setFactorForImage(double);
    int calcDiagonalImage(void);

    QComboBox *comboBoxFileName;
    QComboBox *comboBoxLayer;
    QLabel *imageLabel;
    QImage *image;
    QImage *currentOriginalImage;
    QLabel *sizeImage;
    QDoubleSpinBox *factorSpinBox;
    QPushButton *okFactor;
    QAction *openAct;
    QAction *exitAct;

    class InfoImage {
        public:
            int index;
            QString pathName;
            qreal diagonal;

            bool operator < (const InfoImage &ptr) const {
                return this->diagonal < ptr.diagonal;
            }
    };

    QList <InfoImage> *listInfoImage;
};

#endif // MAINWINDOW_H
