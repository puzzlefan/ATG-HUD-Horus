#include "headgui.h"

#include <iostream>
#include <wiringPiI2C.h>

#include <../Lepton/LeptonThread.h>
#include <../Netzwerk/User/User.h>
#include <../I2C/DaTa/DaTa.h>
#include <../I2C/I2C.h>

#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QGridLayout>
#include <QResizeEvent>
#include <QTimer>
#include <QStackedWidget>
#include <QWidget>

HeadGUI::HeadGUI(user *recentUser, DaTa *recentData, QWidget *parent)
    : QMainWindow(parent)
  {
    mainWidget = new QWidget;
    NormalScreen = new QWidget;
    layout = new QGridLayout;
    ComplexLayout = new QGridLayout;
    Overlay = new QStackedWidget;

    mainWidget->resize(2*ImageWidth,2*ImageHeight);//432;240?!
    NormalScreen->resize(2*ImageWidth,2*ImageHeight);
    mainWidget->showFullScreen();

    defauftValues();
    createConnections();
    createIRPicture();
    createBiometric();
    createCommunication();

    NormalScreen->setLayout(layout);

    createImageOverlay();

    mainWidget->setWindowTitle("PersonalGUI Helmet");
    mainWidget->setLayout(ComplexLayout);
    mainWidget->show();

    networkUser = recentUser;
    sensorData = recentData;

    timerReadingSensors = new QTimer(this);
    connect(timerReadingSensors,SIGNAL(timeout()),this,SLOT(readingSensors()));
    timerReadingSensors->start(1000);

    emit certifyPersonaeSignal();
}

void HeadGUI::createBiometric()
{
    Personae = new QLabel;
    layout->addWidget(Personae,0,2,1,3, Qt::AlignCenter);

    Light = new QLabel;
    layout->addWidget(Light,1,4,1,1, Qt::AlignCenter);
    Light->setText(lightSwitch[0]);
    Light->setStyleSheet("QLabel{background-color : lightgrey;}");

    IRPictureFullScreen = new QLabel;
    layout->addWidget(IRPictureFullScreen,2,4,1,1, Qt::AlignCenter);
    IRPictureFullScreen->setText(PictureFullScreen[0]);

    TempHead = new QLabel;
    layout->addWidget(TempHead,1,2,1,1, Qt::AlignCenter);
    TempHead->setText(unitTemp);

    TempFoot = new QLabel;
    layout->addWidget(TempFoot,1,3,1,1, Qt::AlignCenter);
    TempFoot->setText(unitTemp);

    COHead = new QLabel;
    layout->addWidget(COHead,2,2,1,1, Qt::AlignCenter);
    COHead->setText(unitCO);

    COFoot = new QLabel;
    layout->addWidget(COFoot,2,3,1,1, Qt::AlignCenter);
    COFoot->setText(unitCO);

    layout->setColumnMinimumWidth(0,400);
    std::cout <<" created Biometric"<<std::endl;
}

void HeadGUI::createIRPicture()
{
    IRPicture = new QLabel;
    layout->addWidget(IRPicture, 0, 0, 3, 2, Qt::AlignCenter);

    QPixmap filler(ImageWidth, ImageHeight);
    filler.fill(Qt::red);
    IRPicture->setPixmap(filler);

    thread = new LeptonThread(&abc);
    connect(thread, SIGNAL(updateImage(unsigned short *,int,int, unsigned char*)), this, SLOT(updateImage(unsigned short *, int, int, unsigned char*)));
    thread->start();
}

