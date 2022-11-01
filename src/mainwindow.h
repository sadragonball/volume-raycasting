#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:

    void load_volume(const QString& path);

    void on_stepLength_valueChanged(double arg1);

    void on_loadVolume_clicked();

    void on_threshold_spinbox_valueChanged(double arg1);

    void on_threshold_slider_valueChanged(int value);

    void on_mode_currentTextChanged(const QString &arg1);

    void on_background_clicked();

private:
    Ui::MainWindow *ui;
};
