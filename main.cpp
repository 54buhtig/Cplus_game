//2024 09 03   �ӵ�����Ч��
//2024 09 09   ʵ�ֽ�ʬ����  �����ʬ����bug
//2024 09 09   �޸��ӵ���Ч  ��ʬ���ٵ�bug
//2024 09 09   ��ʬ��ֲ��
//2024 09 10   ���⽵��  �����ռ�  ���տ���������(����������켣�ƶ����ñ���������)
//2024 09 11   �����������  ����������Ծ������  �������������Ч������
//2024 09 11   ���������Ż�
//2024 09 11   ƬͷѲ��
//2024 09 11   �Ż�ƬͷѲ��
//2024 09 12   �����»�
//2024 09 12   �޸��㶹�ӵ�bug
//2024 09 12   ��Ϸ�����ж�
//2024 09 12 11:40:39 ��Ŀ���



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


enum{GOING,WIN,FAIL}; //��Ϸ״̬ö������
int killCount;  //ɱ��ʬ������
int zmCount;  //�Ѿ����ֽ�ʬ������
int gameStatus; //��Ϸ״̬



bool update;   //�ж��Ƿ���µ�״̬����
typedef enum
{
	PEA,  //�㶹
	SUNFLOWER,     //���տ�
	VEGETARAIN_COUNT   //ֲ��������
}VEGETARAIN;

typedef struct vegetarain
{
	int type;   //��ʾֲ�������,����0��ʾû��ֲ��
	int frameIndex;  //����֡������
	int deadtime; //��������ʱ
	bool catched;  //�Ƿ񱻽�ʬ����
	int timer;  //���տ�ֲ���������ļ�ʱ��
	int x, y;   //ֲ�������

	int shootTimer;  //���������

}Vegetarian_type;

Vegetarian_type map[3][9];  //�ö�ά�����������ʾ��ͼ�ϵ�ֲ��


enum {SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};  //������״̬


struct sunshineBall
{
	int x, y;         //��������Ʈ������е�����(x����)
	int frameIndex;   //��ǰ��ʾͼƬ����
	int destY;   //Ʈ���Ŀ��λ��Y����
	bool used;  //�Ƿ��ù�
	int timer;  //����ʱ��


	float xoff; //���������xƫ����
	float yoff;  //���������yƫ����

	float t;  //���������ߵ�ʱ���  0....1
	vector2 p1, p2, p3, p4;  //���������ߵ���ʼ��  �յ�  ���Ƶ�
	vector2 pCur;  //��ǰʱ���������λ��
	float speed;  //�������ٶ�
	int status;  //������״̬

};

struct sunshineBall balls[10];

//��ʬ�Ľṹ��
struct zm
{
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;   //��ʬ�����ڵڼ���
	int blood;
	bool dead;  //��ʬ�Ƿ�����
	bool eating;  //��ʬ���ڳ�ֲ���״̬
	
};
struct zm zms[10];   //��ʬ�ĸ���Ϊ10
IMAGE imgzm[22];
IMAGE imgzmDead[20];
IMAGE imgzmEat[21];
IMAGE imgzmStand[11];




struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;  //��ըЧ��
	int frameIndex;  //֡���
};
struct bullet bullets[30]; //�ӵ���
IMAGE imgBulletNormal;  //�ӵ�ͼƬ
IMAGE imgBulletBlast[4];  //�ӵ���ըЧ��

IMAGE imgBg;  //����ͼƬ���ڴ�ļ���λ��
IMAGE imgBar;  //����
IMAGE imgVegetarianCard[VEGETARAIN_COUNT];   //ֲ��ͼƬ����
IMAGE* imgVegetarian[VEGETARAIN_COUNT][20];   //ֲ������ΪVEGETARAIN_COUNT,����ֲ���ͼƬĬ��Ϊ20��
IMAGE imgSunshineBall[29];   //������

int curX, curY;  //��¼����X,Y����
int curPlant;    //��¼�����ֲ�������
int row, col;   //�к���
int sunshine; //����ֵ

