﻿/**
 * @file /src/main_window.cpp
 *
 * @brief Implementation for the qt gui.
 *
 * @date February 2011
 **/
/*****************************************************************************
** Includes
*****************************************************************************/

#include <QtGui>
#include <QMessageBox>
#include <iostream>
#include <QGraphicsItem>
#include "../include/parkingUI/main_window.hpp"

#define MAP_RE_WIDTH 1024
#define MAP_RE_HEIGHT 1024
#define MAP_GAP 2.61
#define MAP_GAP_ORI 2.56
//2.56

enum {PARKIN, PARKOUT};

/*****************************************************************************
** Namespaces
*****************************************************************************/

//#define

namespace parkingUI {

using namespace Qt;

/*****************************************************************************
** Implementation [MainWindow]
*****************************************************************************/

MainWindow::MainWindow(int argc, char** argv, QWidget *parent)
	: QMainWindow(parent)
	, qnode(argc,argv)
{
  ui.setupUi(this);
  qnode.init();
  this->parkingNum=qnode.parkingNum;
  this->EmptyList=qnode.EmptyList;

  QIcon appIcon("/home/lee-jaewon/catkin_ws/src/Autonomous-Multi-Robot-Parking-System/parkingUI/images/parking.png");
  this->setWindowIcon(appIcon);

  // You have to change user's name : hyedo->???
  //QString img_path = "/home/hyedo/map.pgm";
  QString img_path = QString::fromStdString(qnode.map_path);
  //QString img_path = "/home/lee-jaewon/catkin_ws/src/Autonomous-Multi-Robot-Parking-System/multi_parking_sys/map/mymap0527.pgm";
  QImage img(img_path);


  // Load Image
  map_ori = QPixmap::fromImage(img);


  // Rotate Image 90 degree (CCW)
  //map = map.transformed(QTransform().rotate(-90));

  //Get parking lot size
  #define PARKING_WIDTH qnode.PL_SIZE.at(0)/0.005    // (length(m) / resolution(0.005)
  #define PARKING_HEIGHT qnode.PL_SIZE.at(1)/0.005

  #define INOUT_WIDTH 0.64/0.005
  #define INOUT_HEIGHT 0.66/0.005

  //Get Map size
  #define MAP_WIDTH map_ori.width()
  #define MAP_HEIGHT map_ori.height()
  #define MAP_RESOLUTION qnode.map_resolution

  map = map_ori.scaled(MAP_RE_WIDTH,MAP_RE_HEIGHT);


  // Show map Image on graphicsView
  QGraphicsScene* scene = new QGraphicsScene;
  ui.graphicsView->setScene(scene);
  scene->addPixmap(map);


  //SIGNAL//
  qRegisterMetaType<nav_msgs::Odometry::ConstPtr>("nav_msgs::Odometry::ConstPtr");
  QObject::connect(&qnode,SIGNAL(RobotPose_SIGNAL(nav_msgs::Odometry::ConstPtr)), this, SLOT(RobotPose_SLOT(nav_msgs::Odometry::ConstPtr)));

  qRegisterMetaType<parking_msgs::parkingDone::ConstPtr>("parking_msgs::parkingDone::ConstPtr");
  QObject::connect(&qnode,SIGNAL(ParkingDone_SIGNAL(parking_msgs::parkingDone::ConstPtr)), this, SLOT(ParkingDone_SLOT(parking_msgs::parkingDone::ConstPtr)));

  qRegisterMetaType<parking_msgs::Sequence::ConstPtr>("parking_msgs::Sequence::ConstPtr");
  QObject::connect(&qnode,SIGNAL(Sequence_SIGNAL(parking_msgs::Sequence::ConstPtr)), this, SLOT(Sequence_SLOT(parking_msgs::Sequence::ConstPtr)));

  //INIT
  ParkingLotInit();
  InOutLotInit();


  //Sequence 표 설정
  ui.tableWidget->setColumnCount(6);
  ui.tableWidget->setColumnWidth(0,150);


  // TEST  //




//  ui.unparking_button->setStyleSheet("background-color: rgba(0,0,0,0);");
//  ui.unparking_button->setStyleSheet("color: rgba(0,0,0,0);");
//  ui.unparking_button->setStyleSheet("border: none;");
//  ui.unparking_button->setText("");
//  QDialog* d = new QDialog();
//  QVBoxLayout *layout = new QVBoxLayout();
//  QLabel* lab = new QLabel();
//  layout->alignment(centralWidget(lab));
//  layout->addWidget(lab);
//  layout.setAlignment(lab,center);
//  layout.set
}


