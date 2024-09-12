//2024 09 03   子弹命中效果
//2024 09 09   实现僵尸死亡  解决僵尸死亡bug
//2024 09 09   修复子弹无效  僵尸变少的bug
//2024 09 09   僵尸吃植物
//2024 09 10   阳光降落  阳光收集  向日葵产生阳光(其中阳光球轨迹移动利用贝赛尔曲线)
//2024 09 11   阳光球的生产  解决阳光球飞跃的问题  解决阳光球点击音效的问题
//2024 09 11   代码若干优化
//2024 09 11   片头巡场
//2024 09 11   优化片头巡场
//2024 09 12   卡槽下滑
//2024 09 12   修复豌豆子弹bug
//2024 09 12   游戏结束判定
//2024 09 12 11:40:39 项目完结



#include <stdio.h>
#include <graphics.h>
#include <time.h>
#include <math.h>
#include "tools.h"
#include "vector2.h"
#include <mmsystem.h>


#pragma comment(lib,"winmm.lib")


#define WIN_WIDTH   900
#define WIN_HEIGHT  600
#define ZM_MAX  5


enum{GOING,WIN,FAIL}; //游戏状态枚举类型
int killCount;  //杀僵尸的数量
int zmCount;  //已经出现僵尸的数量
int gameStatus; //游戏状态



bool update;   //判断是否更新的状态变量
typedef enum
{
	PEA,  //豌豆
	SUNFLOWER,     //向日葵
	VEGETARAIN_COUNT   //植物种类数
}VEGETARAIN;

typedef struct vegetarain
{
	int type;   //表示植物的类型,其中0表示没有植物
	int frameIndex;  //播放帧的索引
	int deadtime; //死亡倒计时
	bool catched;  //是否被僵尸捕获
	int timer;  //向日葵植物产出阳光的计时器
	int x, y;   //植物的坐标

	int shootTimer;  //发射计数器

}Vegetarian_type;

Vegetarian_type map[3][9];  //用二维数组来抽象表示地图上的植物


enum {SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};  //阳光球状态


struct sunshineBall
{
	int x, y;         //阳光球在飘落过程中的坐标(x不变)
	int frameIndex;   //当前显示图片索引
	int destY;   //飘落的目标位置Y坐标
	bool used;  //是否被用过
	int timer;  //存在时间


	float xoff; //点击阳光后的x偏移量
	float yoff;  //点击阳光后的y偏移量

	float t;  //贝塞尔曲线的时间点  0....1
	vector2 p1, p2, p3, p4;  //贝塞尔曲线的起始点  终点  控制点
	vector2 pCur;  //当前时刻阳光球的位置
	float speed;  //阳光球速度
	int status;  //阳光球状态

};

struct sunshineBall balls[10];

//僵尸的结构体
struct zm
{
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;   //僵尸出现在第几行
	int blood;
	bool dead;  //僵尸是否死亡
	bool eating;  //僵尸正在吃植物的状态
	
};
struct zm zms[10];   //僵尸的个数为10
IMAGE imgzm[22];
IMAGE imgzmDead[20];
IMAGE imgzmEat[21];
IMAGE imgzmStand[11];




struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;  //爆炸效果
	int frameIndex;  //帧序号
};
struct bullet bullets[30]; //子弹池
IMAGE imgBulletNormal;  //子弹图片
IMAGE imgBulletBlast[4];  //子弹爆炸效果

IMAGE imgBg;  //背景图片在内存的加载位置
IMAGE imgBar;  //卡槽
IMAGE imgVegetarianCard[VEGETARAIN_COUNT];   //植物图片集合
IMAGE* imgVegetarian[VEGETARAIN_COUNT][20];   //植物种类为VEGETARAIN_COUNT,各种植物的图片默认为20张
IMAGE imgSunshineBall[29];   //阳光球

int curX, curY;  //记录鼠标的X,Y坐标
int curPlant;    //记录被点击植物的类型
int row, col;   //行和列
int sunshine; //阳光值

