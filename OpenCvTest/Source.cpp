#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"	
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <time.h>

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "MQTT"
//#define PAYLOAD     "Hello World"
#define QOS         1
#define TIMEOUT     10000L
using namespace cv;
using namespace std;

MQTTClient_message pubmsg;
MQTTClient client;
int rc;
void init()
{
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	pubmsg = MQTTClient_message_initializer;

	MQTTClient_create(&client, ADDRESS, CLIENTID,
		MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(-1);
	}
}

void publish(char* PAYLOAD)
{
	pubmsg.payload = PAYLOAD;
	pubmsg.payloadlen = strlen(PAYLOAD);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	MQTTClient_deliveryToken token;
	MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
	printf("Waiting for up to %d seconds for publication of %s\n"
		"on topic %s for client with ClientID: %s\n",
		(int)(TIMEOUT / 1000), PAYLOAD, TOPIC, CLIENTID);
	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	printf("Message with delivery token %d delivered\n", token);
}

void deinit()
{
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
}

class Person {
private:
	long x, y, id;
public:
	Person(long x, long y, long id)
	{
		this->x = x;
		this->y = y;
		this->id = id;
	}
	int getId()
	{
		return this->id;
	}
	int getX()
	{
		return this->x;
	}
	int getY()
	{
		return this->y;
	}
	void update(long x, long y)
	{
		this->x = x;
		this->y = y;
	}

};


std::vector<Person> listPerson;
int countPerson = 0;
int enter = 0;
int out = 0;
static int id = 0;

bool InActive(long x, long y, long curr)
{
	if (curr >= x && curr <= y)
		return true;
	return false;
}
void counting(Rect mr, long left, long right, long mid, long x, long y)
{
	bool isNew = true;
	for (int i = 0; i < listPerson.size(); i++)
	{
		long def = sqrt(pow(x - listPerson[i].getX(), 2) + pow(y - listPerson[i].getY(), 2));
		if (def <= mr.width / 2 && def <= mr.height / 2)
		{
			isNew = false;
			if (InActive(left, right, x))
			{
				if (x >= mid &&  listPerson[i].getX() < mid)
				{
					countPerson++;
				}
				else if (x <= mid && listPerson[i].getX() > mid)
				{
					countPerson--;
				}
				listPerson[i].update(x, y);
				return;

			}
			else
			{
				listPerson.erase(listPerson.begin() + i);
				return;
			}
		}
	}
	if (isNew && InActive(left, right, x)) {
		id++;
		Person p = Person(x, y, id);
		listPerson.push_back(p);
	}

}


int main()
{

	VideoCapture stream1(0);   //0 is the id of video device.0 if you have only one camera   
	init();
	Mat img1, img2, ContourImg;
	Mat grayA, grayB, diffImage, binaryImg;
	Mat morA;
	stream1.read(img1);
	cvtColor(img1, grayA, COLOR_BGR2GRAY);
	cv::waitKey(1000);
	long width = stream1.get(3);
	long left = int(1 * (width / 16));
	long right = int(4 * (width / 16));
	long mid = int(2 * (width / 16));
	long key = 0;
	time_t start = time(0);
	int fcount = 0;
	while (key != 'q') {

		stream1.read(img2);
		cvtColor(img2, grayB, COLOR_BGR2GRAY);
		absdiff(grayB, grayA, diffImage);
		threshold(diffImage, binaryImg, 50, 255, CV_THRESH_BINARY);
		ContourImg = binaryImg.clone();
		vector< vector< Point> > contours;
		findContours(ContourImg,
			contours, // a vector of contours
			CV_RETR_EXTERNAL, // retrieve the external contours
			CV_CHAIN_APPROX_SIMPLE); // all pixels of each contours
		line(img2, Point2i(left, 0), Point2i(left, 640), CV_RGB(255, 0, 0));
		line(img2, Point2i(right, 0), Point2i(right, 640), CV_RGB(255, 0, 0));
		for (int i = 0; i < contours.size(); i++)
		{
			if (contourArea(contours[i]) >  10000)
			{
				Rect mr = boundingRect(contours[i]);
				Moments m = moments(contours[i]);
				long x = int(m.m10 / m.m00);
				long y = int(m.m01 / m.m00);
				circle(img2, Point2i(x, y), 20, CV_RGB(255, 0, 0));

				//rectangle(img2, mr, CV_RGB(255, 0, 0), 2);
				counting(mr, left, right, mid, x, y);

			}
		}
		stringstream ss;
		ss << countPerson;
		string NumberString = ss.str();
		cv::putText(img2, NumberString.c_str(), Point(50, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 200, 200), 4);
		ss.clear();
		double seconds_since_start = difftime(time(0), start);
		if (seconds_since_start == 5)
		{
			start = time(0);
			if (fcount != countPerson)
				publish((char*)&NumberString[0]);
			fcount = countPerson;
		}
		cv::imshow("Diff", diffImage);
		cv::imshow("Process", binaryImg);
		cv::imshow("Ogrinal", grayA);
		cv::imshow("NewImg", grayB);
		cv::imshow("test", img2);
		key = cv::waitKey(50);
	}
	deinit();
}