  //----------- Methods -----------//



  // Make ParkingLot Labels and setStyle(location, size, color)
  void MainWindow::ParkingLotInit()
  {
    QGraphicsScene* scene = ui.graphicsView->scene();

    PL = new ClickableLabel[parkingNum];
    for(int i=0; i<parkingNum; i++)
    {
      QPixmap pixmap;
      PL[i].setPixmap(pixmap);

      PL[i].setIndex(i);
      PL[i].Pdata = qnode.ParkingData[i];

      QString text = "PL" + QString::number(i);
      PL[i].setText(text);

      std::vector<double> point = TransXY2(qnode.PL[i]);
      if(i==4 || i==5)
      {
        point.at(0) += 10;

      }
      point.at(1) -= 10;
      PL[i].setGeometry(point.at(0),point.at(1),PARKING_WIDTH,PARKING_HEIGHT);

      PL[i].setAlignment(Qt::AlignHCenter | Qt::AlignTop);

      if(PL[i].Pdata.GetParkingStatus() == "empty")
      {
        PL[i].SetUnClickable();
        SetLabelGreen(&PL[i]);
      }
      else
      {
        PL[i].SetClickable();
        SetLabelRed(&PL[i]);
      }
      scene->addWidget(&PL[i]);

      connect(&PL[i], &ClickableLabel::clicked, this, &MainWindow::onParkingLabelClicked);
    }
  }

  // Make InOutLot Labels and setStyle(location, size, color)
  void MainWindow::InOutLotInit()
  {
    QGraphicsScene* scene = ui.graphicsView->scene();

    InputLot = new QLabel;
    QPixmap pixmap;
    InputLot->setPixmap(pixmap);
    InputLot->setText("Input Lot");
    std::vector<double> point = TransXY(qnode.InputSpot);
    point.at(1) += 20;
    InputLot->setGeometry(point.at(0),point.at(1),INOUT_WIDTH,INOUT_HEIGHT);
    InputLot->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    SetLabelGray(InputLot);
    scene->addWidget(InputLot);

    OutputLot = new QLabel;
    OutputLot->setPixmap(pixmap);
    OutputLot->setText("Output Lot");
    point = TransXY(qnode.OutputSpot);
    OutputLot->setGeometry(point.at(0),point.at(1),INOUT_WIDTH,INOUT_HEIGHT);
    OutputLot->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    SetLabelGray(OutputLot);
    scene->addWidget(OutputLot);
  }

  // Callback robot's pose and plot
  void MainWindow::RobotPose_SLOT(nav_msgs::Odometry::ConstPtr odom)
  {
    float y = -odom->pose.pose.position.x;
    float x = -odom->pose.pose.position.y;

    // Location Scaling
    x = x * MAP_RE_WIDTH / (MAP_WIDTH * MAP_RESOLUTION);
    y = y * MAP_RE_HEIGHT / (MAP_HEIGHT * MAP_RESOLUTION);
    x += MAP_GAP / (MAP_WIDTH * MAP_RESOLUTION) * MAP_RE_WIDTH;
    y += MAP_GAP / (MAP_HEIGHT * MAP_RESOLUTION) * MAP_RE_HEIGHT;


    // Draw Point
    int frameNum = odom->header.frame_id[6]-'0';
    UpdateTargetPose(frameNum,x,y);
  }