bool fileExist(const char* name)
{
	FILE* fp = fopen(name, "r");
	if (fp == NULL)
	{
		return false;
	}
	else 
	{
		fclose(fp);   //将fclose放入fp非空的情况里,防止给fclose函数传入空指针!!
		return true;
	}

	//另一种写法
	//if (fp)
	//{
	//	fclose(fp);
	//}
	//return fp!=NULL
	
}
void init()
{


	killCount = 0;  //初始化僵尸击杀数
	zmCount = 0;   //初始化出现僵尸
	gameStatus = GOING;   //游戏状态为继续


	char name[64];
	initgraph(WIN_WIDTH, WIN_HEIGHT,1);
	//加载背景图片
	//通过修改项目属性的字符集属性
	loadimage(&imgBg, "res/bg.jpg");

	//加载卡槽图片
	loadimage(&imgBar, "res/bar5.png");

	//给这块存放指针的二维数组进行初始化（赋值为空指针）
	memset(imgVegetarian, 0, sizeof(imgVegetarian));
	memset(map, 0, sizeof(map));  //给map初始化

	for (int i = 0; i < VEGETARAIN_COUNT;i++)
	{
		//加载植物卡片,植物卡片的位置为"res/Card/card_1.png"……"res/Card/card_2.png"
		sprintf_s(name,sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgVegetarianCard[i], name);

		//加载植物
		//"res/zhiwu/0/1.png"……"res/zhiwu/0/13.png"  
		//'res/zhiwu/1/1.png"……"res/zhiwu/1/18.png"

		for (int j = 0; j < 20; j++)
		{

			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j+1);
			if (fileExist(name))
			{
				//由于IMAGE变量并非是C语言中的标准变量,此处赋值处理是C++的语法
				imgVegetarian[i][j] = new IMAGE;  
				//由于初始化该二维指针数组都为空,因此需要先赋值后再加载图片,否则没有空间可以给图片加载
				loadimage(imgVegetarian[i][j], name);
			}
			else
			{
				break;
			}
		}
	}

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++)
	{
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i],name);
	}

	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		balls[i].used = false;
	}
	//配置随机种子
	srand(time(NULL));
	update = false;
	curPlant = 0; //表示没有植物可以渲染

	sunshine = 50;  //阳光等于50

	//设置字体
	LOGFONT f;
	gettextstyle(&f);
	f.lfWidth = 15;
	f.lfHeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;  //抗锯齿效果
	settextstyle(&f);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);


	//初始化僵尸
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgzm[i], name);
	}

	//初始化子弹
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));


	//初始化爆炸子弹
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[i].getwidth() * k,
			imgBulletBlast[i].getheight() * k, true);
	}


	//加载僵尸死亡的图片帧
	for (int i = 0; i < 20; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgzmDead[i], name);
	}

	//加载僵尸吃植物的图片帧
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name,"res/zm_eat/%d.png",i+1);
		loadimage(&imgzmEat[i], name);
	}


	//加载僵尸站立的图片帧
	for (int i = 0; i < 11; i++)
	{
		sprintf_s(name,sizeof(name),"res/zm_stand/%d.png",i+1);
		loadimage(&imgzmStand[i],name);
	}
}

void drawZM()
{
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used)
		{
			/*IMAGE* img = &imgzm[zms[i].frameIndex];*/
			//IMAGE* img = (zms[i].dead) ? imgzmDead : imgzm;  //改变指针的指向来完成动画切换
			IMAGE* img = NULL;
			if (zms[i].dead)  img = imgzmDead;
			else if (zms[i].eating)  img = imgzmEat;
			else img = imgzm;

			img += zms[i].frameIndex;   //指针的移动幅度由指针的类型确定
			putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);
		}
	}
}


//绘制阳光球
void drawSunshines() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		//if (balls[i].used || balls[i].xoff)
		if(balls[i].used)
		{
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			//putimagePNG(balls[i].x, balls[i].y, img);
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
		}
	}

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276, 75, scoreText);  //输出分数

}



