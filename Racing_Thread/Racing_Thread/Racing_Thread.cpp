#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <iterator>
#include <thread>
#include <mutex>
using namespace std;
#define XLENGTH 50
#define YLENGTH 20
#define HAVE_MUTEX 2
#define NO_MUTEX 1

int rRank[3] = {0};
mutex mtx;
int totalDis = 0;
enum Order
{
	ORDER_KILL,
	ORDER_STOP,
	ORDER_RESUME,
	ORDER_MIDDLE,
	ORDER_CANCLE2 = 10,
	ORDER_FASTER,
	ORDER_SLOWER,
	ORDER_RUN
};

class Graphic
{
private:
	char console[YLENGTH][XLENGTH];
	const char* gameName = "Racing Game";
	const char* wall = "------------";
	const char* track = ">          |";
	const char* inst1 = "Select your horse and start racing!";
	const char* inst2 = "After start, you can select option";
	void InitSet()
	{
		for (int i = 0; i < YLENGTH; i++)
		{
			for (int j = 0; j < XLENGTH; j++)
			{
				console[i][j] = ' ';
			}
		}
		console[2][0] = '1'; console[4][0] = '2'; console[6][0] = '3';
		for (int i = 0; i < strlen(gameName); i++)
			console[0][i] = gameName[i];
		for (int i = 0; i < strlen(wall); i++)
		{
			console[1][i + 2] = wall[i];
			console[3][i + 2] = wall[i];
			console[5][i + 2] = wall[i];
			console[7][i + 2] = wall[i];
		}
		for (int i = 0; i < strlen(track); i++)
		{
			console[2][i + 2] = track[i];
			console[4][i + 2] = track[i];
			console[6][i + 2] = track[i];
		}
		for (int i = 0; i < strlen(inst1); i++)
			console[9][i] = inst1[i];
		for (int i = 0; i < strlen(inst2); i++)
			console[10][i] = inst2[i];
	}
	void Print()
	{
		for (int i = 0; i < YLENGTH; i++)
		{
			for (int j = 0; j < XLENGTH; j++)
			{
				cout << console[i][j];
			}
			cout << endl;
		}
	}

public:
	Graphic()
	{
		InitSet();
		Print();
	}
	void SetStringTo(const char* str, int x, int y)
	{
		for (int i = 0; i < strlen(str); i++)
			console[y][x + i] = str[i];
	}

	static void Draw(char ch, int x, int y)
	{
		COORD cur;
		cur.X = x;
		cur.Y = y;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
		printf("%c", ch);
	}
	static void Draw(const char* str, int x, int y)
	{		
		COORD cur;
		cur.X = x;
		cur.Y = y;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
		printf("%s", str);
	}
	static void Remove(int x, int y)
	{
		COORD cur;
		cur.X = x;
		cur.Y = y;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
		printf(" ");
	}

};

class Horse
{
private:
	thread run;
	HANDLE hThread;
	bool killSelf = 0;
	int speed = 0;
	int number = 0;
	int posX = 0;
	int posY = 0;
	int progrsPosX = 0;
	int progrsPosY = 0;
	int rDis = 0;
	const char shape = '>';	

	void Run(int fullDis, int num)
	{
		int tmp = 5000000;
		int tmp2 = 500000;

		int progress10 = 0;
		int progress1 = 0;
		while (rDis < fullDis)
		{

			rDis += 1;
			mtx.lock();
			totalDis += 1;
			mtx.unlock();
			if (rDis == tmp2)
			{
				tmp2 += 500000;
				progress1++;
				Graphic::Draw((char)progress1%10 + '0', 16, posY);
			}
			if (rDis == tmp)
			{
				tmp += 5000000;
				posX += 1;
				progress10++;
				Graphic::Draw(shape, posX, posY);
				Graphic::Draw((char)progress10+'0', 15, posY);
			}
			if (killSelf)
			{
				for (int i = 2; i >= 0; i--)
				{
					if (rRank[i] == 0)
					{
						rRank[i] = number;
						break;
					}
				}
				return;
			}
		}
		Graphic::Draw("100%", 14, posY);
		mtx.lock();
		for (int i = 0; i < 3; i++)
		{
			if (rRank[i] == 0)
			{
				rRank[i] = number;
				break;
			}
		}
		mtx.unlock();
		rDis = 0;

	}

