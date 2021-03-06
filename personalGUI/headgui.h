#ifndef HEADGUI_H
#define HEADGUI_H

#include <wiringPiI2C.h>

#include <QMainWindow>
#include <QLabel>
#include <QGridLayout>
#include <QResizeEvent>
#include <QTimer>
#include <QPixmap>
#include <QStackedWidget>
#include <QWidget>

#include <../Lepton/LeptonThread.h>
#include <../Netzwerk/User/User.h>
#include <../I2C/DaTa/DaTa.h>

class QLabel;
class QPixmap;
class QGridLayout;

class HeadGUI : public QMainWindow

{
    Q_OBJECT

public:
    HeadGUI(user *recentUser, DaTa *recentData, QWidget *parent = 0);
    ~HeadGUI();
    void newDataFromHeadquater();
    void newDataFromArduino();

	std::mutex abc;

public slots:
    void updateImage(unsigned short *, int, int, unsigned char *);

signals:
    void certifyPersonaeSignal();

    void upSignal();
    void downSignal();
    void rightSignal();
    void leftSignal();
    void backSignal();
    void certifySignal();

    void messageRecivedSignal(QString sendMessage);
    void updateBraceletSignal(bool valueUp, bool valueDown, bool valueRight, bool valueLeft, bool valueBack, bool valueCertify);

    void updateTempHeadSignal(int recentTemp);
    void updateTempFootSignal(int recentTemp);
    void updateCOHeadSignal(int recentCO);
    void updateCOFootSignal(int recentCO);
    void updateMotionControlSignal(bool recentMotion);

    void changingLightSignal();
    void coosingStatusSignal();
    void changeScreenModeSignal();

    void newDataFromHeadquaterSignal();
    void newDataFromArduinoSignal();
    void newValuesForHeadquater();

private slots:
    void certifyPersonae();

    void up();
    void down();
    void right();
    void left();
    void back();
    void certify();

    void coosingStatus();
    void updateBracelet(bool valueUp, bool valueDown, bool valueRight, bool valueLeft, bool valueBack, bool valueCertify);

    void updateTempHead(int recentTemp);
    void updateTempFoot(int recentTemp);
    void updateCOHead(int recentCOHead);
    void updateCOFoot(int recentCOFoot);
    void updateMotionControl(bool recentMotion);

    void messageRecived(QString sendMessage);
    void changingLight();
    void changeScreenMode();

    void sortingValuesForHeadquater();
    void sortingNewDataFromHeadquater();
    void sortingNewDataFromArduino();
    void readingSensors();

private:
    //other
    void createConnections();
    void defauftValues();

    QGridLayout *layout;
    QWidget *mainWidget;

    user *networkUser;
    DaTa *sensorData;


    //menu/communication
    void createCommunication();

    QLabel *Status;
    QLabel *Messages;

    int vertical;
    int horizontal;
    int NumberDiffMenues = 3;
    int NumberDiffValues[5] = {3,2,1,3,4};
    int timeNoMotion;

    bool emergencyPossible;
    bool SendEmergency;
    bool otherSignals;
    bool answerPossible;

    QString recivedMessage;
    QString messageAnswers[3] = {" ","angenommen","nicht verfügbar"};
    QString Stati[5]={"Status1","Status2","Status3","Status4","Emergency"};


    //biometric data
    void createBiometric();

    QTimer *timerReadingSensors;
    QLabel *Personae;
    QLabel *TempHead;
    QLabel *TempFoot;
    QLabel *Light;
    QLabel *COHead;
    QLabel *COFoot;

    QString lightSwitch[2] = {"Off","On"};
    QString Name [5]= {"Tobias Finke","Aron Haselhoff","Timon Gronotte","Vicky Bietenbeck","Beke Pierick"};
    QString unitTemp =" °C";
    QString unitCO = " ppm";

    //for ImageFull Screen
    void createImageOverlay();

    QGridLayout *ComplexLayout;
    QWidget *NormalScreen;
    QStackedWidget *Overlay;
    QLabel *IRPictureFullScreen;
    QLabel *IRPictureFullScreenOverlay;

    QString PictureFullScreen[] = {"Normal","Vollbild","Outline","Voll Outl"};

    //for Camera
    void createIRPicture();

    QLabel *IRPicture;
    QImage *rgbImage;
    QVector<unsigned short> *rawData;
    LeptonThread *thread;

    int ImageWidth = 320;
    int ImageHeight = 240;
    int FrameWidth = 160;
    int FrameHeight = 120;
    int PacketWidth = 80;
    int PacketWords = PacketWidth + 2;
    int RowPacketWords = 2*PacketWords;
    int PacketBytes = 2*PacketWords;
    int FrameWords = FrameWidth*FrameHeight;
    int SegmentHeight = FrameHeight/4;
    int SegmentPackets = 60;
    unsigned short rawMin;
    unsigned short rawMax;

    //Data
    int ID;
    bool newConfirmedID;
    bool IDisSet;


    int recentStatus;
    bool updatedStatus;

    int recentAnswer;
    bool answeredMessage;

    int recentTempHead;
    int recentTempFoot;
    int recentCOHead;
    int recentCOFoot;

    bool updatedTempHead;
    bool updatedTempFoot;
    bool updatedCOHead;
    bool updatedCOFoot;

    bool lightOn;
    bool IRPictureMaxSize;
	bool outline = false;

    unsigned char *resultPicture;
    bool updatedPicture;
};

#endif // HEADGUI_H