void HeadGUI::createConnections()
{
    connect(this,SIGNAL(certifyPersonaeSignal()),this,SLOT(certifyPersonae()));

    connect(this,SIGNAL(upSignal()),this,SLOT(up()));
    connect(this,SIGNAL(downSignal()),this,SLOT(down()));
    connect(this,SIGNAL(rightSignal()),this,SLOT(right()));
    connect(this,SIGNAL(leftSignal()),this,SLOT(left()));
    connect(this,SIGNAL(backSignal()),this,SLOT(back()));
    connect(this,SIGNAL(certifySignal()),this,SLOT(certify()));

    connect(this,SIGNAL(coosingStatusSignal()),this,SLOT(coosingStatus()));
    connect(this,SIGNAL(updateBraceletSignal(bool,bool,bool,bool,bool,bool)),this,SLOT(updateBracelet(bool,bool,bool,bool,bool,bool)),Qt::DirectConnection);

    connect(this,SIGNAL(updateTempHeadSignal(int)),this,SLOT(updateTempHead(int)));
    connect(this,SIGNAL(updateTempFootSignal(int)),this,SLOT(updateTempFoot(int)));
    connect(this,SIGNAL(updateCOHeadSignal(int)),this,SLOT(updateCOHead(int)));
    connect(this,SIGNAL(updateCOFootSignal(int)),this,SLOT(updateCOFoot(int)));
    connect(this,SIGNAL(updateMotionControlSignal(bool)),this,SLOT(updateMotionControl(bool)));

    connect(this,SIGNAL(changingLightSignal()),this,SLOT(changingLight()));
    connect(this,SIGNAL(messageRecivedSignal(QString)),this,SLOT(messageRecived(QString)));
    connect(this,SIGNAL(changeScreenModeSignal()),this,SLOT(changeScreenMode()));

    connect(this,SIGNAL(newDataFromArduinoSignal()),this,SLOT(sortingNewDataFromArduino()));
    connect(this,SIGNAL(newDataFromHeadquaterSignal()),this,SLOT(sortingNewDataFromHeadquater()));
    connect(this,SIGNAL(newValuesForHeadquater()),this,SLOT(sortingValuesForHeadquater()));
}

void HeadGUI::createCommunication()
{
    Status = new QLabel;
    layout->addWidget(Status,2,0,3,3, Qt::AlignCenter);
    Status->setText("Okay");

    Messages = new QLabel;
    layout->addWidget(Messages,2,2,3,3, Qt::AlignCenter);
    Messages->setText("No new Messages");
}

void HeadGUI::createImageOverlay()
{
    Overlay->addWidget(NormalScreen);//!

    IRPictureFullScreenOverlay = new QLabel;//!!!
    IRPictureFullScreenOverlay->showMaximized();
    Overlay->addWidget(IRPictureFullScreenOverlay);

    ComplexLayout->addWidget(Overlay);
}

void HeadGUI::newDataFromHeadquater()
{
    emit newDataFromHeadquaterSignal();
}

void HeadGUI::newDataFromArduino()
{std::cout<<"new Data"<<std::endl;
    emit newDataFromArduinoSignal();
}

void HeadGUI::sortingNewDataFromHeadquater()
{
    if(networkUser->getBool(NEW_MESSAGE) == true)
    {
        networkUser->setBools(NEW_MESSAGE,false);
        QString newMessage = QString::fromStdString(networkUser->getMessage());
        emit messageRecivedSignal(newMessage);
    }
}

void HeadGUI::sortingNewDataFromArduino()
{std::cout<<"calling Update Bracelet"<<std::endl;
    emit updateBraceletSignal(sensorData->getButton(KNOB_UP), sensorData->getButton(KNOB_DOWN), sensorData->getButton(KNOB_RIGHT), sensorData->getButton(KNOB_LEFT), sensorData->getButton(KNOB_BACK), sensorData->getButton(KNOB_CERTIFY));
}

void HeadGUI::readingSensors()
{
    emit updateTempHeadSignal(sensorData->getRawDaTa(TEMPRETURE_OBOVE));
    emit updateTempFootSignal(sensorData->getRawDaTa(TEMPRETURE_BELOW));

    emit updateCOHeadSignal(sensorData->getRawDaTa(CO_TOP));
    emit updateCOFootSignal(sensorData->getRawDaTa(CO_FLOP));

    emit updateMotionControlSignal(sensorData->getRawDaTa(EMERGENCY));
}

void HeadGUI::defauftValues()
{
    //values for menu
    horizontal = 0;
    vertical = 1;

    answerPossible = false;

    emergencyPossible = false;
    SendEmergency = false;

    //values for communication with headquater
    IDisSet = false;

    recentStatus = 0;
    updatedStatus = false;

    recentAnswer = 0;
    answeredMessage = false;

    recentTempHead = 0;
    recentTempFoot = 0;
    recentCOHead = 0;
    recentCOFoot = 0;

    updatedTempHead = false;
    updatedTempFoot = false;
    updatedCOHead = false;
    updatedCOFoot = false;
    newConfirmedID = false;

    //values from headquater
    recivedMessage= "Noch keine Nachrichten";

    //other values
    lightOn = false;
    updatedPicture = false;
    IRPictureMaxSize = false;
    timeNoMotion = 0;
}