bool fileExist(const char* name)
{
	FILE* fp = fopen(name, "r");
	if (fp == NULL)
	{
		return false;
	}
	else 
	{
		fclose(fp);   //��fclose����fp�ǿյ������,��ֹ��fclose���������ָ��!!
		return true;
	}

	//��һ��д��
	//if (fp)
	//{
	//	fclose(fp);
	//}
	//return fp!=NULL
	
}
void init()
{


	killCount = 0;  //��ʼ����ʬ��ɱ��
	zmCount = 0;   //��ʼ�����ֽ�ʬ
	gameStatus = GOING;   //��Ϸ״̬Ϊ����


	char name[64];
	initgraph(WIN_WIDTH, WIN_HEIGHT,1);
	//���ر���ͼƬ
	//ͨ���޸���Ŀ���Ե��ַ�������
	loadimage(&imgBg, "res/bg.jpg");

	//���ؿ���ͼƬ
	loadimage(&imgBar, "res/bar5.png");

	//�������ָ��Ķ�ά������г�ʼ������ֵΪ��ָ�룩
	memset(imgVegetarian, 0, sizeof(imgVegetarian));
	memset(map, 0, sizeof(map));  //��map��ʼ��

	for (int i = 0; i < VEGETARAIN_COUNT;i++)
	{
		//����ֲ�￨Ƭ,ֲ�￨Ƭ��λ��Ϊ"res/Card/card_1.png"����"res/Card/card_2.png"
		sprintf_s(name,sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgVegetarianCard[i], name);

		//����ֲ��
		//"res/zhiwu/0/1.png"����"res/zhiwu/0/13.png"  
		//'res/zhiwu/1/1.png"����"res/zhiwu/1/18.png"

		for (int j = 0; j < 20; j++)
		{

			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j+1);
			if (fileExist(name))
			{
				//����IMAGE����������C�����еı�׼����,�˴���ֵ������C++���﷨
				imgVegetarian[i][j] = new IMAGE;  
				//���ڳ�ʼ���ö�άָ�����鶼Ϊ��,�����Ҫ�ȸ�ֵ���ټ���ͼƬ,����û�пռ���Ը�ͼƬ����
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
	//�����������
	srand(time(NULL));
	update = false;
	curPlant = 0; //��ʾû��ֲ�������Ⱦ

	sunshine = 50;  //�������50

	//��������
	LOGFONT f;
	gettextstyle(&f);
	f.lfWidth = 15;
	f.lfHeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;  //�����Ч��
	settextstyle(&f);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);


	//��ʼ����ʬ
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgzm[i], name);
	}

	//��ʼ���ӵ�
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));


	//��ʼ����ը�ӵ�
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[i].getwidth() * k,
			imgBulletBlast[i].getheight() * k, true);
	}


	//���ؽ�ʬ������ͼƬ֡
	for (int i = 0; i < 20; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgzmDead[i], name);
	}

	//���ؽ�ʬ��ֲ���ͼƬ֡
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name,"res/zm_eat/%d.png",i+1);
		loadimage(&imgzmEat[i], name);
	}


	//���ؽ�ʬվ����ͼƬ֡
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
			//IMAGE* img = (zms[i].dead) ? imgzmDead : imgzm;  //�ı�ָ���ָ������ɶ����л�
			IMAGE* img = NULL;
			if (zms[i].dead)  img = imgzmDead;
			else if (zms[i].eating)  img = imgzmEat;
			else img = imgzm;

			img += zms[i].frameIndex;   //ָ����ƶ�������ָ�������ȷ��
			putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);
		}
	}
}


//����������
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
	outtextxy(276, 75, scoreText);  //�������

}



