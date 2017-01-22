#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <thread>

#include "Image/CImageBuffer.hpp"
#include "Image/motionAlgorithms/CFaceDetect.hpp"
#include "Image/motionAlgorithms/CMoveDetect.hpp"
#include "Image/videoInput/CVideoCapture.hpp"
#include "common/CAlertNotification.hpp"
#include "common/CMainFunctions.hpp"
#include "common/CConfig.hpp"
#include "common/CMemoryUssage.hpp"
#include "common/CVersion.hpp"
#include "logging/Log.hpp"


#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "Net/CServerFactory.hpp"
#include "ImageServer/CImageServer.hpp"
#include "Net/servers/CTCPServer.hpp"
#include "client/CImageReceiverWorker.hpp"
#include "client/CClient.hpp"

#include "common/CConnectionSettings.hpp"

#include "QtNetwork/QTcpSocket"

#include "Net/AES/CAES.hpp"
#include "Net/servers/CTCPServer.hpp"

void loop(MainWindow* window)
{
    CConfig& config = CConfig::getInstance();
    QTcpSocket socket;

#if 1
//    NNet::CTCPServer* serv = new  NNet::CTCPServer(conf, nullptr, nullptr);
    std::vector<std::shared_ptr<NNet::IServer>> servers;
    QByteArray ba;

//    NNet::CAES::crypt(ba, nullptr);

    NNet::CNetConnectionConfig conf;
    std::vector<NNet::pClient> clientList;
    std::vector<NNet::pWorker> workerList;
//    CClient::addClientToList(clientList, workerList, 1, "");
//    NNet::CTCPServer serv(conf, *config.mCryptionConfig, nullptr);

    LOG_WARN << config.mConnectionSettings->mTCPPort;
//    new CImageServer(10000, NNet::TypeProtocol::TCP);

//    CImageServer::addServer(config.mConnectionSettings->mTCPPort, TypeProtocol::TCP, servers);

#endif

    size_t bufferId = 0;
    CAlertNotification alert(bufferId);

    std::vector<std::shared_ptr<NAlgorithms::IAlertAlgorithm>> algorithms;
    CMainFunctions::addAlgorithm(algorithms);
    LOG_WARN << "in loop" << !window->mStopFlag;

    while (true && !window->mStopFlag)
    {
        LOG_WARN << "in loop 2" << !window->mStopFlag;

        window->mpVideoCapture->processNextFrame(bufferId);

        const CImageBuffer& buff = CImageBuffer::getBufferInstance();
        cv::Mat mask;
//        CMainFunctions::doAlgo(alert, algorithms, mask);
        if (!CImageBuffer::getBufferInstance().empty(0))
        {
            LOG_DEBUG_EVERY_COUNT(100, processNextGuiFrame, "process next frame");
            auto matPtr =buff.getLast(bufferId);
            if (matPtr)
            {
                window->setWindowTitle(QString::fromStdString(matPtr->getDateTimeWithMS())
                                       + "; img id:" + QString::number(matPtr->getId())
                                       + "; currentBufferSize:" + QString::number(CImageBuffer::getBufferInstance().size(0))
                                       + "; mem: " + QString::number(CMemoryUssage::processMemoryUssage())
                                       + "; memMax: " + QString::number(CMemoryUssage::processMaxMemoryUssage())
                                       + "; " + QString::fromStdString(CVersion::getApplicationInfo()));
                if (window->ui->actionIsShow->isChecked())
                {
                    auto currentWidget = window->ui->tabWidget->currentWidget();
                    const cv::Mat mat = matPtr->getMat();
                    if ( currentWidget == window->ui->tab_input)
                    {
                        LOG_WARN << 4 << mat.cols << mat.rows <<  mat.step << mat.empty();
                        QImage imgIn= QImage((uchar*) mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
                        imgIn = imgIn.rgbSwapped();
                        window->ui->imageLabel->setPixmap(QPixmap::fromImage(imgIn));
                    }
                    else if (currentWidget == window->ui->tab_mask)
                    {
                        QImage imgMask= QImage((uchar*) mask.data, mask.cols, mask.rows, mask.step, QImage::Format_RGB888);
                        imgMask = imgMask.rgbSwapped();
                        window->ui->maskLabel->setPixmap(QPixmap::fromImage(imgMask));
                    }
                }
            }
            else
            {
                LOG_DEBUG << "empty" << CImageBuffer::getBufferInstance().size(0);
            }
        }
        CMainFunctions::sleep(0.1); //TODO:
    }

    window->ui->logOutput->append("exit loop");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mStopFlag(false)
{
    ui->setupUi(this);
    ui->actionStop->setEnabled(false);
    ui->actionContinue->setEnabled(false);
    fillConfig();

    connect(ui->actionRun, SIGNAL(triggered()), this, SLOT(onRunLoop()));
    connect(ui->actionContinue, SIGNAL(triggered()), this, SLOT(onContinue()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(onStopLoop()));
    connect(ui->actionReReadConfig, SIGNAL(triggered()), this, SLOT(onReadConfig()));

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(aboutThis()));
    connect(ui->actionAbout_Qt,SIGNAL(triggered()),this,SLOT(aboutQt()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onReadConfig()
{
    fillConfig();
}

void MainWindow::aboutThis()
{
    QMessageBox::information(this,
                             "About",
                             "Develop by Moskalenko Yuriy<br><br>"
                             + QString::fromStdString(CVersion::getApplicationInfo())
                             + "<br><br>2016");
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::closeEvent(QCloseEvent*)
{
    onStopLoop();
}

void MainWindow::fillConfig()
{
    #define READ_NUMBER(prefix, name) do { \
    CConfig& config = CConfig::getInstance(); \
    QLineEdit* lineEdit = ui->tabConfig->findChild<QLineEdit*>(#prefix "." #name"_Edit"); \
    if (lineEdit != nullptr) { \
       lineEdit->setText(QString::number(config.m##prefix.m##name)); \
    }} while(0)

#define READ_STRING(prefix, name) do { \
    CConfig& config = CConfig::getInstance(); \
    QLineEdit* lineEdit = ui->tabConfig->findChild<QLineEdit*>(#prefix "." #name"_Edit"); \
    if (lineEdit != nullptr) { \
      lineEdit->setText(QString::fromStdString(config.m##prefix.m##name)); \
    } \
    } while(0)

#define UPDATE_FONT(field, newFontSize) do { \
    QFont font = field->font();              \
    font.setPointSize(newFontSize);          \
    field->setFont(font);                    \
    } while (0)

#define CREATE_UI_FIELDS(prefix, name) do { \
    if (ui->tabConfig->findChild<QHBoxLayout*>(#prefix "." #name"_layout") == nullptr) { \
       QHBoxLayout* horizontalLayout = new QHBoxLayout(); \
       horizontalLayout = new QHBoxLayout(); \
       horizontalLayout->setObjectName(#prefix "." #name"_layout"); \
       vertConfig->addLayout(horizontalLayout);\
       \
       QLabel *label = new QLabel(ui->tabConfig);   \
       UPDATE_FONT(label, CConfig::getInstance().mGuiConfig.mLabelPointSize); \
       label->setObjectName(#prefix "." #name"_Label"); \
       QLineEdit* lineEdit = new QLineEdit(ui->tabConfig); \
       lineEdit->setObjectName(#prefix "." #name"_Edit"); \
       horizontalLayout->addWidget(label); \
       horizontalLayout->addWidget(lineEdit); \
       QString labelStr = #prefix "." #name; \
       labelStr.replace("Config", ""); \
       label->setText(labelStr); \
       UPDATE_FONT(lineEdit, CConfig::getInstance().mGuiConfig.mLineEditPointSize); \
    }} while (0)

#define SET_NUMBER_FROM_CONFIG(prefix, name) do { \
    CConfig& config = CConfig::getInstance(); \
    QLineEdit* lineEdit = ui->tabConfig->findChild<QLineEdit*>(#prefix "." #name"_Edit"); \
    if (lineEdit != nullptr) { \
       bool ok; \
       config.m##prefix.m##name = lineEdit->text().toInt(&ok); \
       if (!ok) { \
           ui->logOutput->append("fail "); \
       } \
    } \
    } while (0)

#define SET_STRING_FROM_CONFIG(prefix, name) do { \
    CConfig& config = CConfig::getInstance(); \
    QLineEdit* lineEdit = ui->tabConfig->findChild<QLineEdit*>(#prefix "." #name"_Edit"); \
    if (lineEdit != nullptr) { \
        config.m##prefix.m##name = lineEdit->text().toStdString(); \
    } \
    } while (0)

#define READ_STRING_FROM_CONFIG(prefix, name) do { \
    SET_STRING_FROM_CONFIG(prefix, name); \
    CREATE_UI_FIELDS(prefix, name); \
    READ_STRING(prefix, name);      \
    } while (0)

#define READ_NUMBER_FROM_CONFIG(prefix, name) do { \
    SET_NUMBER_FROM_CONFIG(prefix, name); \
    CREATE_UI_FIELDS(prefix, name); \
    READ_NUMBER(prefix, name); \
    } while (0)


//    auto vertConfig = ui->verticalConfig1;


//    READ_NUMBER_FROM_CONFIG(InputConfig, CameraId);
//    READ_NUMBER_FROM_CONFIG(InputConfig, BufferSize);
//    READ_NUMBER_FROM_CONFIG(InputConfig, ResizeScaleFactor);
//    READ_NUMBER_FROM_CONFIG(InputConfig, FrameWidth);
//    READ_NUMBER_FROM_CONFIG(InputConfig, FrameHeight);

//    READ_NUMBER_FROM_CONFIG(AlertNotificationConfig, IsWriteFullMaskToVideo);
//    READ_STRING_FROM_CONFIG(AlertNotificationConfig, SaveVideoPath);
//    READ_NUMBER_FROM_CONFIG(AlertNotificationConfig, FirstTimeSkip);

//    READ_NUMBER_FROM_CONFIG(FaceDetectConfig, IsRun);
//    READ_NUMBER_FROM_CONFIG(FaceDetectConfig, SkipFrames);

//    vertConfig = ui->verticalConfig2;

//    READ_NUMBER_FROM_CONFIG(MoveDetectConfig, AreaMinSize);
//    READ_NUMBER_FROM_CONFIG(MoveDetectConfig, AreaMaxSize);
//    READ_NUMBER_FROM_CONFIG(MoveDetectConfig, MinTrashHold);
//    READ_NUMBER_FROM_CONFIG(MoveDetectConfig, IsRun);
//    READ_NUMBER_FROM_CONFIG(MoveDetectConfig, AdoptAreaSizeToFPSCoeficient);
//    READ_NUMBER_FROM_CONFIG(MoveDetectConfig, FromWhichFPSAdoptArea);
//    READ_NUMBER_FROM_CONFIG(MoveDetectConfig, MintLightFromWhichAdopt);
//    READ_NUMBER_FROM_CONFIG(MoveDetectConfig, AdoptTrashHoldToLightCoeficient);

//    vertConfig = ui->verticalConfig3;

//    READ_NUMBER_FROM_CONFIG(MoveDetectBackgroundSubtractorConfig, AreaMinSize);
//    READ_NUMBER_FROM_CONFIG(MoveDetectBackgroundSubtractorConfig, AreaMaxSize);
//    READ_NUMBER_FROM_CONFIG(MoveDetectBackgroundSubtractorConfig, MinTrashHold);
//    READ_NUMBER_FROM_CONFIG(MoveDetectBackgroundSubtractorConfig, IsRun);
//    READ_NUMBER_FROM_CONFIG(MoveDetectBackgroundSubtractorConfig, AdoptAreaSizeToFPSCoeficient);
//    READ_NUMBER_FROM_CONFIG(MoveDetectBackgroundSubtractorConfig, FromWhichFPSAdoptArea);
//    READ_NUMBER_FROM_CONFIG(MoveDetectBackgroundSubtractorConfig, MintLightFromWhichAdopt);
//    READ_NUMBER_FROM_CONFIG(MoveDetectBackgroundSubtractorConfig, AdoptTrashHoldToLightCoeficient);
}

void MainWindow::onRunLoop()
{
    fillConfig();
    mpVideoCapture.reset();

    try
    {
        mpVideoCapture = std::shared_ptr<NVideoInput::CVideoCapture>(
                    new NVideoInput::CVideoCapture(*CConfig::getInstance().mInputConfig));
        onContinue();
    }
    catch (const std::runtime_error& exc)
    {
        QMessageBox::critical(this,
                              "CVideoCapture create fail",
                              QString::fromStdString(exc.what()));
    }
}

void MainWindow::onContinue()
{
    mStopFlag = false;

    mLoopThread = std::thread(loop, this);
    ui->actionRun->setEnabled(false);
    ui->actionContinue->setEnabled(false);
    ui->actionStop->setEnabled(true);
}

void MainWindow::onStopLoop()
{
    mStopFlag = true;
    if (mLoopThread.joinable())
    {
        mLoopThread.join();
        ui->actionRun->setEnabled(true);
        ui->actionContinue->setEnabled(true);
        ui->actionStop->setEnabled(false);
    }
}