void HeadGUI::certifyPersonae()
{
    ID = 0;
    horizontal = 4;
}

void HeadGUI::up()
{
    if(vertical>0)
    {
        vertical--;
    }

    emergencyPossible = false;
    SendEmergency = false;

    emit coosingStatusSignal();
}

void HeadGUI::down()
{
    if (vertical < NumberDiffValues[horizontal])
    {
        vertical++;
    }

    emergencyPossible = false;
    SendEmergency = false;

    emit coosingStatusSignal();
}

void HeadGUI::right()
{
    if (horizontal < NumberDiffMenues && answerPossible == true && IDisSet == true)
    {
        horizontal++;
    }

    if(horizontal < NumberDiffMenues && answerPossible==false && IDisSet == true)//jump over 1
    {
        if(horizontal==0)
        {
            horizontal = 2;
        }
        else
        {
            horizontal++;
        }
    }

   if (vertical > NumberDiffValues[horizontal])
    {
        vertical = NumberDiffValues[horizontal];
    }

    emergencyPossible = false;
    SendEmergency = false;

    emit coosingStatusSignal();
}

void HeadGUI::left()
{
    if (horizontal > 0 && answerPossible == true && IDisSet == true)
    {
        horizontal--;
    }

    if(horizontal > 0 && answerPossible == false && IDisSet == true)//jump over 1
    {
        if(horizontal==2)
        {
            horizontal = 0;
        }
        else
        {
            horizontal--;
        }
    }

    if (vertical > NumberDiffValues[horizontal])
    {
        vertical = NumberDiffValues[horizontal];
    }

    emergencyPossible = false;
    SendEmergency = false;

    emit coosingStatusSignal();
}

void HeadGUI::back()
{
    if(IDisSet == true)
    {
        if (emergencyPossible == false)
        {
            SendEmergency = false;
            emergencyPossible = true;
            horizontal = 0;
            vertical = 0;
        }
        else
        {
            SendEmergency = true;
        }
    }

    emit coosingStatusSignal();
}

void HeadGUI::certify()
{
    if (SendEmergency==false)
    {
       sensorData->setSendData(0,PARTY);

        switch (horizontal)
        {
        case 0:
        {
            Status->setText(Stati[vertical]);
            Status->setStyleSheet("QLabel{background-color : lightgray;}");

            recentStatus = vertical;
            updatedStatus = true;

           emit newValuesForHeadquater();

            break;
        }

        case 1:
        {
            QString between ="Antwort: ";
            QString txtMessage = recivedMessage + between + messageAnswers[vertical];
            Messages->setText(txtMessage);
            Messages->setStyleSheet("QLabel{background-color : lightgrey;}");

            recentAnswer = vertical;
            answeredMessage = true;
            answerPossible = false;

            emit newValuesForHeadquater();

            break;
        }

        case 2:
        {
            if(vertical == 0)
            {
                lightOn = false;
            }
            else
            {
                lightOn = true;
            }

            emit changingLightSignal();

            break;
        }

        case 3:
        {
			switch (vertical)
			{
				case 0:
				{
					IRPictureMaxSize = false;
					outline = false;
					break;
				}
				case 1:
				{
					IRPictureMaxSize = true;
					outline = false;
					break;
				}
				case 2:
				{
					IRPictureMaxSize = false;
					outline = true;
					break;
				}
				case 3:
				{
					IRPictureMaxSize = true;
					outline = true;
					break;
				}
				default:
				{
					IRPictureMaxSize = false;
					outline = false;
					break;
				}
			}

            emit changeScreenModeSignal();
			/*
				short version, but no testing so no risk

				IRPictureMaxSize = (vertical % 2 == 0);
				outline = vertical > 1;
			*/
            break;
        }

        case 4:
        {
            Personae->setText(Name[vertical]);

            ID = vertical;
            newConfirmedID = true;
            IDisSet = true;

            horizontal = 0;//reset um aus certifyPersonae raus zu kommen

            emit newValuesForHeadquater();

            break;
        }
        }
    }
    else
    {
        Status->setText("MAYDAY");
        Status->setStyleSheet("QLabel{background-color : red;}");

        recentStatus = 5;
        updatedStatus = true;
        sensorData->setSendData(1,PARTY);

        emit newValuesForHeadquater();
    }
}