void drawcards()
{
	//��Ⱦ����
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
			if (bullets[i].blast)   //��ը��ʾ��ըЧ����ͼƬ
			{
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else    //������ʾ�����ӵ�����Ч����ͼƬ
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
}



/************
*���»���
*��������updateWindow
****************/
void updateWindow()
{
	BeginBatchDraw();     //��ʼ˫����
	putimage(-112, 0,&imgBg);         //���ر���ͼƬ

	putimagePNG(250, 0, &imgBar);  //���ؿ���ͼƬ

	drawcards();
	drawVegetarian();
	drawSunshines();


	drawZM();
	drawBullets();
	EndBatchDraw();    //����˫����
}

void createSunshine()
{


	//1.�������ȡһ������׹����
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
		balls[i].t = 0;     //���⿪ʼ��׹
		balls[i].p1 = vector2(260-112 + rand() % (900 - (260-112)), 60);   //���
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);  //�յ�
		int off = 2;  //�����½�ʱ�ƶ�������
		float distance = balls[i].p4.y - balls[i].p1.y;  //����
		balls[i].speed = 1.0 / (distance / off);   //�ٶ� 1.0��ʾʱ��
	}


	//2.������ػ�����տ�����������
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == SUNFLOWER + 1)  //Ѱ�ҵ�ͼ�ϵ����տ�
			{
				map[i][j].timer++;    //��ʱ����1
				if (map[i][j].timer > 200) {
					map[i][j].timer = 0;
					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);
					if (k >= ballMax) return;
					balls[k].used = true;
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);  //���
					int w = (100 + rand() % 51) * (rand() % 2 ? 1 : -1);  //�������ľ���
					balls[k].p4 = vector2(map[i][j].x + w,
						map[i][j].y + imgVegetarian[SUNFLOWER][0]->getheight()
						- imgSunshineBall[0].getheight());  //�յ�
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);  //���Ƶ�
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);  //���Ƶ�
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;   //�ٶ�
					balls[k].t = 0;   //��ʾ����
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
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;  //�ı�����֡
			if (balls[i].status == SUNSHINE_PRODUCT)   //�������տ������׶�
			{
				struct sunshineBall* sun = &balls[i];  //ָ��ָ��ǰ�������λ��
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1)
				{
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND)  //����̫�������״̬,��������2��״̬����
			{
				balls[i].timer++;
				if (balls[i].timer > 100)
				{
					balls[i].used = false;
					balls[i].timer = 0;
				}

			}
			else if (balls[i].status == SUNSHINE_COLLECT)  //̫�����ռ�״̬
			{
				struct sunshineBall* sun = &balls[i];  //ָ��ָ��ǰ�������λ��
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);  //p1+t*(p4-p1);
				if (sun->t > 1)
				{
					sun->used = false;
					sunshine += 25;  //̫��ֵ+25;
				}
			}
			else if (balls[i].status ==   SUNSHINE_DOWN)  //̫������׹״̬
			{
				struct sunshineBall* sun = &balls[i];  //ָ��ָ��ǰ�������λ��
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

			int x = balls[i].pCur.x;  //������ǰ��X����
			int y = balls[i].pCur.y;  //������ǰ��Y����
			if (msg->x > x && msg->x <x + w && msg->y >y && msg->y < y + h)
			{
				/*balls[i].used = false;*/
				balls[i].status = SUNSHINE_COLLECT;


				//mciSendString("play res/sunshine.mp3",0,0,0);
				PlaySound("res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC); //������Ч�Ľӿ�(��֧��MP3��ʽ)
				//������յ�
				/*float destX = 262;
				float destY = 0;

				float angle = atan((y - destY) / (x - destX));
				balls[i].xoff = 8 * cos(angle);
				balls[i].yoff = 8 * sin(angle);*/


				balls[i].p1 = balls[i].pCur;    //���
				balls[i].p4 = vector2(262, 0);  //�յ�
				balls[i].t = 0;  //��ʾ����ƶ�
				float distance = dis(balls[i].p1 - balls[i].p4);  //C++������  ȡģ
				float off = 8;  //�ƶ��Ĳ���
				balls[i].speed = 1.0 / (distance / off);
				break;
			}
		}
	}
}