void drawcards()
{
	//渲染卡牌
	for (int i = 0; i < VEGETARAIN_COUNT; i++)
	{
		int x = 338 + i * imgVegetarianCard[0].getwidth();
		int y = 6;
		putimagePNG(x, y, &imgVegetarianCard[i]);
	}
}

void drawVegetarian()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type > 0)
			{
				//int x = 256 + j* 81;
				//int y = 179 + i * 102 + 14;

				int sort = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				//putimagePNG(x,y,imgVegetarian[sort][index]);
				putimagePNG(map[i][j].x, map[i][j].y, imgVegetarian[sort][index]);

			}
		}
	}

	if (curPlant)
	{
		IMAGE* imgP = imgVegetarian[curPlant - 1][0];
		putimagePNG(curX - imgP->getwidth() / 2, curY - imgP->getheight() / 2, imgP);
	}

}

void drawBullets()
{
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++)
	{
		if (bullets[i].used)
		{
			if (bullets[i].blast)   //爆炸显示爆炸效果的图片
			{
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else    //正常显示正常子弹飞行效果的图片
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
}



/************
*更新画面
*函数名：updateWindow
****************/
void updateWindow()
{
	BeginBatchDraw();     //开始双缓存
	putimage(-112, 0,&imgBg);         //加载背景图片

	putimagePNG(250, 0, &imgBar);  //加载卡槽图片

	drawcards();
	drawVegetarian();
	drawSunshines();


	drawZM();
	drawBullets();
	EndBatchDraw();    //结束双缓存
}

void createSunshine()
{


	//1.从阳光池取一个来下坠阳光
	static int count = 0;
	static int fre = 100;
	count++;
	if (count >= fre)
	{
		fre = 100 + rand() % 50;
		count = 0;
		int ballMax = sizeof(balls) / sizeof(balls[0]);
		
		/*int ballMax = sizeof(balls) / sizeof(balls[0]);*/
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)return;
		balls[i].used = true;
		balls[i].frameIndex = 0;
		//balls[i].x = 260 + rand() % (900 - 260);
		//balls[i].y = 60;
		//balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;
		//balls[i].xoff = 0;
		//balls[i].yoff = 0;

		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;     //阳光开始下坠
		balls[i].p1 = vector2(260-112 + rand() % (900 - (260-112)), 60);   //起点
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);  //终点
		int off = 2;  //阳光下降时移动的像素
		float distance = balls[i].p4.y - balls[i].p1.y;  //距离
		balls[i].speed = 1.0 / (distance / off);   //速度 1.0表示时间
	}


	//2.从阳光池获得向日葵的生产阳光
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == SUNFLOWER + 1)  //寻找地图上的向日葵
			{
				map[i][j].timer++;    //计时器加1
				if (map[i][j].timer > 200) {
					map[i][j].timer = 0;
					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);
					if (k >= ballMax) return;
					balls[k].used = true;
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);  //起点
					int w = (100 + rand() % 51) * (rand() % 2 ? 1 : -1);  //阳光掉落的距离
					balls[k].p4 = vector2(map[i][j].x + w,
						map[i][j].y + imgVegetarian[SUNFLOWER][0]->getheight()
						- imgSunshineBall[0].getheight());  //终点
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);  //控制点
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);  //控制点
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;   //速度
					balls[k].t = 0;   //表示出产
				}
			}
		}
	}
}
void updateSunshine()
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used)
		{
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;  //改变阳光帧
			if (balls[i].status == SUNSHINE_PRODUCT)   //处于向日葵生产阶段
			{
				struct sunshineBall* sun = &balls[i];  //指针指向当前阳光球的位置
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1)
				{
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND)  //处于太阳球落地状态,由生产的2个状态决定
			{
				balls[i].timer++;
				if (balls[i].timer > 100)
				{
					balls[i].used = false;
					balls[i].timer = 0;
				}

			}
			else if (balls[i].status == SUNSHINE_COLLECT)  //太阳球收集状态
			{
				struct sunshineBall* sun = &balls[i];  //指针指向当前阳光球的位置
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);  //p1+t*(p4-p1);
				if (sun->t > 1)
				{
					sun->used = false;
					sunshine += 25;  //太阳值+25;
				}
			}
			else if (balls[i].status ==   SUNSHINE_DOWN)  //太阳球下坠状态
			{
				struct sunshineBall* sun = &balls[i];  //指针指向当前阳光球的位置
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);  //p1+t*(p4-p1);
				if (sun->t > 1)
				{
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}





		/*	if (balls[i].timer == 0)
			{
				balls[i].y += 2;
			}
			if (balls[i].y >= balls[i].destY)
			{
				balls[i].timer++;
				if (balls[i].timer >= 100)
				{
					balls[i].used = false;
				}
				
			}
		}
		else if (balls[i].xoff)
		{
			float destX = 262;
			float destY = 0;
			float angle = atan((balls[i].y - destY) / (balls[i].x - destX));
			balls[i].xoff = 8 * cos(angle);
			balls[i].yoff = 8 * sin(angle);


			balls[i].x -= balls[i].xoff;
			balls[i].y -= balls[i].yoff;
			if (balls[i].y < 0 || balls[i].x < 262)
			{
				balls[i].xoff = 0;
				balls[i].yoff = 0;
				sunshine += 50;
			}*/
		}
	}
}