void HeadGUI::coosingStatus()
{
    if(IRPictureMaxSize == true)
    {
        IRPictureMaxSize = false;
        emit changeScreenModeSignal();
    }

    if (SendEmergency == false)
    {
        switch(horizontal)
        {
        case 0:
            Status->setText(Stati[vertical]);
            Status->setStyleSheet("QLabel{background-color : white;}");

            Messages->setText(recivedMessage);
            Messages->setStyleSheet("QLabel{background-color : lightgrey;}");

            break;

        case 1:
        std::cout <<"antwort eingeben"<<std::endl;
            Messages->setText(messageAnswers[vertical]);
            Messages->setStyleSheet("QLabel{background-color : white;}");

            Status->setText(Stati[recentStatus]);
            Status->setStyleSheet("QLabel{background-color : lightgrey;}");

            break;

        case 2:
            Light->setText(lightSwitch[vertical]);

            break;

        case 3:
            IRPictureFullScreen->setText(PictureFullScreen[vertical]);

            break;

        case 4:
            Personae->setText(Name[vertical]);

            break;
        }
    }
    else
    {
        Status->setText("Mayday melden?");
        Status->setStyleSheet("QLabel{background-color : orange;}");
    }
}

void HeadGUI::sortingValuesForHeadquater()
{
    if(newConfirmedID == true)
    {
        networkUser->setBools(NEW_CONFIRMED_ID, true);
        networkUser->setID(ID);

        newConfirmedID = false;
    }

    if(IDisSet == true)
    {
        if(updatedStatus == true)
        {
            networkUser->setBools(UPDATED_STATUS_SIGNAL, true);
            networkUser->setInteger(RECENT_STATUS, recentStatus);

            updatedStatus = false;
        }

        if(answeredMessage == true)
        {
            networkUser->setBools(ANSWERD_MESSAGE_SIGNAL, true);
            networkUser->setInteger(ANSWER, recentAnswer);

            answeredMessage = false;
        }

        if(updatedTempHead == true)
        {
            networkUser->setBools(UPDATED_TEMP_HEAD_SIGNAL, true);
            networkUser->setInteger(RECENT_TEMP_HEAD, recentTempHead);

            updatedTempHead = false;
        }

        if(updatedTempFoot == true)
        {
            networkUser->setBools(UPDATED_TEMP_FOOT_SIGNAL, true);
            networkUser->setInteger(RECENT_TEMP_FOOT, recentTempFoot);

            updatedTempFoot = false;
        }

        if(updatedCOHead == true)
        {
            networkUser->setBools(UPDATED_CO_HEAD_SIGNAL, true);
            networkUser->setInteger(RECENT_CO_HEAD, recentCOHead);

            updatedCOHead = false;
        }

        if(updatedCOFoot == true)
        {
            networkUser->setBools(UPDATED_CO_FOOT_SIGNAL, true);
            networkUser->setInteger(RECENT_CO_FOOT, recentCOFoot);

            updatedCOFoot = false;
        }

        if(updatedPicture == true)
        {
            networkUser->setBools(UPDATE_IMAGE_SIGNAL, true);
			networkUser->setBITBildMutex(true);
            for(int i = 0; i < 2 * PacketBytes*FrameHeight; i++)
            {
				*(networkUser->getBITBildArray() + i) = *(resultPicture + i);//vielleicht nur handle übergeben?
            }
			networkUser->setBITBildMutex(false);
            updatedPicture = false;
        }
    }
}

void HeadGUI::messageRecived(QString sendMessage)
{std::cout <<"neu Nachricht"<<std::endl;
    recivedMessage = sendMessage;
    Messages->setText(recivedMessage);

    answerPossible = true;
}