	void Stop()
	{
		while (1)
		{
			if (mtx.try_lock())
			{
				SuspendThread(hThread);
				mtx.unlock();
				break;
			}
		}
	}
	
	void Resume()
	{
		ResumeThread(hThread);
	}
	
	void MoveFaster()
	{
		speed++;
		AdjustSpeed();
		SetThreadPriority(hThread, speed);
	}
	
	void MoveSlower()
	{
		speed--;
		AdjustSpeed();
		SetThreadPriority(hThread, speed);
	}
	
	void AdjustSpeed()
	{
		if (speed < -2)
			speed = -2;
		else if (2 < speed)
			speed = 2;
	}
	
	void Die()
	{	
		killSelf = 1;
	}

public:
	void Act(Order order)
	{
		switch (order) {
		case ORDER_RUN:
		{
			StartRun();
			break; 
		};
		case ORDER_STOP:
		{
			Stop();
			break;
		}
		case ORDER_RESUME:
		{
			Resume();
			break;
		}
		case ORDER_FASTER:
		{
			MoveFaster();
			break;
		}
		case ORDER_SLOWER:
		{
			MoveSlower();
			break;
		}
		case ORDER_KILL:
		{
			Die();
			break;
		}
		default:
			break;
		}
	}

	void StartRun()
	{
		run = thread(&Horse::Run, this, 50000000, number);
		hThread = run.native_handle();		
	}

	void SetHorseXY(int num, int x, int y)
	{
		number = num;
		posX = x;
		posY = y;
		Graphic::Draw('%', 17, posY);
	}
	~Horse()
	{
		run.detach();
	}
};

class Commander
{
#define UP 0x48
#define DOWN 0x50
#define RIGHT 0x4D
#define LEFT 0x4B
private:
	char shape = '>';
	int posX = 0;
	int posY = 0;
	int yMin = 0;
	int yMax = 0;

	Horse *horses[3];
	bool canStart = 0;

	int selectionX[3] = {1,9,17};
	int selectionY[3] = {12,14,15};
	
