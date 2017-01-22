#pragma once

#include <QMainWindow>
#include <thread>

namespace NVideoInput
{
class CVideoCapture;
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
   void fillConfig();

protected:
    //void loop(CVideoCapture& videoCapture, MainWindow& window);
private slots:
    void onRunLoop();
    void onContinue();
    void onStopLoop();
    void onReadConfig();
    void aboutThis();
    void aboutQt();

public:
    Ui::MainWindow *ui;

    std::shared_ptr<NVideoInput::CVideoCapture> mpVideoCapture;
    std::thread mLoopThread;
    bool mStopFlag;

protected:
    virtual void closeEvent(QCloseEvent*) override;
};
