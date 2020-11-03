#include <iostream>
#include <vector>
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


struct ComplexNum
{
	float x, y;

	float length()
	{
		return sqrt(x * x + y * y);
	}

	bool operator!=(const ComplexNum other)
	{
		return (x != other.x || y != other.y);
	}
	bool operator==(const ComplexNum other)
	{
		return (x == other.x && y == other.y);
	}

	ComplexNum operator+(const ComplexNum other)
	{
		return { x + other.x, y + other.y };
	}

	void operator+=(const ComplexNum other)
	{
		x += other.x;
		y += other.y;
	}

	ComplexNum operator*(const ComplexNum other)
	{
		return 	{ x * other.x - y * other.y, x * other.y + y * other.x };
	}

};

struct Circle
{
	ComplexNum pos;
	float radius;
	float rotation;
	float rotationSpeed = 0;

	void updateRotation(float totalTime)
	{
		rotation = rotationSpeed *totalTime ;
	}
};

const int ScreenSizeX = 500;
const int ScreenSizeY = 500;
const float SizeX = 500;
const float SizeY = 500;
int circleCount = 6;
const float PI = 3.14159;


olc::Sprite* overlay;
olc::Sprite* data;
std::vector<ComplexNum> nums;
std::vector<Circle> circles;

ComplexNum spriteCoordToSpace(int w, int h, ComplexNum pos)
{
	return { pos.x / (float)(w - 1) * 2.0f -1.0f,pos.y / (float)(h-1) * 2.0f -1.0f };
}

ComplexNum toScreenSpace(ComplexNum pos)
{
	return { (pos.x+SizeX/2.0f) * (float)ScreenSizeX/ SizeX,(pos.y+ SizeY / 2.0f) * (float)ScreenSizeY/ SizeY};//todo move screeninfo to vars
}
float toScreenSpace(float num)
{
	return num * (float)ScreenSizeX/10.0f;
}

void TraceLine(ComplexNum start)
{
	int n = 0;
	ComplexNum currentPos = start;
	ComplexNum previousPos = { -1,-1 };
	float startingRadians = 0;//range : [0, 2PI]

	while (n < 500)
	{
		//radial search
		for(int i = 0; i<8; i++)
		{
			float radians = startingRadians + (float)i * PI / 4.0f;
			ComplexNum position = ComplexNum{ roundf(cos(radians)),roundf(sin(radians)) }+currentPos;
			//std::cout << position.x << " " << position.y << "  "<<radians << std::endl;
			//if(currentPos==ComplexNum{15,15})
			//	std::cout << radians << std::endl;
			if(data->GetPixel(position.x, position.y)==olc::Pixel(1,0,0))
			{
				if(position != previousPos)
				{
					std::cout << std::endl;
					nums.push_back(spriteCoordToSpace(data->width,data->height,position));
					//std::cout << position.x << " " << position.y << std::endl;
					previousPos = currentPos;
					currentPos = position;
					startingRadians = fmod(radians, 2.0f * PI);
					break;
				}
			}

		}
		if (currentPos == start)
			break;
		n++;
	}
}


//todo please rewrite, please
void getNumbers()
{
	ComplexNum start = { -1,-1 };
	bool breakCondition = false;
	for(int i=0; i<data->width;i++)
	{
		for(int j = 0; j<data->height; j++)
		{
			// outside the sprite olc::BLACK is returned, added a bit of red to prevent the outside as being reconized a point
			breakCondition = data->GetPixel(i, j) == olc::Pixel(1, 0, 0, 255);
			if (breakCondition)
			{
				start = { (float)i,(float)j };
				break;
			}
		}

		if (breakCondition)
		{
			break;
		}
	}
	if (start.x != -1)
	{
		TraceLine(start);
	}
	else
	{
		std::cout << "no start of curve found" << std::endl;
	}

}