	bool isHorseRunning = false;
	bool isHorseSelected = false;
	int selectedHorse = 0;
	int selectedOrder = 0;
public:
	Commander()
	{		
		for (int i = 0; i < 3; i++)
			horses[i] = new Horse();
		Graphic::Draw(shape, selectionX[posX], selectionY[posY]);
		horses[0]->SetHorseXY(1, 2, 2);
		horses[1]->SetHorseXY(2, 2, 4);
		horses[2]->SetHorseXY(3, 2, 6);
		Graphic::Draw("HORSE1", selectionX[0] + 1, selectionY[0]);
		Graphic::Draw("HORSE2", selectionX[1] + 1, selectionY[0]);
		Graphic::Draw("HORSE3", selectionX[2] + 1, selectionY[0]);
		Graphic::Draw("Kill", selectionX[0] + 1, selectionY[1]);
		Graphic::Draw("Stop", selectionX[1] + 1, selectionY[1]);
		Graphic::Draw("Resume", selectionX[2] + 1, selectionY[1]);
		Graphic::Draw("Cancle", selectionX[0] + 1, selectionY[2]);
		Graphic::Draw("Faster", selectionX[1] + 1, selectionY[2]);
		Graphic::Draw("Slower", selectionX[2] + 1, selectionY[2]);
	}
	void RankHorse()
	{

	}
	void Adjust()
	{
		if (posX < 0)
			posX = 0;
		else if (posX >= (sizeof(selectionX) / sizeof(int)))
			posX -= 1;
		if (posY < yMin)
			posY = yMin;
		else if (posY > yMax)
			posY = yMax;

		if (selectedOrder<10 && selectedOrder>=ORDER_MIDDLE)
			selectedOrder = ORDER_MIDDLE - 1;
		else if (selectedOrder < 0 && selectedOrder < ORDER_MIDDLE)
			selectedOrder = ORDER_KILL;
		else if (selectedOrder<ORDER_CANCLE2 && selectedOrder>ORDER_MIDDLE)
			selectedOrder = ORDER_CANCLE2;
		else if (selectedOrder>10 && selectedOrder>=ORDER_RUN)
			selectedOrder = ORDER_RUN-1;		

		if (selectedOrder < 0)
			selectedOrder += 10;
		else if (selectedOrder > 20)
			selectedOrder -= 10;
		if (selectedHorse < 0)
			selectedHorse = 0;
		else if (selectedHorse > 2)
			selectedHorse = 2;		
	}
	void Move(int inputKey)
	{
		Graphic::Remove(selectionX[posX], selectionY[posY]);
		switch (inputKey) 
		{
		case UP:
		{
			posY--;
			selectedOrder -= 10;
			break;
		}
		case DOWN:
		{
			posY++;
			selectedOrder += 10;
			break;
		}
	 	case RIGHT: 
		{
			posX++;
			if(!isHorseSelected)
				selectedHorse++; 
			else
				selectedOrder++; 
			break;
		}
		case LEFT:
		{
			posX--;
			if(!isHorseSelected)
				selectedHorse--; 
			else
				selectedOrder--; 
			break; 
		}
		default: break; 
		}
		Adjust();
		Graphic::Draw(shape, selectionX[posX], selectionY[posY]);
	}
	void ReturnCommander()
	{
		isHorseSelected = false;
		selectedHorse = 0;
		selectedOrder = 0;
		yMin = 0;
		yMax = 0;
		Graphic::Remove(selectionX[posX], selectionY[posY]);
		posX = 0;
		posY = 0;
		Graphic::Draw(shape, selectionX[posX], selectionY[posY]);
	}
	void Excute()
	{
		if (!isHorseRunning)
		{
		    isHorseRunning = true;
		    for (int i = 0; i < 3; i++)
		    {
		    	horses[i]->Act(ORDER_RUN);
		    }
			canStart = 1;
			Graphic::Remove(selectionX[posX], selectionY[posY]);
			posX = 0;
			posY = 0;
			Graphic::Draw(shape, selectionX[posX], selectionY[posY]);
		}
		else if (!isHorseSelected)
		{
			isHorseSelected = true;
			yMin = 1;
			yMax = 2;
			Graphic::Remove(selectionX[posX], selectionY[posY]);
			posX = 0;
			posY = 1;
			Graphic::Draw(shape, selectionX[posX], selectionY[posY]);
		}
		else if (selectedOrder == ORDER_CANCLE2)
		{
			ReturnCommander();
		}
		else
		{
			horses[selectedHorse]->Act((Order)selectedOrder);
			ReturnCommander();
		}
	}
	void GetKey(int input)
	{
		if (input == UP|| input == DOWN|| input == LEFT|| input == RIGHT) // 방향키라면
			Move(input);
		else if (input == 13)
			Excute();
	}
	bool CanStart()
	{
		return canStart;
	}
	void IsStarted()
	{
		canStart = 0;
	}
	~Commander()
	{
		delete horses[0];
		delete horses[1];
		delete horses[2];
	}
};

int main()
{
	while (1)
	{
		Graphic graphic;
		Commander commander;
		while (1)
		{
			int isEnd = 0;
			for (int i = 0; i < 3; i++)
			{
				if (rRank[i] == 0)
					break;
				if (i == 2)
					isEnd = 1;
			}
			if (isEnd == 0) {
				if(_kbhit())
				commander.GetKey(_getch());
			}
			else
			{
				char ranking[30];
				char total[30];
				sprintf_s(ranking, "ranking: %d %d %d", rRank[0], rRank[1], rRank[2]);
				sprintf_s(total, "total distance: %d", totalDis);
				Graphic::Draw(ranking, 0, 16);
				Graphic::Draw(total, 0, 17);
				_getch();
				rRank[0] = 0;				
				rRank[1] = 0;
				rRank[2] = 0;
				totalDis = 0;
				system("cls");
				break;
			}
		}
	}
}