void HeadGUI::updateBracelet(bool valueUp, bool valueDown, bool valueRight, bool valueLeft, bool valueBack,bool valueCertify)
{std::cout<<"update Bracelet"<<std::endl;
    otherSignals = false;

    if (valueCertify == true)
    {std::cout<<"certify"<<std::endl;
        otherSignals = true;
        emit certifySignal();
    }

    if (valueBack == true && otherSignals == false)
    {std::cout<<"back"<<std::endl;
        otherSignals = true;
        emit backSignal();
    }

    if ( valueRight == true && otherSignals == false)
    {std::cout<<"right"<<std::endl;
        otherSignals = true;
        emit rightSignal();
    }

    if ( valueLeft == true && otherSignals == false)
    {std::cout<<"left"<<std::endl;
        otherSignals = true;
        emit leftSignal();
    }

    if (valueUp == true && otherSignals == false)
    {std::cout<<"up"<<std::endl;
        otherSignals = true;
        emit upSignal();
    }

    if (valueDown == true && otherSignals == false)
    {std::cout<<"down"<<std::endl;
        otherSignals = true;
        emit downSignal();
    }
}

//Color for IRCamera
const int colormap[] = {255, 255, 255, 253, 253, 253, 251, 251, 251, 249, 249, 249, 247, 247, 247, 245, 245, 245, 243, 243, 243, 241, 241, 241, 239, 239, 239, 237, 237, 237, 235, 235, 235, 233, 233, 233, 231, 231, 231, 229, 229, 229, 227, 227, 227, 225, 225, 225, 223, 223, 223, 221, 221, 221, 219, 219, 219, 217, 217, 217, 215, 215, 215, 213, 213, 213, 211, 211, 211, 209, 209, 209, 207, 207, 207, 205, 205, 205, 203, 203, 203, 201, 201, 201, 199, 199, 199, 197, 197, 197, 195, 195, 195, 193, 193, 193, 191, 191, 191, 189, 189, 189, 187, 187, 187, 185, 185, 185, 183, 183, 183, 181, 181, 181, 179, 179, 179, 177, 177, 177, 175, 175, 175, 173, 173, 173, 171, 171, 171, 169, 169, 169, 167, 167, 167, 165, 165, 165, 163, 163, 163, 161, 161, 161, 159, 159, 159, 157, 157, 157, 155, 155, 155, 153, 153, 153, 151, 151, 151, 149, 149, 149, 147, 147, 147, 145, 145, 145, 143, 143, 143, 141, 141, 141, 139, 139, 139, 137, 137, 137, 135, 135, 135, 133, 133, 133, 131, 131, 131, 129, 129, 129, 126, 126, 126, 124, 124, 124, 122, 122, 122, 120, 120, 120, 118, 118, 118, 116, 116, 116, 114, 114, 114, 112, 112, 112, 110, 110, 110, 108, 108, 108, 106, 106, 106, 104, 104, 104, 102, 102, 102, 100, 100, 100, 98, 98, 98, 96, 96, 96, 94, 94, 94, 92, 92, 92, 90, 90, 90, 88, 88, 88, 86, 86, 86, 84, 84, 84, 82, 82, 82, 80, 80, 80, 78, 78, 78, 76, 76, 76, 74, 74, 74, 72, 72, 72, 70, 70, 70, 68, 68, 68, 66, 66, 66, 64, 64, 64, 62, 62, 62, 60, 60, 60, 58, 58, 58, 56, 56, 56, 54, 54, 54, 52, 52, 52, 50, 50, 50, 48, 48, 48, 46, 46, 46, 44, 44, 44, 42, 42, 42, 40, 40, 40, 38, 38, 38, 36, 36, 36, 34, 34, 34, 32, 32, 32, 30, 30, 30, 28, 28, 28, 26, 26, 26, 24, 24, 24, 22, 22, 22, 20, 20, 20, 18, 18, 18, 16, 16, 16, 14, 14, 14, 12, 12, 12, 10, 10, 10, 8, 8, 8, 6, 6, 6, 4, 4, 4, 2, 2, 2, 0, 0, 0, 0, 0, 9, 2, 0, 16, 4, 0, 24, 6, 0, 31, 8, 0, 38, 10, 0, 45, 12, 0, 53, 14, 0, 60, 17, 0, 67, 19, 0, 74, 21, 0, 82, 23, 0, 89, 25, 0, 96, 27, 0, 103, 29, 0, 111, 31, 0, 118, 36, 0, 120, 41, 0, 121, 46, 0, 122, 51, 0, 123, 56, 0, 124, 61, 0, 125, 66, 0, 126, 71, 0, 127, 76, 1, 128, 81, 1, 129, 86, 1, 130, 91, 1, 131, 96, 1, 132, 101, 1, 133, 106, 1, 134, 111, 1, 135, 116, 1, 136, 121, 1, 136, 125, 2, 137, 130, 2, 137, 135, 3, 137, 139, 3, 138, 144, 3, 138, 149, 4, 138, 153, 4, 139, 158, 5, 139, 163, 5, 139, 167, 5, 140, 172, 6, 140, 177, 6, 140, 181, 7, 141, 186, 7, 141, 189, 10, 137, 191, 13, 132, 194, 16, 127, 196, 19, 121, 198, 22, 116, 200, 25, 111, 203, 28, 106, 205, 31, 101, 207, 34, 95, 209, 37, 90, 212, 40, 85, 214, 43, 80, 216, 46, 75, 218, 49, 69, 221, 52, 64, 223, 55, 59, 224, 57, 49, 225, 60, 47, 226, 64, 44, 227, 67, 42, 228, 71, 39, 229, 74, 37, 230, 78, 34, 231, 81, 32, 231, 85, 29, 232, 88, 27, 233, 92, 24, 234, 95, 22, 235, 99, 19, 236, 102, 17, 237, 106, 14, 238, 109, 12, 239, 112, 12, 240, 116, 12, 240, 119, 12, 241, 123, 12, 241, 127, 12, 242, 130, 12, 242, 134, 12, 243, 138, 12, 243, 141, 13, 244, 145, 13, 244, 149, 13, 245, 152, 13, 245, 156, 13, 246, 160, 13, 246, 163, 13, 247, 167, 13, 247, 171, 13, 248, 175, 14, 248, 178, 15, 249, 182, 16, 249, 185, 18, 250, 189, 19, 250, 192, 20, 251, 196, 21, 251, 199, 22, 252, 203, 23, 252, 206, 24, 253, 210, 25, 253, 213, 27, 254, 217, 28, 254, 220, 29, 255, 224, 30, 255, 227, 39, 255, 229, 53, 255, 231, 67, 255, 233, 81, 255, 234, 95, 255, 236, 109, 255, 238, 123, 255, 240, 137, 255, 242, 151, 255, 244, 165, 255, 246, 179, 255, 248, 193, 255, 249, 207, 255, 251, 221, 255, 253, 235, 255, 255, 24};