void computeCircles()
{
	//compute center of 1st circle
	circles = {};
	ComplexNum center = { 0,0 };
	for (auto& c : nums) { center += c; }

	for (int i = 0; i < circleCount; i++)
	{
		int rSpeed;
		if(i % 2 == 0)
		{
			rSpeed = (i / 2)+1;
		}
		else
		{
			rSpeed = (-i / 2)-1;
		}

		float rotationStepRadians = 2.0f * PI/(float)(nums.size()-1)*rSpeed;
		ComplexNum rotation = { cos(rotationStepRadians),sin(rotationStepRadians) };

		ComplexNum cInfo = nums[0];
		for (int j = 1; j < nums.size(); j++) 
		{
			ComplexNum rotation = { cos(rotationStepRadians*(float)j),sin(rotationStepRadians* (float)j) };
			cInfo += nums[j]*rotation;
		}
		//std::cout << "x:" <<  cInfo.x << " y:" << cInfo.y << std::endl;
		//std::cout << std::endl;

		if (i == 0)
			circles.push_back({ center,cInfo.length(),atan2(cInfo.y,cInfo.x), (float)rSpeed });
		else
			circles.push_back({ {0,0},cInfo.length(),atan2(cInfo.y,cInfo.x),(float) rSpeed });
	}
}


class PixelControler : public olc::PixelGameEngine
{
public:
	PixelControler()
	{
		sAppName = "fourier";
	}

	void drawC(Circle c)
	{
		ComplexNum screenPos = toScreenSpace(c.pos);
		float screenRad = toScreenSpace(c.radius);
		DrawCircle(screenPos.x, screenPos.y, screenRad);
		DrawLine(screenPos.x, screenPos.y, screenPos.x + cos(c.rotation) * screenRad, screenPos.y + sin(c.rotation) * screenRad);
		
	}
public:
	bool OnUserCreate() override
	{	
		data = new olc::Sprite("img2.png");
		olc::Pixel p = data->GetPixel(0, 0);
		getNumbers();
		computeCircles();
		return true;
	}

	float totalTime = 0;
	ComplexNum prevPos = { 0,0 };
public:
	bool OnUserUpdate(float fTimeElapsed) override
	{
		//if(floor((totalTime+fTimeElapsed)/6.0f)>floor(totalTime/6.0f))
		//	overlay = new olc::Sprite(500, 500);

		totalTime += fTimeElapsed;
		//circleCount = 4 + (int)(totalTime/6.0f);
		//computeCircles();


		ComplexNum center;
		for(int i = 0; i<circles.size(); i++)
		{
			circles[i].updateRotation(totalTime);
			if (i != 0)
			{
				circles[i].pos =
				{
					circles[i - 1].pos.x + circles[i - 1].radius * cos(circles[i - 1].rotation),
					circles[i - 1].pos.y + circles[i - 1].radius * sin(circles[i - 1].rotation)
				};
			}
		}


		SetDrawTarget(overlay);

		ComplexNum pixelpos = toScreenSpace({
					circles[circles.size() - 1].pos.x + circles[circles.size() - 1].radius * cos(circles[circles.size() - 1].rotation),
					circles[circles.size() - 1].pos.y + circles[circles.size() - 1].radius * sin(circles[circles.size() - 1].rotation)
			});
		if (prevPos.x == 0)
		{
			Draw(pixelpos.x, pixelpos.y, olc::RED);
		}
		else
		{
			DrawLine(prevPos.x, prevPos.y, pixelpos.x, pixelpos.y);
		}
		prevPos = pixelpos;

		SetDrawTarget(nullptr);
		Clear(olc::BLACK);
		DrawSprite(0, 0, overlay);
	
		for (int i = 0; i < circles.size(); i++)
		{
			drawC(circles[i]);
		}
			
		for(int i =0; i<nums.size(); i++)
		{
			ComplexNum num = toScreenSpace(nums[i]);
			DrawCircle(num.x, num.y, 1);
		}

		//ComplexNum center = { 0,0 };
		//for (auto& c : nums) { center += c; }
		//DrawCircle(center.x, center.y, 3);


		return true;
	}
};

int main()
{
	overlay = new olc::Sprite(500, 500);
	PixelControler controler;
	if (controler.Construct(500, 500, 1, 1))
		controler.Start();
	
}
