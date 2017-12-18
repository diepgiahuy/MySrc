#include < stdio.h>
#include < iostream>
#include <time.h>
#include < opencv2\opencv.hpp>
#include < opencv2/core/core.hpp>
#include < opencv2/highgui/highgui.hpp>
#include < opencv2/video/background_segm.hpp>


using namespace cv;
using namespace std;
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
/** Global variables */
String face_cascade_name = "\\home\\pi\\MQTT\\haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "\\home\\pi\\MQTT\\haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";
Mat blur(Mat image)
{
	for (int i = 1; i<12; i = i + 2)
	{
		// smooth the image in the "src" and save it to "dst"
		// blur(src, dst, Size(i,i));

		// Gaussian smoothing
		// GaussianBlur( src, dst, Size( i, i ), 0, 0 );

		// Median smoothing
		medianBlur(image, image, i);

	
	}
	return image;
}

bool InActive(long x , long y , long curr)
{
	if (curr >= x && curr <= y)
		return true;
	return false;
}
void counting(Rect mr, long left , long right , long mid , long x , long y)
{	
	for (int i = 0; i < listPerson.size(); i++)
	{
		long def = sqrt(pow(x - listPerson[i].getX(), 2) + pow(y - listPerson[i].getY(), 2));
		if (def <= x && def <= y)
		{
			if (InActive(left, right, listPerson[i].getX()))
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
			}
			else
			{
				listPerson.erase(listPerson.begin() + i);
			}
			return;
		}
	}
		id++;
		Person p = Person(x, y, id);
		listPerson.push_back(p);

}
void detectAndDisplay(Mat frame)
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);
	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.3, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

	for (size_t i = 0; i < faces.size(); i++)
	{
		Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
		ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);


	}
	//-- Show what you got
	//imshow(window_name, frame);
}

int main()
{

	VideoCapture stream1(0);   //0 is the id of video device.0 if you have only one camera   
	Mat img1, img2, ContourImg;
	Mat grayA, grayB, diffImage, binaryImg;
	Mat morA;
	stream1.read(img1);
	cvtColor(img1, grayA, COLOR_BGR2GRAY);
	cv::waitKey(1000);
	img1 = blur(img1);
	Mat element = getStructuringElement(MORPH_RECT, Size(7, 7), Point(3, 3));
	long width = stream1.get(3);
	long left = int(1* (width / 16));
	long right = int(4 * (width / 16));
	long mid = int(2 * (width / 16));
	long key = 0;
	time_t start = time(0);
	//-- 1. Load the cascades
	if (!face_cascade.load(face_cascade_name)) { printf("--(!)Error loading face cascade\n"); return -1; };
	if (!eyes_cascade.load(eyes_cascade_name)) { printf("--(!)Error loading eyes cascade\n"); return -1; };

	while (key != 'q') {

		stream1.read(img2);
		detectAndDisplay(img2);
		cvtColor(img2, grayB, COLOR_BGR2GRAY);
		absdiff(grayB, grayA, diffImage);
		threshold(diffImage, binaryImg, 75, 255, CV_RETR_EXTERNAL);
	//	morphologyEx(binaryImg, binaryImg, CV_MOP_ERODE, element);
		morphologyEx(binaryImg, morA, CV_MOP_CLOSE, element);
		//morphologyEx(morA, morA, CV_MOP_OPEN, element);

		//morphologyEx(fgMaskMOG2, testImg, CV_MOP_OPEN, element);
		ContourImg = morA.clone();
		vector< vector< Point> > contours;
		findContours(ContourImg,
			contours, // a vector of contours
			CV_RETR_EXTERNAL, // retrieve the external contours
			CV_CHAIN_APPROX_SIMPLE); // all pixels of each contours
		line(img2, Point2i(left, 0), Point2i(left, 640), CV_RGB(255, 0, 0));
		line(img2, Point2i(right, 0), Point2i(right, 640), CV_RGB(255, 0, 0));
		for (int i = 0; i < contours.size(); i++)
		{
			//if(contourArea(contours[i]) > 50000)
			//cout << contourArea(contours[i]) << "\n";
			if (contourArea(contours[i]) >  16000 )
			{
				Rect mr = boundingRect(contours[i]);
					Moments m = moments(contours[i]);
					long x = int(m.m10 / m.m00);
					long y = int(m.m01 / m.m00);
					circle(img2, Point2i(x, y), 20, CV_RGB(255, 0, 0));

					rectangle(img2, mr, CV_RGB(255, 0, 0), 2);
					counting(mr, left, right, mid, x, y);
			}
		}
		stringstream ss;
		ss << countPerson;
		string NumberString = ss.str();
		cv::putText(img2, NumberString.c_str(), Point(50, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 200, 200), 4);
		ss.clear();
		double seconds_since_start = difftime(time(0), start);
		if (seconds_since_start == 1)
		{
			start = time(0);
			cout << seconds_since_start << " ";

		}
		cv::imshow("Diff", diffImage);
		cv::imshow("Process", binaryImg);
		cv::imshow("Ogrinal", grayA);
		cv::imshow("NewImg", grayB);
		cv::imshow("test", img2);
		key = cv::waitKey(5);
	}
}