  // Callback is current parking mission done
  void MainWindow::ParkingDone_SLOT(parking_msgs::parkingDone::ConstPtr data)
  {
    //if(!data->data) return;
    if(data->type == "ParkIn")
    {
      if(data->job == "Parker")
      {
        if(data->parkinglot<100)
        {
          PL[data->parkinglot].SetClickable();
          std::cout <<"data->parkinglot: "<<data->parkinglot<<"\n";

          //Parking된 공간이므로 이제 빨간색으로 변하게함
          SetLabelRed(parkingLotTarget_label);
          parkingLotTarget_info->SetStatus("full");

          //만약 사용자가 Dialog를 보고있다면 실시간으로 변동
          newDialog->SetInfo(parkingLotTarget_info);
          newDialog->ParkingIn_done();

          PL[parkingLotTarget_info->GetParkingLot()].Pdata=*parkingLotTarget_info;
          qnode.ParkingData[parkingLotTarget_info->GetParkingLot()] = *parkingLotTarget_info;
          qnode.WriteParkingData();

        }
      }
    }

    if(data->type == "ParkOut")
    {
      if(data->job == "ParkOuter")
      {
        if(data->parkinglot<100)
        {
          //주차공간list에서 현재 출자명령이 들어간 원소 추가
          EmptyList.push_back(data->parkinglot);


          //출차된 공간이므로 이제 빨간색으로 변하게함
          SetLabelGreen(parkoutLotTarget_label);
          parkoutLotTarget_info->SetStatus("empty");
          parkoutLotTarget_info->SetEmptyTime();
          parkoutLotTarget_info->SetCarNum("");
          //만약 사용자가 Dialog를 보고있다면 실시간으로 변동
          newDialog->SetInfo(parkoutLotTarget_info);
          newDialog->ParkingOut_done();
          PL[parkoutLotTarget_info->GetParkingLot()].Pdata=*parkoutLotTarget_info;

          qnode.ParkingData[parkoutLotTarget_info->GetParkingLot()] = *parkoutLotTarget_info;
          qnode.WriteParkingData();
//          ParkingLotInit();
        }
      }
    }
    ParkingLotInit();
  }

  // Callback is current parking mission done
  void MainWindow::ParkoutDone_SLOT(parking_msgs::parkingDone::ConstPtr data)
  {
//    if(!data->data) return;
//    if(data->type == "ParkOut")
//    {
//      if(data->job == "Parker")
//      {
//        if(data->parkinglot<100)
//        {
//          on_pushButton_ParkOut_clicked();
////          //출차된 공간이므로 이차제 빨간색으로 변하게함
////          SetLabelGreen(parkoutLotTarget_label);
////          parkoutLotTarget_info->SetStatus("empty");

////          //만약 사용자가 Dialog를 보고있다면 실시간으로 변동
////          newDialog->SetInfo(parkoutLotTarget_info);
////          newDialog->ParkingOut_done();

////          PL[parkoutLotTarget_info->GetParkingLot()].Pdata=*parkoutLotTarget_info;
////          qnode.ParkingData[parkoutLotTarget_info->GetParkingLot()] = *parkoutLotTarget_info;
////          qnode.WriteParkingData();

//        }
//      }
//    }
  }

  void MainWindow::Sequence_SLOT(parking_msgs::Sequence::ConstPtr seq)
  {
    //QScrollArea* scrollArea = new QScrollArea();
    while (ui.tableWidget->rowCount() > 0) {
        ui.tableWidget->removeRow(0);
    }

    std::cout<< "Sequence_SLOT In ";
    ui.tableWidget->setColumnCount(8);
    ui.tableWidget->setColumnWidth(0,100);
    ui.tableWidget->setColumnWidth(1,75);
    ui.tableWidget->setColumnWidth(2,75);
    ui.tableWidget->setColumnWidth(3,75);
    ui.tableWidget->setColumnWidth(4,75);
    ui.tableWidget->setColumnWidth(5,75);
    ui.tableWidget->setColumnWidth(6,75);
    ui.tableWidget->setColumnWidth(7,75);

    for(int i=0;i<seq->miniSequence.size();i++)
    {
      for(int j=0; j<seq->miniSequence[i].process.size(); j++)
      {
        ui.tableWidget->insertRow(ui.tableWidget->rowCount());

        QString strS = "Seq_";
        strS += QString::number(i);
        strS += " : ";
        strS += QString::fromStdString(seq->miniSequence[i].order);
        QTableWidgetItem* itemS = new QTableWidgetItem(strS);
        ui.tableWidget->setItem(ui.tableWidget->rowCount()-1,0,itemS);

        QString strR = "Robot : ";
        strR += QString::number(seq->miniSequence[i].process[j].robotNumber+1);
        QTableWidgetItem* itemR = new QTableWidgetItem(strR);
        ui.tableWidget->setItem(ui.tableWidget->rowCount()-1,1,itemR);

        QString strM = "Job : ";
        strM += QString::fromStdString(seq->miniSequence[i].process[j].job);
        QTableWidgetItem* itemM = new QTableWidgetItem(strM);
        ui.tableWidget->setItem(ui.tableWidget->rowCount()-1,2,itemM);

        if(seq->miniSequence[i].process[j].condition=="Done")
        {
          ParkingIn_Ready();
        }

        for(int k=0; k<seq->miniSequence[i].process[j].action.size(); k++)
        {
          QString strA = "";
          strA += QString::fromStdString(seq->miniSequence[i].process[j].action[k].action);
          QTableWidgetItem* itemA = new QTableWidgetItem(strA);
          if(seq->miniSequence[i].process[j].action[k].condition=="Working")
          {
            itemA->setBackground(Qt::cyan);
          }
          else if(seq->miniSequence[i].process[j].action[k].condition=="Done")
          {
            itemA->setBackground(Qt::red);
          }
          ui.tableWidget->setItem(ui.tableWidget->rowCount()-1,3+k,itemA);
        }
      }

    }
    std::cout<< "Sequence_SLOT Out ";
  }