void collectSunshine(ExMessage *msg)
{
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();
	for (int i = 0; i < count; i++)
	{
		if (balls[i].used)
		{
			/*int x = balls[i].x;
			int y = balls[i].y;*/

			int x = balls[i].pCur.x;  //阳光球当前的X坐标
			int y = balls[i].pCur.y;  //阳光球当前的Y坐标
			if (msg->x > x && msg->x <x + w && msg->y >y && msg->y < y + h)
			{
				/*balls[i].used = false;*/
				balls[i].status = SUNSHINE_COLLECT;


				//mciSendString("play res/sunshine.mp3",0,0,0);
				PlaySound("res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC); //播放音效的接口(不支持MP3格式)
				//阳光的终点
				/*float destX = 262;
				float destY = 0;

				float angle = atan((y - destY) / (x - destX));
				balls[i].xoff = 8 * cos(angle);
				balls[i].yoff = 8 * sin(angle);*/


				balls[i].p1 = balls[i].pCur;    //起点
				balls[i].p4 = vector2(262, 0);  //终点
				balls[i].t = 0;  //表示起点移动
				float distance = dis(balls[i].p1 - balls[i].p4);  //C++的重载  取模
				float off = 8;  //移动的参数
				balls[i].speed = 1.0 / (distance / off);
				break;
			}
		}
	}
}

//处理僵尸产生的逻辑
void  createZM()
{
	if (zmCount >= ZM_MAX)
	{
		return;
	}
	static int zmFre = 100;
	static int count = 0;
	count++;
	if (count > zmFre)
	{
		count = 0;
		zmFre = 300 + rand() % 200;
		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMax && zms[i].used; i++);
		if (i > zmMax) return;
		if (i < zmMax)
		{
			memset(&zms[i], 0, sizeof(zms[i])); //对僵尸进行清零
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].row = rand() % 3;   //哪一行有僵尸
			zms[i].y = 172 + (1 + zms[i].row) * 100;
			zms[i].speed = 1;
			zms[i].blood = 100;
			zms[i].dead = false;    //僵尸死亡初始化
			zms[i].frameIndex = 0;
			zmCount++;  //出现僵尸数量+1
		}
	}
	
}


