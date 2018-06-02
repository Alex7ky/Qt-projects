#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class QFileDialog;
class QLabel;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private slots:
    void open();
    void save();
    void selectFileName();
    void selectLayer();

private:
    void createActions();
    void createMenus();
    void createComboBoxLayer();
    bool setLoadFile(const QString &);
    void sortItemsComboBox(const QString &);

    QMap <QString, int> map;
    QComboBox *comboBoxFileName;
    QComboBox *comboBoxLayer;
    QLabel *imageLabel;
    QImage *image;
    QImage *currentOriginalImage;
    QLabel *sizeImage;
    QAction *openAct;
    QAction *saveAct;
    QAction *exitAct;
};

#endif // MAINWINDOW_H