  // Second Window에서 오는 주차 실행 시그널을 받는 SLOT
  void MainWindow::ParkingIn_SLOT(ParkingInfo* info)
  {
    bool isCar=true;

    parking_msgs::carNum service;
    service.request.signal=true;

    //service.response.carNum = "123가1234";
    //if(isCar)
    if(qnode.clientCarNum.call(service))
    {
      QMessageBox msgBox;
      QString str1 = "차량번호가 올바른지 확인해주세요";
      QString str2 = "차량번호 : ";
      str2 += QString::fromStdString(service.response.carNum);

      msgBox.setWindowTitle("차량번호 확인 메시지");
      msgBox.setText(str1);
      msgBox.setInformativeText(str2);
      msgBox.setStandardButtons(QMessageBox::No|QMessageBox::Yes);
      msgBox.setDefaultButton(QMessageBox::Yes);
      int ret = msgBox.exec();

      if(ret==QMessageBox::Yes)
      {

        parking_msgs::order srv;
        srv.request.order = PARKIN;
        srv.request.parkinglot = info->GetParkingLot();
        srv.request.carNum = service.response.carNum;

        if(qnode.client.call(srv))
        {
          std::cout<<"Successed Service!!"<<"\n";

          std::vector<double> target;
          int index = info->GetParkingLot();
          target = qnode.PL[index];

          info->SetCarNum(service.response.carNum);
          info->SetStartTime();
          info->SetStatus("parking");

          parkingLotTarget_label = &PL[index];
          parkingLotTarget_info = info;
          PL[index].Pdata = *info;
          parkingLotTarget = target;

          std::cout<<"Published!!! \n";
          newDialog->SetInfo(info);
          newDialog->ParkingIn_ing();

          SetLabelGray(&PL[index]);
          ParkingIn_NotReady();

          //비어있는 주차공간list에서 현재 추자명령이 들어간 원소 삭제
          EmptyList.remove(index);

          std::cout <<"index: "<<index<<"\n";
          PL[index].SetClickable();
        }
        else
        {
          std::cout<<"Failed Service!!"<<"\n";
        }
      }
      else if(ret==QMessageBox::No)
      {

      }
      else {

      }
    }
    else
    {
      QMessageBox msgBox;
      QString str1 = "차량번호가 인식되지 않았습니다.";
      QString str2 = "차량을 올바르게 위치해주세요.";

      msgBox.setIcon(QMessageBox::Information);
      msgBox.setWindowTitle("차량번호 확인 불가");
      msgBox.setText(str1);
      msgBox.setInformativeText(str2);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
    }
  }

