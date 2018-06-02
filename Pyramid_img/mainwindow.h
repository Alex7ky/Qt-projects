#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

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
    void createActions();
    void createMenus();
    void createMainWidget();
    void addItemComboBoxLayer();
    void initWidgets();
    void fileBoxSetEnabled(bool);
    void factorBoxSetHidden(bool);
    bool setLoadFile(const QString &);
    void sortItemsComboBox(const QString &);
    void setFactorForImage(double);
    int calcDiagonalImage();

    QMap <QString, qreal> map;
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
};

#endif // MAINWINDOW_H