//����ʬ�������߼�
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
			memset(&zms[i], 0, sizeof(zms[i])); //�Խ�ʬ��������
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].row = rand() % 3;   //��һ���н�ʬ
			zms[i].y = 172 + (1 + zms[i].row) * 100;
			zms[i].speed = 1;
			zms[i].blood = 100;
			zms[i].dead = false;    //��ʬ������ʼ��
			zms[i].frameIndex = 0;
			zmCount++;  //���ֽ�ʬ����+1
		}
	}
	
}


//���½�ʬ����Ϣ
void updataZM()
{
	int zmMax = sizeof(zms) / sizeof(zms[0]);
	static int count = 0;
	count++;
	if (count > 2*2)
	{
		count = 0;
		//���½�ʬ״̬
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)
			{
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 25)
				{
					zms[i].used = false;
					//printf("GAME OVER\n");
					//MessageBox(NULL, "over", "over", 0); //���԰汾
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
							gameStatus = WIN;  //��Ϸ�ж�Ϊʤ��
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

//�����ӵ�������
void  updateBullets()
{
	static int frecount = 0;
	if (++frecount < 2) return;
	frecount = 0;
	//�����ӵ��˶����ٶ�

	int count = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < count; i++)
	{
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}

			//�ӵ���ײ���
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
	//�����ӵ������Ƶ��

	int line[3] = { 0 };    //ս��
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int dangerX = WIN_WIDTH - imgzm[0].getwidth();   //��ɫ������
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


						int zwX = 256-112 + j * 81;  //ֲ�������
						int zwY = 179 + i * 102 + 14;  //ֲ��������
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
				//�ӵ��뽩ʬ������ͬһ�У����ӵ��뽩ʬ�����Ӵ�
			{
				zms[k].blood -= 20;
				bullets[i].blast = true;  //�ӵ���ը
				bullets[i].speed = 0;


				if (zms[k].blood <= 0)
				{
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;  //��ʾ�������̵�ͼƬ֡���
				}
				break;    //��ö�ӵ�����Ҫ��������ʬ������ײ���
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
			if (map[row][k].type == 0)continue;  //�жϸ��и�����û��ֲ��

			//��ý�ʬ��ֲ���������Ϣ,ͨ��ֲ��������������жϽ�ʬ�Ƿ���ֲ�﷢���ཻ
			int zhiwuX = 256-112 + k * 81;
			int x1 = zhiwuX+10;    //ֲ��ĺ�����1
			int x2 = zhiwuX + 60;  //ֲ��ĺ�����2
			int x3 = zms[i].x + 80; //��ʬ�ĺ�����
			if (x3 > x1 && x3 < x2)
			{
				if (map[row][k].catched) {
					/*zms[i].frameIndex++;*/
					map[row][k].deadtime++;
					if (map[row][k].deadtime > 100)
					{
						map[row][k].deadtime = 0;  //��ո��е��� ������ʱ 
						map[row][k].type = 0;      //ֲ������Ϊ��
						zms[i].eating = false;     //��ʬ��״̬�����ǳ�ֲ��
						zms[i].frameIndex = 0;     //��ʬ��֡����Ϊ0
						zms[i].speed = 1;      //�ָ���ʬ��ǰ��
						map[row][k].catched = false; //ֲ�ﱻ�Ժ���и���Ӧ����û�б������״̬
					}
					/*if (zms[i].frameIndex > 100) {
						
					}*/
				}
				else {
					map[row][k].catched = true;  //ֲ�ﱻֲ��Թ�
					map[row][k].deadtime = 0;   //ֲ��ս�����������ʱ
					zms[i].eating = true;    //��ʬ���ڳ�ֲ��
					zms[i].speed = 0;  //��ʬ��ֲ��ʱ�����ƶ�
					zms[i].frameIndex = 0;  //��ʬ��֡����
				}
			}
			

		}
	}
}


void collisionCheak()
{
	cheakBullet2Zm();  //�ӵ��뽩ʬ����ײ���
	cheakZm2Zhiwu();   //ֲ���뽩ʬ����ײ���
}

