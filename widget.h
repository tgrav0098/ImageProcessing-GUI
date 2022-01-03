#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QFileDialog>
#include <QMessageBox>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private slots:
// loads image
void on_btn_image_clicked();
// saves modified image
void on_save_image_clicked();
// changes brightness/ contrast
void on_brightnessSlider_valueChanged(int value);
// changes brightness/ contrast
void on_contrastSlider_valueChanged(int value);
// changes brightness/ contrast
void modify_contrast_brightness();


private:
    Ui::Widget *ui;
    QImage *image;
    QImage *image_modified;
    QUdpSocket *socket;
};
#endif // WIDGET_H