//更新僵尸的信息
void updataZM()
{
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	static int count = 0;
	count++;
	if (count > 2*2)
	{
		count = 0;
		//更新僵尸状态
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)
			{
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 25)
				{
					zms[i].used = false;
					//printf("GAME OVER\n");
					//MessageBox(NULL, "over", "over", 0); //简略版本
					//exit(0);
					gameStatus = FAIL;
				}
			}
		}
	}

	static int count2 = 0;
	count2++;
	if (count2 > 4*2)
	{
		count2 = 0;
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)
			{
				if (zms[i].dead)
				{
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20)
					{
						zms[i].used = false; 
						killCount++;
						if (killCount == ZM_MAX)
						{
							gameStatus = WIN;  //游戏判定为胜利
						}
					}
				}
				else if(zms[i].eating) {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else
				{
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}
}

//更新子弹的数据
void  updateBullets()
{
	static int frecount = 0;
	if (++frecount < 2) return;
	frecount = 0;
	//降低子弹运动的速度

	int count = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < count; i++)
	{
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}

			//子弹碰撞检测
			if (bullets[i].blast)
			{
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4)
				{
					bullets[i].used = false;
				}
			}
		}
	}
}

void shoot()
{

	static int count = 0;
	if (++count < 2) return;
	count = 0;
	//降低子弹发射的频率

	int line[3] = { 0 };    //战区
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int dangerX = WIN_WIDTH - imgzm[0].getwidth();   //红色警戒线
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used && zms[i].x < dangerX)
		{
			line[zms[i].row] = 1;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == PEA + 1 && line[i])
			{
				/*static int count = 0;
				count++;*/

				map[i][j].shootTimer++;
				if (map[i][j].shootTimer > 100)
				{
					map[i][j].shootTimer = 0;
					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax)
					{
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 4;


						bullets[k].blast = false;
						bullets[k].frameIndex = 0;
						/*int x = 256 + j * 81;
						int y = 179 + i * 102 + 14;*/


						int zwX = 256-112 + j * 81;  //植物横坐标
						int zwY = 179 + i * 102 + 14;  //植物纵坐标
						bullets[k].x = zwX + imgVegetarian[map[i][j].type - 1][0]->getwidth()-10;
						bullets[k].y = zwY + 5;
					}
				}
			}
		}
	}
}

void cheakBullet2Zm()
{
	int bCount = sizeof(bullets) / sizeof(bullets[0]);
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bCount; i++)
	{
		if (bullets[i].used == false || bullets[i].blast) continue;
		for (int k = 0; k < zCount; k++)
		{
			if (zms[k].used == false) continue;
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if (zms[k].dead == false && bullets[i].row == zms[k].row && x > x1 && x < x2)
				//子弹与僵尸共存在同一行，且子弹与僵尸发生接触
			{
				zms[k].blood -= 20;
				bullets[i].blast = true;  //子弹爆炸
				bullets[i].speed = 0;


				if (zms[k].blood <= 0)
				{
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;  //表示死亡过程的图片帧序号
				}
				break;    //这枚子弹不需要与其他僵尸进行碰撞检测
			}
		}
	}
}

void cheakZm2Zhiwu()
{
	int ZmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < ZmCount; i++)
	{
		if (zms[i].dead) continue;
		int row = zms[i].row;
		for (int k = 0; k < 9; k++)
		{
			if (map[row][k].type == 0)continue;  //判断该行该列有没有植物

			//获得僵尸和植物的坐标信息,通过植物的坐标区间来判断僵尸是否与植物发生相交
			int zhiwuX = 256-112 + k * 81;
			int x1 = zhiwuX+10;    //植物的横坐标1
			int x2 = zhiwuX + 60;  //植物的横坐标2
			int x3 = zms[i].x + 80; //僵尸的横坐标
			if (x3 > x1 && x3 < x2)
			{
				if (map[row][k].catched) {
					/*zms[i].frameIndex++;*/
					map[row][k].deadtime++;
					if (map[row][k].deadtime > 100)
					{
						map[row][k].deadtime = 0;  //清空该行的死 亡倒计时 
						map[row][k].type = 0;      //植物类型为空
						zms[i].eating = false;     //僵尸的状态不再是吃植物
						zms[i].frameIndex = 0;     //僵尸的帧动画为0
						zms[i].speed = 1;      //恢复僵尸的前进
						map[row][k].catched = false; //植物被吃后该行该列应该是没有被捕获的状态
					}
					/*if (zms[i].frameIndex > 100) {
						
					}*/
				}
				else {
					map[row][k].catched = true;  //植物被植物吃过
					map[row][k].deadtime = 0;   //植物刚进入死亡倒计时
					zms[i].eating = true;    //僵尸正在吃植物
					zms[i].speed = 0;  //僵尸吃植物时不会移动
					zms[i].frameIndex = 0;  //僵尸的帧画面
				}
			}
			

		}
	}
}