void HeadGUI::updateImage(unsigned short *data, int minValue, int maxValue, unsigned char *result)
{
    QImage rgbImage = QImage(FrameWidth,FrameHeight,QImage::Format_RGB888);
    QVector<unsigned short> rawData = QVector<unsigned short> (FrameWords);

   // Record the raw data and min/max values
    memcpy(&rawData[0], data, 2*LeptonThread::FrameWords);//memcpy(wohin gespeichert,woher daten, Nummer der Bytes die kopiert werden)

    abc.unlock();

    if(maxValue == 0) maxValue = 2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2-1;

    rawMin = minValue;
    rawMax = maxValue;
	if (!outline)
	{
		// Map "rawData" to rgb values in "rgbImage" via the colormap
		int diff = rawMax - rawMin + 1;

		//std::cout<<"min "<< minValue<<" max "<<maxValue<< " diff "<<diff<<std::endl;

		for (int y = 0; y < LeptonThread::FrameHeight; ++y)
		{
			for (int x = 0; x < LeptonThread::FrameWidth; ++x)
			{
				int baseValue = rawData[LeptonThread::FrameWidth*(LeptonThread::FrameHeight - 1 - y) + (LeptonThread::FrameWidth - 1 - x)]; // take input value in [0, 65536)
				int scaledValue = 256 * (baseValue - rawMin) / diff; // map value to interval [0, 256), and set the pixel to its color value above
				//std::cout<< baseValue<<" "<<scaledValue<<std::endl;
				rgbImage.setPixel(x, y, qRgb(colormap[3 * scaledValue], colormap[3 * scaledValue + 1], colormap[3 * scaledValue + 2]));//segmentation fault kann mit begrenzung der Werte von 1 bis 256 verieden werden nur Frame rate leidet sehr
			}
		}
	}
	else
	{
        double diff = rawMax/rawMin;
		for (int y = 0; y < LeptonThread::FrameHeight; ++y)
		{
			for (int x = 0; x < LeptonThread::FrameWidth; ++x)
			{
				int baseValue = rawData[LeptonThread::FrameWidth*(LeptonThread::FrameHeight - 1 - y) + (LeptonThread::FrameWidth - 1 - x)]; // take input value in [0, 65536)
				int comparrator = baseValue;//initialise value which will be the refference
				if ((x + 1) < LeptonThread::FrameWidth)
				{
					comparrator = rawData[LeptonThread::FrameWidth*(LeptonThread::FrameHeight - 1 - y) + (LeptonThread::FrameWidth - x)]; //value to determine how big the difference is
				}
				int scaledValue = 256.0 * (comparrator/baseValue)/diff; // map value to interval [0, 256), and set the pixel to its color value above
				//if (scaledValue > 256) scaledValue = 256;//cut off to high peeks
				rgbImage.setPixel(x, y, qRgb(scaledValue,scaledValue,scaledValue));//segmentation fault kann mit begrenzung der Werte von 1 bis 256 verieden werden nur Frame rate leidet sehr
			}
		}
	}

    // Update the on-screen image
    if(IRPictureMaxSize == false)
    {
		QPixmap pixmap = QPixmap::fromImage(rgbImage).scaled(ImageWidth, ImageHeight, Qt::KeepAspectRatio);
        IRPicture->setPixmap(pixmap);
    }
    else
    {
		QPixmap pixmap = QPixmap::fromImage(rgbImage).scaled(1.4*ImageWidth, 1.4*ImageHeight, Qt::KeepAspectRatio);
        IRPictureFullScreenOverlay->setPixmap(pixmap);//!
    }

    updatedPicture = true;
    resultPicture = result;
    emit sortingValuesForHeadquater();
}