  void MainWindow::ParkingOut_SLOT(ParkingInfo* info)
  {

    QMessageBox msgBox;
    QString str1 = "본인의 차량번호가 맞습니까?";
    QString str2 = "차량번호 : ";
    str2 += QString::fromStdString(info->GetCarNum());

    msgBox.setWindowTitle("차량번호 확인 메시지");
    msgBox.setText(str1);
    msgBox.setInformativeText(str2);
    msgBox.setStandardButtons(QMessageBox::No|QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();

    if(ret==QMessageBox::Yes)
    {
      parking_msgs::order srv;
      srv.request.order = PARKOUT;
      srv.request.parkinglot = info->GetParkingLot();
      srv.request.carNum = info->GetCarNum();

      if(qnode.client.call(srv))
      {
        std::cout<<"Successed Service!!"<<"\n";

        std::vector<double> target;
        int index = info->GetParkingLot();
        target = qnode.PL[index];

        info->SetCarNum(info->GetCarNum());
        info->SetStartTime();
        info->SetStatus("parkout");

        parkoutLotTarget_label = &PL[index];
        parkoutLotTarget_info = info;
        parkoutLotTarget = target;

        std::cout<<"Published!!! \n";
        newDialog->SetInfo(info);
        newDialog->ParkingOut_ing();

        SetLabelGray(&PL[index]);
        ParkingOut_NotReady();

//        //비어있는 주차공간list에서 현재 추자명령이 들어간 원소 삭제
//        EmptyList.push_back(index);
      }

    }
    else if(ret==QMessageBox::No)
    {

    }
    else {

    }
  }

  //Robot 위치 표시
  void MainWindow::UpdateTargetPose(int num, float x, float y)  //, QPixmap* pixmap)
  {
    // item-> data로 robot1,2,3 구분
    QGraphicsScene* scene = ui.graphicsView->scene();
    QList<QGraphicsItem*> items = scene->items();

    for (QGraphicsItem* item : items) {
      QVariant temp = num;
      if (item->data(0)==temp) {
        scene->removeItem(item);
        delete item;
      }
    }

    // QGraphicsEllipseItem 생성 및 추가
    int radius = 12;  // 반지름 설정
    QGraphicsEllipseItem* ellipseItem = new QGraphicsEllipseItem(x - radius, y - radius, radius * 2, radius * 2);
    QColor color;
    switch(num%5)
    {
    case 0:
      color = RED;
      break;
    case 1:
      color = GREEN;
      break;
    case 2:
      color = BLUE;
      break;
    case 3:
      color = YELLOW;
      break;
    case 4:
      color = CYAN;
      break;
    }
    ellipseItem->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    ellipseItem->setBrush(QBrush(color));
    QVariant value = num;
    ellipseItem->setData(0,value);
    scene->addItem(ellipseItem);

    //Label 생성 및 추가
    QString itemName = "Robot_"+QString::number(num);
    QGraphicsTextItem* textItem = new QGraphicsTextItem(itemName);

    //Label Font 설정
    QFont font;
    font.setPointSize(8);
    font.setBold(false);
    textItem->setFont(font);
    textItem->setPos(x-20,y+10);
    textItem->setData(0,value);
    scene->addItem(textItem);
  }

  void MainWindow::ParkingIn_Ready()
  {
    canPark = true;
    ui.pushButton_ParkIn->setEnabled(true);
    ui.pushButton_ParkIn->setText("주차하기");
  }

  void MainWindow::ParkingIn_NotReady()
  {
    canPark = false;
    ui.pushButton_ParkIn->setEnabled(false);
    ui.pushButton_ParkIn->setText("로봇 채우는중..");
  }

  void MainWindow::ParkingOut_Ready()
  {
    canPark = true;
    //ui.pushButton_ParkOut->setEnabled(true);
    ui.pushButton_ParkOut->setText("출차하기");
  }

  void MainWindow::ParkingOut_NotReady()
  {
    canPark = false;
    //ui.pushButton_ParkOut->setEnabled(false);
    ui.pushButton_ParkOut->setText("출차중..");
  }

  void MainWindow::openNewWindow(ParkingInfo* info)
  {
    delete newDialog;
    newDialog = new NewDialog(info,this);
    //newDialog->exec();
    newDialog->show();
    connect(newDialog, &NewDialog::ParkingIn_SIGNAL, this, &MainWindow::ParkingIn_SLOT);
    connect(newDialog, &NewDialog::ParkingOut_SIGNAL, this, &MainWindow::ParkingOut_SLOT);
  }

  void MainWindow::openNewWindow(int num)
  {
    delete newDialog;
    newDialog = new NewDialog(&qnode.ParkingData[num],this);
    newDialog->show();
    connect(newDialog, &NewDialog::ParkingIn_SIGNAL, this, &MainWindow::ParkingIn_SLOT);
    connect(newDialog, &NewDialog::ParkingOut_SIGNAL, this, &MainWindow::ParkingOut_SLOT);
  }

  MainWindow::~MainWindow() {}


}  // namespace parkingUI

std::vector<double> parkingUI::MainWindow::TransXY(std::vector<double> point)
{
  std::vector<double> result = point;

  result.at(0) = (-point.at(1)+MAP_GAP_ORI)*MAP_RE_WIDTH/(MAP_WIDTH * MAP_RESOLUTION);
  result.at(0) -= PARKING_WIDTH/2;

  result.at(1) = (-point.at(0)+MAP_GAP_ORI)*MAP_RE_HEIGHT/(MAP_HEIGHT * MAP_RESOLUTION);
  result.at(1) -= PARKING_HEIGHT/2;

  return result;
}

std::vector<double> parkingUI::MainWindow::TransXY2(std::vector<double> point)
{
  std::vector<double> result = point;

  result.at(0) = (-point.at(1)+MAP_GAP)*MAP_RE_WIDTH/(MAP_WIDTH * MAP_RESOLUTION);
  result.at(0) -= PARKING_WIDTH/2;

  result.at(1) = (-point.at(0)+MAP_GAP)*MAP_RE_HEIGHT/(MAP_HEIGHT * MAP_RESOLUTION);
  result.at(1) -= PARKING_HEIGHT/2;

  return result;
}


void parkingUI::MainWindow::SetLabelGreen(QLabel* l)
{
  l->setStyleSheet("background-color: rgba(204,255,204,100);");
}

void parkingUI::MainWindow::SetLabelRed(QLabel* l)
{
  l->setStyleSheet("background-color: rgba(255,204,204,180);");
}

void parkingUI::MainWindow::SetLabelGray(QLabel* l)
{
  l->setStyleSheet("background-color: rgba(220,220,220,180);");
}

void parkingUI::MainWindow::onParkingLabelClicked(ParkingInfo* info)
{
  std::cout<<info->GetParkingStatus()<<"\n";
  if(!(info->GetParkingStatus()=="empty"))
  {
    //선택한 주차공간 채우기
    openNewWindow(info);
  }

}

void parkingUI::MainWindow::on_pushButton_ParkIn_clicked()
{
  //가장 가까운 주차공간부터 채우기
  if(!EmptyList.empty())
  {
    openNewWindow(&qnode.ParkingData[*std::min_element(EmptyList.begin(), EmptyList.end())]);
  }
  else
  {
    QMessageBox msgBox;
    QString str1 = "더이상 주차 가능한 공간이 없습니다";

    msgBox.setWindowTitle("주차불가!");
    msgBox.setText(str1);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Ok)
    {
        // 확인 버튼이 클릭되었을 때 실행할 코드 작성
    }
  }
}