void collisionCheak()
{
	cheakBullet2Zm();  //子弹与僵尸的碰撞检测
	cheakZm2Zhiwu();   //植物与僵尸的碰撞检测
}

void updateVegetarian()
{
	static int count = 0;
	if (++count < 2)return;
	count = 0;

	//降低更新植物的频率

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type > 0)
			{
				map[i][j].frameIndex++;
				int zhiwuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgVegetarian[zhiwuType][index] == NULL)
				{
					map[i][j].frameIndex = 0;
				}
			}
		}
	}
}

/*********************
*函数名：updateGame
*作用：更新游戏信息
**********************/
void updateGame()
{

	updateVegetarian();


	createSunshine();  //创建阳光
	updateSunshine();  //更新阳光

	createZM();    //创建僵尸
	updataZM();  //更新僵尸

	collisionCheak();  //射击检测

	shoot();       //射击
	updateBullets();  //更新子弹


	
}

void reduceSunshine(int Curplant)
{
	if (Curplant-1 == SUNFLOWER)
	{
		sunshine -= 50;
	
	}
	else if (Curplant-1 == PEA)
	{
		sunshine -= 100;
	}
}


//鼠标点击的函数
void click()
{
	ExMessage msg;           //鼠标信息
	static int status = 0;   //左键点击状态
	if (peekmessage(&msg))   //判断是否有鼠标信息
	{
		
		if (msg.message == WM_LBUTTONDOWN) //点击植物
		{
			if (msg.x > 338 && msg.x < 338 + 65* VEGETARAIN_COUNT && msg.y < 96) 
			{
				int index = (msg.x - 338) / 65;
				status = 1;   //存在左键点击事件
				curPlant = index + 1 ;
			}
			else
			{
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)   //鼠标移动时植物渲染
		{
			curX = msg.x;
			curY = msg.y;
			//记录鼠标移动的坐标，方便植物拖动
		}
		else if (msg.message == WM_LBUTTONUP&&status ==1)   //处理植物种植
		{
			if (msg.x > 256-112 && msg.y > 179 && msg.y < 489)
			{
				 col = (msg.x - (256-112)) / 81;   //得到列
				 row = (msg.y - 179) / 102;  //得到行

				if (map[row][col].type == 0)  //表示该行该列没有植物种植
				{
					if (curPlant-1 == SUNFLOWER && sunshine >= 50)  //阳光值是否能够种植向日葵
					{
						map[row][col].type = curPlant;
						
					}
					else if (curPlant-1 == PEA && sunshine >= 100)//阳光值是否能够种植豌豆
					{
						map[row][col].type = curPlant;
					}
					else
					{
						status = 0;//状态复位
						curPlant = 0;  //植物种类变成0,表示没有植物
						return;
					}

					map[row][col].frameIndex = 0;
					map[row][col].shootTimer = 0;
					
					reduceSunshine(curPlant);   //根据植物来减少阳光值

					map[row][col].x = 256-112 +  col * 81;   //获得植物位置
				    map[row][col].y = 179 + row * 102 + 14;
				}
			}
			status = 0;//状态复位
			curPlant = 0;  //植物种类变成0,表示没有植物
		}
	}
}

void startUI()
{
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;    //判断显示那个选项
	while (1)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu1 : &imgMenu2);
		
		ExMessage msg;
		if (peekmessage(&msg))
		{
			if (msg.message == WM_LBUTTONDOWN &&
				msg.x > 474 && msg.x < 474 + 300 &&
				msg.y>75 && msg.y < 75 + 140)
			{
				flag = 1;
			}
			else if(msg.message==WM_LBUTTONUP && flag  )
			{
				EndBatchDraw();
				break;
			}
		}
		EndBatchDraw();
	}

}