void HeadGUI::updateTempHead(int recentTemp)
{
    QString txtrecentTemp = QString::number(recentTemp);
    QString txtTempHead = txtrecentTemp+unitTemp;
    TempHead->setText(txtTempHead);

    recentTempHead = recentTemp;
    updatedTempHead = true;

    emit newValuesForHeadquater();
}

void HeadGUI::updateTempFoot(int recentTemp)
{
    QString txtrecentTemp = QString::number(recentTemp);
    QString txtTempFoot = txtrecentTemp+unitTemp;
    TempFoot->setText(txtTempFoot);

    recentTempFoot = recentTemp;
    updatedTempFoot =true;

    emit newValuesForHeadquater();
}

void HeadGUI::updateCOHead(int recentCO)
{
    QString txtrecentCO = QString::number(recentCO);
    QString txtCOHead = txtrecentCO+unitCO;
    COHead->setText(txtCOHead);

    recentCOHead = recentCO;
    updatedCOHead = true;

    emit newValuesForHeadquater();
}

void HeadGUI::updateCOFoot(int recentCO)
{
    QString txtrecentCO = QString::number(recentCO);
    QString txtCOFoot = txtrecentCO+unitCO;
    COFoot->setText(txtCOFoot);

    recentCOFoot = recentCO;
    updatedCOFoot =true;

    emit newValuesForHeadquater();
}

void HeadGUI::updateMotionControl(bool recentMotion)
{
    if(recentMotion == true)
    {
        timeNoMotion++;
    }
    else
    {
        timeNoMotion = 0;
    }

    if(timeNoMotion == 30)//only needs to be done once
    {
        SendEmergency = true;
		emit certifySignal();
    }

    if(timeNoMotion == 15)
    {
		emit coosingStatusSignal();
    }
}

void HeadGUI::changingLight()
{
   if(lightOn == true)
   {
       Light->setText(lightSwitch[1]);
       Light->setStyleSheet("QLabel{background-color : yellow;}");
       sensorData->setSendData(1,LAMPE);
   }
   else
   {
       Light->setText(lightSwitch[0]);
       Light->setStyleSheet("QLabel{background-color : lightgrey;}");
       sensorData->setSendData(0,LAMPE);
   }

}

void HeadGUI::changeScreenMode()
{
    if(IRPictureMaxSize == true)
    {
        Overlay->setCurrentIndex(1);//!
    }
    else
    {
        Overlay->setCurrentIndex(0);
    }
}

HeadGUI::~HeadGUI()
{

}