void updateVegetarian()
{
	static int count = 0;
	if (++count < 2)return;
	count = 0;

	//���͸���ֲ���Ƶ��

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
*��������updateGame
*���ã�������Ϸ��Ϣ
**********************/
void updateGame()
{

	updateVegetarian();


	createSunshine();  //��������
	updateSunshine();  //��������

	createZM();    //������ʬ
	updataZM();  //���½�ʬ

	collisionCheak();  //������

	shoot();       //���
	updateBullets();  //�����ӵ�


	
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


//������ĺ���
void click()
{
	ExMessage msg;           //�����Ϣ
	static int status = 0;   //������״̬
	if (peekmessage(&msg))   //�ж��Ƿ��������Ϣ
	{
		
		if (msg.message == WM_LBUTTONDOWN) //���ֲ��
		{
			if (msg.x > 338 && msg.x < 338 + 65* VEGETARAIN_COUNT && msg.y < 96) 
			{
				int index = (msg.x - 338) / 65;
				status = 1;   //�����������¼�
				curPlant = index + 1 ;
			}
			else
			{
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)   //����ƶ�ʱֲ����Ⱦ
		{
			curX = msg.x;
			curY = msg.y;
			//��¼����ƶ������꣬����ֲ���϶�
		}
		else if (msg.message == WM_LBUTTONUP&&status ==1)   //����ֲ����ֲ
		{
			if (msg.x > 256-112 && msg.y > 179 && msg.y < 489)
			{
				 col = (msg.x - (256-112)) / 81;   //�õ���
				 row = (msg.y - 179) / 102;  //�õ���

				if (map[row][col].type == 0)  //��ʾ���и���û��ֲ����ֲ
				{
					if (curPlant-1 == SUNFLOWER && sunshine >= 50)  //����ֵ�Ƿ��ܹ���ֲ���տ�
					{
						map[row][col].type = curPlant;
						
					}
					else if (curPlant-1 == PEA && sunshine >= 100)//����ֵ�Ƿ��ܹ���ֲ�㶹
					{
						map[row][col].type = curPlant;
					}
					else
					{
						status = 0;//״̬��λ
						curPlant = 0;  //ֲ��������0,��ʾû��ֲ��
						return;
					}

					map[row][col].frameIndex = 0;
					map[row][col].shootTimer = 0;
					
					reduceSunshine(curPlant);   //����ֲ������������ֵ

					map[row][col].x = 256-112 +  col * 81;   //���ֲ��λ��
				    map[row][col].y = 179 + row * 102 + 14;
				}
			}
			status = 0;//״̬��λ
			curPlant = 0;  //ֲ��������0,��ʾû��ֲ��
		}
	}
}

void startUI()
{
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;    //�ж���ʾ�Ǹ�ѡ��
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

	//��ʬ��վλ
	vector2 points[9] =
	{ {550,80},{530,160},{630,170},{530,200},{515,270},
		{566,370},{605,340},{750,280},{690,340} };   //vector2���������ָ����

	//ʹÿֻ��ʬ���������֡���
	int index[9];
	for (int i = 0; i < 9; i++)
	{
		index[i] = rand() % 11;
	}

	int count = 0;

//������ֵ��ƶ�
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
				index[k] = (index[k] + 1) % 11;  //����֡ͼƬ
			}
		}
		if (count >= 10)count = 0;  //����������
		EndBatchDraw();
		Sleep(5);
	}


	//ͣ��1s����
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
		Sleep(10);  //ͣ��3��
	}

//���������ƶ�
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
				index[k] = (index[k] + 1) % 11;  //����֡ͼƬ
			}
			if (count >= 10)count = 0;  //����������
		}
		
		EndBatchDraw();
		Sleep(5);
	}
}

//�����»�
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

		//��ʾ�����Ϳ���
		putimage(-112, 0, &imgBg);
		putimagePNG(250,y, &imgBar);


		//��ʾֲ�￨��
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