void viewScene()
{
	int xMin = WIN_WIDTH - imgBg.getwidth();

	//僵尸的站位
	vector2 points[9] =
	{ {550,80},{530,160},{630,170},{530,200},{515,270},
		{566,370},{605,340},{750,280},{690,340} };   //vector2这个变量是指坐标

	//使每只僵尸产生随机的帧序号
	int index[9];
	for (int i = 0; i < 9; i++)
	{
		index[i] = rand() % 11;
	}

	int count = 0;

//背景向街道移动
	for (int x = 0; x >= xMin; x-=2)
	{
		BeginBatchDraw();
		putimage(x, 0, &imgBg);

		count++;
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x-xMin+x,points[k].y,&imgzmStand[index[k]]);
			if (count >= 10)
			{
				index[k] = (index[k] + 1) % 11;  //更换帧图片
			}
		}
		if (count >= 10)count = 0;  //将计数归零
		EndBatchDraw();
		Sleep(5);
	}


	//停留1s左右
	for (int i = 0; i < 100; i++)
	{
		BeginBatchDraw();
		putimage(xMin, 0, &imgBg);
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x, points[k].y, &imgzmStand[index[k]]);
			index[k] = (index[k] + 1) % 11;
		}
		EndBatchDraw();
		Sleep(10);  //停留3秒
	}

//背景向房子移动
	for (int x = xMin; x <= -112; x += 2)
	{
		BeginBatchDraw();
		putimage(x, 0, &imgBg);

		count++;
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgzmStand[index[k]]);
			if (count >= 10)
			{
				index[k] = (index[k] + 1) % 11;  //更换帧图片
			}
			if (count >= 10)count = 0;  //将计数归零
		}
		
		EndBatchDraw();
		Sleep(5);
	}
}

//卡槽下滑
void barsDown()
{
	//int ymin = -imgBar.getheight();
	//for (int y = ymin; y<= 0; y+=2)
	//{
	//	BeginBatchDraw();

	//	putimage(0, y, &imgBar);

	//	EndBatchDraw();
	//	Sleep(5);
	//}
	int height = imgBar.getheight();
	for (int y = -height; y <= 0; y++)
	{
		BeginBatchDraw();

		//显示背景和卡槽
		putimage(-112, 0, &imgBg);
		putimagePNG(250,y, &imgBar);


		//显示植物卡牌
		for (int i = 0; i < VEGETARAIN_COUNT; i++)
		{
			int x = 338 + i * 65;
			putimage(x, 6+y, &imgVegetarianCard[i]);  
		}

		EndBatchDraw();
		Sleep(5);
	}


}


bool checkOver()
{
	int ret = false;
	if (gameStatus == WIN)
	{
		loadimage(0, "res/gameWin.png");
		Sleep(2000);
		ret = true;
		
	}
	else if (gameStatus == FAIL)
	{
		Sleep(2000);
		loadimage(0, "res/gameFail.png");
		ret = true;
		
	}
	return ret;
}

int main(void)
{
	init();
	startUI();
	viewScene();
	barsDown();


	int timer = 0;
	bool flag = true;
	while (1)
	{

			click();
			timer += getDelay();
			if (timer >= 10)
			{
				flag = true;
				timer = 0;
			}
			if (flag)
			{
				flag = false;
				
				updateGame();
				updateWindow();
				if(checkOver()) break;
			}
		    
	}
	
	system("pause");

	return 0;
}

