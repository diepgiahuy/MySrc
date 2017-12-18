
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////
class Person {
private:
	int x, y, id;
	bool enter = false;
public:
	Person(int x, int y, int id)
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
	bool getEnter()
	{
		return this->enter;
	}
	void update(int x, int y )
	{
		this->x = x;
		this->y = y;
		this->enter = true;
	}

};