void parkingUI::MainWindow::on_pushButton_ParkOut_clicked()
{
//  int y=0;
//  //임시 !!!
//  std::cout<<y<<"\n"; y++;
//  //출차된 공간이므로 이제 빨간색으로 변하게함
//  SetLabelGreen(parkoutLotTarget_label);
//  std::cout<<y<<"\n"; y++;
//  parkoutLotTarget_info->SetStatus("empty");
//  std::cout<<y<<"\n"; y++;
//  parkoutLotTarget_info->SetEmptyTime();
//  std::cout<<y<<"\n"; y++;
//  parkoutLotTarget_info->SetCarNum("");
//  std::cout<<y<<"\n"; y++;
//  //만약 사용자가 Dialog를 보고있다면 실시간으로 변동
//  newDialog->SetInfo(parkoutLotTarget_info);
//  std::cout<<y<<"\n"; y++;
//  newDialog->ParkingOut_done();
//  std::cout<<y<<"\n"; y++;
//  PL[parkoutLotTarget_info->GetParkingLot()].Pdata=*parkoutLotTarget_info;
//  std::cout<<y<<"\n"; y++;

//  qnode.ParkingData[parkoutLotTarget_info->GetParkingLot()] = *parkoutLotTarget_info;
//  std::cout<<y<<"\n"; y++;

//  qnode.WriteParkingData();
//  std::cout<<y<<"\n"; y++;
//  ParkingLotInit();
//  std::cout<<y<<"\n"; y++;
}
