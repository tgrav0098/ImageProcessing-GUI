#include "widget.h"
#include "ui_widget.h"
#include <iostream>
#include <QDebug>
#include "qthread.h"

// methods
void modifyImageContrast (QImage * image1,QImage * image2, int position);
void mofidyImageBrightness(QImage * image1,QImage * image2, int position);
void get_max_min(QImage *image1,int & min, int& max);

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Widget)
{
    ui->setupUi(this);
    image = NULL;
    image_modified = NULL;
    socket = new QUdpSocket();
}

Widget::~Widget()
{
    delete ui;
}

// select image
void Widget::on_btn_image_clicked()
{
    if (image != NULL) {delete image;}
    image = new QImage();
    QString file_name = QFileDialog::getOpenFileName(this,tr("Open BMPfile"),"",tr("Images(*.bmp)"));
    if(!file_name.isEmpty()){
        bool val = image->load(file_name);
        if(val){
            (*image) = image->scaled(300,300); // scale image to match label
            // image_modified will be the one that displays the modified image
            image_modified = new QImage();
            *image_modified = *image;
            ui->lbl_image->setPixmap(QPixmap::fromImage(*image)); // display image
            ui->lbl_image_2->setPixmap(QPixmap::fromImage(*image)); // display image on label that will
            // keep original image for refererence
        }
        // reset sliders' positions : done everytime an image is chosen
        (ui->contrastSlider)->setValue(50);
        (ui->brightnessSlider)->setValue(99);
    }
    else {
        //error: tell the user that image cannot be open
        QMessageBox::information(this,tr("ERROR!"), tr("Image not valid."));
    }
}

// save image
void Widget::on_save_image_clicked()
{
    if(image_modified != NULL){
        QString fileName = QFileDialog::getSaveFileName(this,tr("Save BMP file"),"",tr("Image(*.bmp)"));
        if(!fileName.isEmpty()){
            image_modified->save(fileName);
        }
        else{
            //error no Image to save
            QMessageBox::information(this,tr("ERROR!"), tr("No image to save."));
        }
    }
}
// function to modify both contrast and brightness of loaded image
// this function will be used everytime a slider is moved, so that the contrast/ brightness can be adjusted
// accordingly

void Widget::modify_contrast_brightness()
{
    QImage * temp_image = new QImage(); *temp_image = *image;
    // first modify the contrast of the image based the value of the contrast slider
    modifyImageContrast(image,temp_image,ui->contrastSlider->value());
    // then modify the image brighntess
    mofidyImageBrightness(temp_image,image_modified,ui->brightnessSlider->value());
    // display modified image
    ui->lbl_image->setPixmap(QPixmap::fromImage(*image_modified));
    delete temp_image; temp_image=NULL;
}

// adjust contrast via contrast stretching
// method to adjust contrast: takes image1 and a discrete position with range [0,100] that determines the
// contrast
// modifies image2
void modifyImageContrast(QImage * image1,QImage * image2, int position){
    int s1,s2;
    int t1, t2;
    int compress_num,new_position;
    double compress_fact,stretch_s1,stretch_s2;
    //* compute distribution of the pixels of the image (max, min)
    get_max_min(image1,s1,s2);
    //* case 1: compress brightness histogram (lower contrast) (up to 40 % of distribution range
    //*compressed on each side)
    if(position < 50){
        //** determine compression number: how much the brightness histogram will be compressed
        //** on each side
        compress_num = (s2-s1) * 0.4;
        //** determine 50 levels of compression: when position is 50: no compression
        //** when position is 0 : very high compression
        compress_fact = compress_num / 50.0;
        new_position= 50 - position; // position normalized
        //** determine to what values [t1, t2] to compress based on slider position
        t1 = s1 + compress_fact * new_position;
        t2= s2 - compress_fact * new_position;
    }
    //* case 2: stretch brightness histogram (higher contrast) (up to [0,255] distribution)
    else{
        //** determine 50 levels of stretch on each side of brightness histogram
        stretch_s1 = s1/50.0; // stretch from current value of min down to 0
        stretch_s2 = (255-s2)/50.0; // stretch from current value of max up to 255
        new_position = position - 50; // position normalized
        // determine to what values [t1, t2] to stretch based on slider position
        t1 = s1 - (stretch_s1 * new_position);
        t2 = s2 + (stretch_s2 * new_position);

    }
    //* compress or stretch brightness histogram: go from distribution [s1, s2] -> [t1, t2]
    //* linear stretching formula: f(pixel) = a*pixel + b
    //** calc a and b for formula
    double a = (1.0*(t2-t1) )/ (s2-s1);
    double b= t1 - (s1*a);
    int new_pix;
    //** modify image
    for (int i=0;i<image1->width();i++)
    {
        for (int j=0;j<image1->height();j++)
        {
            // apply formula f(pixel) = pixel*a + b
            new_pix = (image1->pixelIndex(i,j))*a +b;
            if (new_pix <= 255) { image2->setPixel(i,j,new_pix);}
            //just in case a pixel value ends up beyond 255 (not expected)
            else {qDebug()<< "Warning!!!";}
        }
    }
}

//brightness function
// method to adjust image brightness by multiplying all pixels by a constant, 200 discrete brightness levels
void mofidyImageBrightness(QImage * image1,QImage * image2, int position){

    // determine brightness multiplier that would modify each pixel
    // case 1, brightness decrease: position < 100, map position to a multiplier value [0.02,1]
    double multiplier;
    int value;
    if (position < 99){
        multiplier = position / 99.0;
    }
    // case2, brightness increase: position > 100, map position to a multiplier value [1,10]
    else{
        multiplier = (position-99)*0.09 +1;
    }
    //multiply each pixel by multiplier to adjust brightness
    for (int i=0;i<image1->width();i++)
    {
        for (int j=0;j<image1->height();j++)
        {
            value = image1->pixelIndex(i,j)*multiplier;
            // values that end up > 255, are set to 255
            if (value>255){
                value = 255;
            }
            image2->setPixel(i,j,value);
        }
    }
}

// function to find min and max pixel value of an image
// needed for contrast function
void get_max_min(QImage *image1,int & min, int& max) {
    min = image1->pixelIndex(0,0); max = image1->pixelIndex(0,0);
    for (int i=0;i<image1->width();i++)
    {
        for (int j=0;j<image1->height();j++)
        {
            if (image1->pixelIndex(i,j) < min){
                min = image1->pixelIndex(i,j);
            }
            else if (image1->pixelIndex(i,j) > max){
                max = image1->pixelIndex(i,j);
            }
        }
    }
}

// sliders
// both sliders do the same thing: when their value changes, both the contrast and brightness of the original image are modified based
// on the two sliders' positions
// brightness slider
void Widget::on_brightnessSlider_valueChanged(int position)
{
    if (image != NULL){
        // modify both contrast and brightness
        modify_contrast_brightness();
    }
    else {
        // error : image not loaded: do nothing
    }
    /*QByteArry function below was added to implement the use of the sliders across the network*/
    QByteArray sendVal;
        sendVal.reserve(4);
       sendVal.resize(2);
       sendVal.setNum(position);
       if (position < 99)
       {
           sendVal = sendVal.setNum(position).rightJustified(2,'0');
       }
       qInfo() << sendVal;
       socket->writeDatagram(sendVal,QHostAddress("192.168.215.166"),80);
}
// contrast slider


void Widget::on_contrastSlider_valueChanged(int position)
{
    if (image != NULL){
        // modify both contrast and brightness
        modify_contrast_brightness();
    } else{
        // error no Image loaded: do nothing
    }
}
