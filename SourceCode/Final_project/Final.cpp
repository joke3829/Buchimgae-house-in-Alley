#pragma comment (lib, "msimg32.lib")
#include<windows.h>
#include<ctime>
#include"fmod.hpp"
#include"fmod_errors.h"
#include"resource.h"
#define IDC_BUTTON1 101

HWND hWnd, c_hWnd;
HBITMAP explain;
bool on_explain;
FMOD::System* ssystem;
FMOD::Sound* s_bad_end, * correct, * incorrect, * s_laddle, * hit_sound, * stage1, * stage3, * scene_sound, * nurvous
, * fried, * grab_pan, * secret_hit, * stage2, * s_title, * ending;
FMOD::Channel* channel = 0;	//�������
FMOD::Channel* channel1 = 0;	//ȿ����
FMOD::Channel* channel2 = 0;	//���� �Ҹ�
FMOD::Channel* channel3 = 0;	//ȯ����
FMOD_RESULT result;
void* extradriverdata = 0;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"��� ����";

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChildProc(HWND c_hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

struct LADLE {	//���� ����ü
	int x, y;	//������ ��ġ
	HBITMAP ladle;	//���� ��Ʈ��
};
LADLE ladle_03;

struct FIREBOARD {
	bool used;	//�ÿ����ΰ�?
	int vari;	//���� ���� = P_select
	HBITMAP pancake; //�� ��Ʈ��
	int count;	//�󸶵��� �����߳�
	RECT range;	//��� Ŭ���ؾ� ���õǳ�
	int x, y;	//���� ��ġ
	int width, height;	//�� ũ��
};

struct FALLINGPOT {
	int x, y;	//���� ��ġ
	bool falling;	//�������� �ֳ�?
	bool crash;		//�浹�߳�?
	HBITMAP pot;
};
FIREBOARD f_board_03[8];
FIREBOARD Stack_03[10];
FALLINGPOT falling_pot_03;

struct ORDER {
	int vari;	//�մ��� ����1. �Ϲ���, 2. ���ز�, 3. ������
	bool place; //�ڸ��� �����߳�?(���� �ö�Գ�)
	int x, y;	//�մ��� ��ġ
	HBITMAP face;	//�մ� ��Ʈ��
	double face_count;	//�ִϸ��̼� ī��Ʈ
	int en_count;	//��ƾ �ð�	15�� �� ��ȯ
	int or_top;		//�������� �ֹ��߳�?
	FIREBOARD stack[10];	//�� �ֹ��߳�?
	int complete = 0; //��� �Ϸ��߳�?
	int complete_face = 0; //��� �Ϸ����� �� ǥ��
	HBITMAP score; //���� ��Ʈ��
	int score_x = 0; //���� ��ġ
	int score_y = 0; //���� ��ġ
	int c_06 = 0;
	bool hit_pot;	//���� �¾ҳ�?
	bool get_dish; //������ �޾ҳ�?
	bool correct; //�¾ҳ�?
	bool youangry; //���� ȭ����?
};
ORDER order[3];	//0���� �մ�, 1, �߾Ӽմ�, 2. ������ �մ�


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE PrevhInstance, LPSTR lpszCmdParam, int nCmdShow) {
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.hInstance = hInstance;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	WndClass.lpszClassName = L"ChildClass";
	WndClass.lpfnWndProc = ChildProc;
	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 1024, 768, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	srand(time(NULL));
	PAINTSTRUCT ps_03;
	static RECT rt_03;
	static HDC hDC_03, mDC1_03, mDC2_03;	//DC��
	static HBITMAP hBit_03, background_03[7], oldBit_03[2];	//hBit, ���, oldBit
	static HBITMAP fire_03[4], c_board_03, dish_03, pot_03;	//����, ����, ����, ����
	static HBITMAP score_board_03, score_text_03, score_num_03[10];	//������, ���� �ؽ�Ʈ, 0~9
	static HBITMAP life_board_03, heart_03[2];	//ü�� ��, ��Ʈ
	static HBITMAP talkBox_03, pancake_03[12], hit_pot_03, minus_heart_03;		//��ǳ��, ���� ����, �ε��� ����, ��Ʈ ����
	static HBITMAP order_face_03[9];	//�մ� ��
	static HBITMAP order_check_06[3]; //�մ� ���뿡 ���� ����
	static HBITMAP villain_03[5];	//������ �� 0. ���� 1~2. ���� 3~4. ����
	static HBITMAP master_03[8];	//������ ������ ����������� 0~2 ��� 3~7������
	static HBITMAP camera_03, light_03;	//ī�޶�� ����

	static int stage_03, fire_count_03;	//���° ��������, ���� �ִϸ��̼� ī��Ʈ
	static int P_select_03, P_top_03, score_03;	//���� ���� �����, ���ÿ� ���� �󸶳� �׾ҳ�? ����
	static int mx_03, my_03, cal_score_03, sub_i_03;	//���콺 ��ǥ, ���� ���� ����, ��������
	static int life_03;	//���� ü��
	static bool grab_pot_03;	//���� ��Ҵ°�?
	static int count_06 = 0; //�մ� �ֹ��� ���� ���� �丮�� ���� �´��� Ȯ�ο�
	static bool exist_v_03, isTimer5003;	//���ز��� �����ϳ�?, 5003Ÿ�̸Ӱ� �۵����ΰ�
	static double v_percent_03, v_check_03;	//���ز� ���� Ȯ��, ���ز� Ȯ�� üũ
	static int success_03;	//�ֹ� ���� Ƚ��

	switch (Message) {
	case WM_CREATE:
		stage_03 = 0;	//������ 1��������
		P_top_03 = 0;
		{
			heart_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP36));	//������Ʈ
			heart_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP35));	//�μ��� ��Ʈ
		}
		{//��� ��Ʈ�� ����
			background_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));	//Ÿ��Ʋ
			background_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));	//������
			background_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP70));	//������
			background_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP71));	//������
			background_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP72));	//���� ����
			background_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP51));	//��� ����
			background_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP69));	//��� ����2
		}
		{//���� �ؽ�Ʈ
			score_num_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP25));
			score_num_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP26));
			score_num_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP27));
			score_num_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP28));
			score_num_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP29));
			score_num_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP30));
			score_num_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP31));
			score_num_03[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP32));
			score_num_03[8] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP33));
			score_num_03[9] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP34));
		}
		{
			fire_count_03 = 0;
			fire_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
			fire_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
			fire_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP4));
			fire_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));
		}//���� ��Ʈ��
		{
			c_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));	//����
			dish_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));	//����
			ladle_03.ladle = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));	//����
			pot_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));	//����
			score_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP23));	//������
			score_text_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP24));	//���� �ؽ�Ʈ
			life_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP37));	//��Ʈ ��
			talkBox_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP39));	//��ǳ��
			hit_pot_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP47));	//�μ��� ����
			minus_heart_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP50));	//��Ʈ ����
			explain = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP73));
		}
		{//���� ���� 0~2 ��ġ 3~5 ���� 6~8 ���� 9~11 �޹� 
			pancake_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP14));
			pancake_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP15));
			pancake_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP16));
			pancake_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP17));
			pancake_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP18));
			pancake_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP19));
			pancake_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP20));
			pancake_03[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP21));
			pancake_03[8] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP22));
			pancake_03[9] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP11));
			pancake_03[10] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP12));
			pancake_03[11] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP13));
		}
		{//�մ� �� 0��� 1ȭ�� 2�г� 3 �Դ���1, 4 �Դ��� 2, 5����1, 6. ����2
			order_face_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP41));
			order_face_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP40));
			order_face_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP38));
			order_face_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP42));
			order_face_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP43));
			order_face_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP48));
			order_face_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP49));
			order_face_03[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP67));
			order_face_03[8] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP68));
		}
		{
			order_check_06[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP46));
			order_check_06[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP45));
			order_check_06[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP44));
		}
		{
			villain_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP52));
			villain_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP53));
			villain_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP54));
			villain_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP55));
			villain_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP56));
		}
		{
			camera_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP58));
			light_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP57));
		}
		{
			master_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP59));
			master_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP60));
			master_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP61));
			master_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP62));
			master_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP63));
			master_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP64));
			master_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP65));
			master_03[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP66));
		}
		for (int i = 0; i < 8; ++i) {//���� ���� ����
			f_board_03[i].used = false;
			if (i < 4) {
				f_board_03[i].range.left = f_board_03[i].x = 66 + (i * 119);
				f_board_03[i].range.right = 66 + ((i + 1) * 119);
				f_board_03[i].range.top = 465; f_board_03[i].range.bottom = 582;
				f_board_03[i].y = 495;
			}
			else {
				f_board_03[i].range.left = f_board_03[i].x = 66 + ((i - 4) * 119);
				f_board_03[i].range.right = 66 + ((i - 3) * 119);
				f_board_03[i].range.top = 582; f_board_03[i].range.bottom = 699;
				f_board_03[i].y = 612;
			}
			f_board_03[i].width = 117; f_board_03[i].height = 60;
		}
		for (int i = 0; i < 3; ++i) {//�մ� �ʱ� ����
			order[i].vari = 1;
			order[i].y = (i * 2000) + 500;
			order[i].x = (i * 322) + 38;
			order[i].score_x = (i * 322) + 45;
			order[i].score_y = 300;
			order[i].place = false;
			order[i].hit_pot = false;
			order[i].face = order_face_03[0];
			order[i].youangry = false;
			for (int j = 0; j < 10; ++j) {
				order[i].stack[j].x = (i * 320) + 225;
				order[i].stack[j].y = 210 - (j * 15);
				order[i].stack[j].width = 117;
				order[i].stack[j].height = 40;
			}
		}
		score_03 = 0;
		life_03 = 3;
		P_select_03 = 1;	//��ġ�� ����(Ű�Է����� �ٲ�)
		ladle_03.x = 670; ladle_03.y = 425;	//�ʱ� ���� ��ġ
		grab_pot_03 = false;
		v_percent_03 = 0;
		success_03 = 0;
		isTimer5003 = false;
		on_explain = false;
		{
			falling_pot_03.crash = false;
			falling_pot_03.falling = false;
			falling_pot_03.x = falling_pot_03.y = -1000;
			falling_pot_03.pot = pot_03;
		}
		result = FMOD::System_Create(&ssystem);
		if (result != FMOD_OK)
			exit(0);

		ssystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
		ssystem->createSound("sound\\1997 spring.mp3",
			FMOD_LOOP_NORMAL, 0, &s_bad_end);	//��忣�� ����
		ssystem->createSound("sound\\correct.mp3",
			FMOD_DEFAULT, 0, &correct);		//�­����� ����
		ssystem->createSound("sound\\incorrect.mp3",
			FMOD_DEFAULT, 0, &incorrect);	//Ʋ������ ����
		ssystem->createSound("sound\\laddle.mp3",
			FMOD_DEFAULT, 0, &s_laddle);	//���� ����
		ssystem->createSound("sound\\pot_hit.mp3",
			FMOD_DEFAULT, 0, &hit_sound);	//���� ����
		ssystem->createSound("sound\\stage1.mp3", FMOD_LOOP_NORMAL, 0, &stage1);
		ssystem->createSound("sound\\stage3.mp3", FMOD_LOOP_NORMAL, 0, &stage3);
		ssystem->createSound("sound\\scene sound.mp3", FMOD_LOOP_NORMAL, 0, &scene_sound);
		ssystem->createSound("sound\\nurvous.mp3", FMOD_LOOP_NORMAL, 0, &nurvous);
		ssystem->createSound("sound\\fried.mp3", FMOD_DEFAULT, 0, &fried);
		ssystem->createSound("sound\\grab_pan.mp3", FMOD_DEFAULT, 0, &grab_pan);
		ssystem->createSound("sound\\secret_hit.mp3", FMOD_DEFAULT, 0, &secret_hit);
		ssystem->createSound("sound\\stage2.mp3", FMOD_LOOP_NORMAL, 0, &stage2);
		ssystem->createSound("sound\\title.mp3", FMOD_LOOP_NORMAL, 0, &s_title);
		ssystem->createSound("sound\\ending.mp3", FMOD_LOOP_NORMAL, 0, &ending);
		channel->stop();	//����� ä��
		channel1->stop();	//ȿ���� ä��
		channel2->stop();	//���� �̵��Ҹ� ä��
		channel3->stop();
		ssystem->playSound(s_title, 0, false, &channel);
		channel->setVolume(0.3);	//����� ä��
		break;
	case WM_PAINT:
		GetClientRect(hWnd, &rt_03);
		hDC_03 = BeginPaint(hWnd, &ps_03);
		hBit_03 = CreateCompatibleBitmap(hDC_03, rt_03.right, rt_03.bottom);
		mDC1_03 = CreateCompatibleDC(hDC_03);
		mDC2_03 = CreateCompatibleDC(mDC1_03);
		oldBit_03[0] = (HBITMAP)SelectObject(mDC1_03, hBit_03);
		//���������� ���� ��� ���� 0 ������, 1~3 ������, 4~����, 5��忣��, 6. ���2
		{
			if (stage_03 == 0) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[0]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//��� �ֱ�
			}
			else if (stage_03 == 1) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[1]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//��� �ֱ�
			}
			else if (stage_03 == 2) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[2]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//��� �ֱ�
			}
			else if (stage_03 == 3) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[3]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//��� �ֱ�
			}
			else if (stage_03 == 4) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[4]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//��� �ֱ�

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 373, 580, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //���� �ؽ�Ʈ �ֱ�
				if (score_03 >= 99999) {
					score_03 = 99999;
				}
				cal_score_03 = score_03;
				sub_i_03 = 0;
				for (int i = 10000; i >= 1; i /= 10) {
					switch (cal_score_03 / i) {
					case 0:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[0]);
						break;
					case 1:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[1]);
						break;
					case 2:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[2]);
						break;
					case 3:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[3]);
						break;
					case 4:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[4]);
						break;
					case 5:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[5]);
						break;
					case 6:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[6]);
						break;
					case 7:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[7]);
						break;
					case 8:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[8]);
						break;
					case 9:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[9]);
						break;
					}
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 488, 580, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //���� �ؽ�Ʈ �ֱ�
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}
			else if (stage_03 == 5) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[5]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//��� �ֱ�

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 360, 400, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //���� �ؽ�Ʈ �ֱ�
				if (score_03 >= 99999) {
					score_03 = 99999;
				}
				cal_score_03 = score_03;
				sub_i_03 = 0;
				for (int i = 10000; i >= 1; i /= 10) {
					switch (cal_score_03 / i) {
					case 0:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[0]);
						break;
					case 1:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[1]);
						break;
					case 2:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[2]);
						break;
					case 3:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[3]);
						break;
					case 4:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[4]);
						break;
					case 5:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[5]);
						break;
					case 6:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[6]);
						break;
					case 7:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[7]);
						break;
					case 8:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[8]);
						break;
					case 9:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[9]);
						break;
					}
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 475, 400, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //���� �ؽ�Ʈ �ֱ�
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}
			else if (stage_03 == 6) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[6]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//��� �ֱ�

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 360, 50, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //���� �ؽ�Ʈ �ֱ�
				if (score_03 >= 99999) {
					score_03 = 99999;
				}
				cal_score_03 = score_03;
				sub_i_03 = 0;
				for (int i = 10000; i >= 1; i /= 10) {
					switch (cal_score_03 / i) {
					case 0:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[0]);
						break;
					case 1:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[1]);
						break;
					case 2:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[2]);
						break;
					case 3:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[3]);
						break;
					case 4:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[4]);
						break;
					case 5:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[5]);
						break;
					case 6:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[6]);
						break;
					case 7:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[7]);
						break;
					case 8:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[8]);
						break;
					case 9:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[9]);
						break;
					}
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 475, 50, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //���� �ؽ�Ʈ �ֱ�
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}

			if (stage_03 >= 1 && stage_03 <=3) {
				{//�մ� ���
					for (int i = 0; i < 3; ++i) {
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, order[i].face);
						TransparentBlt(mDC1_03, order[i].x, order[i].y, 167, 260, mDC2_03, 0, 0, 167, 260, RGB(34, 177, 76));//�մ� ���
						if ((order[i].vari == 1 || order[i].vari == 3) && order[i].y == 180) {
							if (order[i].place && order[i].y == 180 && !order[i].hit_pot) {
								oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, talkBox_03);
								TransparentBlt(mDC1_03, (i * 320) + 205, 130, 155, 150,
									mDC2_03, 0, 0, 155, 150, RGB(34, 177, 76));//��ǳ�� ���

								oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, dish_03);
								TransparentBlt(mDC1_03, (i * 320) + 220, 210, 127, 50,
									mDC2_03, 0, 0, 127, 54, RGB(34, 177, 76));//���� ���
								for (int j = 0; j < order[i].or_top; ++j) {
									oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, order[i].stack[j].pancake);
									TransparentBlt(mDC1_03, order[i].stack[j].x, order[i].stack[j].y, order[i].stack[j].width, order[i].stack[j].height,
										mDC2_03, 0, 0, 380, 209, RGB(34, 177, 76));
								}
							}
						}
						if (order[i].complete == 1) {
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, order[i].score);
							TransparentBlt(mDC1_03, order[i].score_x, order[i].score_y, 140, 50,
								mDC2_03, 0, 0, 140, 50, RGB(34, 177, 76));
						}
					}
				}
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, falling_pot_03.pot);
				TransparentBlt(mDC1_03, falling_pot_03.x, falling_pot_03.y, 167, 50, mDC2_03, 0, 0, 241, 114,RGB(34, 177, 76));	//�������� ���� �ֱ�
				
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, fire_03[fire_count_03]);
				StretchBlt(mDC1_03, 0, 440, 740, 289, mDC2_03, 0, 0, 740, 289, SRCCOPY);	//���� �ֱ�

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, c_board_03);
				StretchBlt(mDC1_03, 740, 440, 268, 289, mDC2_03, 0, 0, 268, 289, SRCCOPY);	//���� �ֱ�

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, pot_03);
				TransparentBlt(mDC1_03, 740, 440, 268, 145, mDC2_03, 0, 0, 241, 114, RGB(34, 177, 76)); //���� �ֱ�

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, dish_03);
				TransparentBlt(mDC1_03, 740, 584, 268, 145, mDC2_03, 0, 0, 127, 54, RGB(34, 177, 76)); //���� �ֱ�

				{//���� ǥ��
					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_board_03);
					TransparentBlt(mDC1_03, 0, 0, 300, 60, mDC2_03, 0, 0, 300, 60, RGB(34, 177, 76)); //������ �ֱ�

					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);
					TransparentBlt(mDC1_03, 10, 15, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //���� �ؽ�Ʈ �ֱ�
					if (score_03 >= 99999) {
						score_03 = 99999;
					}
					cal_score_03 = score_03;
					sub_i_03 = 0;
					for (int i = 10000; i >= 1; i /= 10) {
						switch (cal_score_03 / i) {
						case 0:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[0]);
							break;
						case 1:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[1]);
							break;
						case 2:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[2]);
							break;
						case 3:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[3]);
							break;
						case 4:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[4]);
							break;
						case 5:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[5]);
							break;
						case 6:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[6]);
							break;
						case 7:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[7]);
							break;
						case 8:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[8]);
							break;
						case 9:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[9]);
							break;
						}
						TransparentBlt(mDC1_03, (sub_i_03 * 30) + 125, 15, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //���� �ؽ�Ʈ �ֱ�
						cal_score_03 -= (cal_score_03 / i) * i;
						sub_i_03++;
					}
				}
				{//��Ʈ ǥ��
					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, life_board_03);
					TransparentBlt(mDC1_03, 838, 0, 170, 60, mDC2_03, 0, 0, 170, 60, RGB(34, 177, 76)); //ü�� �� �ֱ�
					switch (life_03) {
					case 0:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[1]);
						TransparentBlt(mDC1_03, 843, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 898, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 953, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						break;
					case 1:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[1]);
						TransparentBlt(mDC1_03, 843, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 898, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[0]);
						TransparentBlt(mDC1_03, 953, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						break;
					case 2:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[1]);
						TransparentBlt(mDC1_03, 843, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[0]);
						TransparentBlt(mDC1_03, 898, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 953, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						break;
					case 3:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[0]);
						TransparentBlt(mDC1_03, 843, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 898, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 953, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						break;
					}
				}

				for (int i = 0; i < P_top_03; ++i) {
					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, Stack_03[i].pancake);
					TransparentBlt(mDC1_03, Stack_03[i].x, Stack_03[i].y, Stack_03[i].width, Stack_03[i].height, mDC2_03, 0, 0, 380, 209, RGB(34, 177, 76)); //���� �ֱ�
				}

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, ladle_03.ladle);
				TransparentBlt(mDC1_03, ladle_03.x, ladle_03.y, 70, 100, mDC2_03, 0, 0, 104, 222, RGB(34, 177, 76));	//���� �ֱ�

				for (int i = 0; i < 8; ++i) {
					if (f_board_03[i].used) {
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, f_board_03[i].pancake);
						TransparentBlt(mDC1_03, f_board_03[i].x, f_board_03[i].y, f_board_03[i].width, f_board_03[i].height,
							mDC2_03, 0, 0, 380, 209, RGB(34, 177, 76));	//���ǿ� �� �ֱ�
					}
				}
			}
		}
		BitBlt(hDC_03, 0, 0, rt_03.right, rt_03.bottom, mDC1_03, 0, 0, SRCCOPY);
		SelectObject(mDC1_03, oldBit_03[0]);
		DeleteObject(hBit_03);
		DeleteDC(mDC2_03);
		DeleteDC(mDC1_03);
		EndPaint(hWnd, &ps_03);
		break;
	case WM_TIMER:
		switch (wParam) {
		case 1003: {//���� �ִϸ��̼�
			fire_count_03++;
			fire_count_03 %= 4;
			break;
		}
		case 2003: {//���� �� ���� ī��Ʈ ����
			for (int i = 0; i < 8; ++i) {
				if (f_board_03[i].used) {
					f_board_03[i].count++;
					if (f_board_03[i].count == 7) {
						switch (f_board_03[i].vari) {
						case 1:
							f_board_03[i].pancake = pancake_03[1];
							break;
						case 2:
							f_board_03[i].pancake = pancake_03[4];
							break;
						case 3:
							f_board_03[i].pancake = pancake_03[7];
							break;
						case 4:
							f_board_03[i].pancake = pancake_03[10];
							break;
						}
					}
					else if (f_board_03[i].count == 17) {
						switch (f_board_03[i].vari) {
						case 1:
							f_board_03[i].pancake = pancake_03[2];
							break;
						case 2:
							f_board_03[i].pancake = pancake_03[5];
							break;
						case 3:
							f_board_03[i].pancake = pancake_03[8];
							break;
						case 4:
							f_board_03[i].pancake = pancake_03[11];
							break;
						}
					}
				}
			}
			break;
		}
		case 3003: {//������Ʈ
			if (falling_pot_03.crash) {
				falling_pot_03.crash = false;
				falling_pot_03.x = falling_pot_03.y = -1000;
			}
			KillTimer(hWnd, 3003);
			break;
		}
		case 4003: {//1�� �մ� ���� ī��Ʈ
			if (order[0].hit_pot) {
				order[0].score_y -= 10;
				if (order[0].vari == 1) {
					if (order[0].c_06 % 2 == 0)
						order[0].face = order_face_03[5];
					else if (order[0].c_06 % 2 == 1) {
						order[0].face = order_face_03[6];
					}
				}
				else if (order[0].vari == 2) {
					if (order[0].c_06 % 2 == 0)
						order[0].face = villain_03[1];
					else if (order[0].c_06 % 2 == 1) {
						order[0].face = villain_03[2];
					}
				}
				order[0].c_06++;
				if (order[0].c_06 >= 4) {
					order[0].hit_pot = false;
					order[0].c_06 = 0;
					if (order[0].vari != 4) {
						order[0].y += 40;
						order[0].face_count = 100;
						order[0].complete = 0;
						order[0].score_y = 300;
						order[0].youangry = true;
						if (order[0].vari == 1)
							life_03--;
					}
					KillTimer(hWnd, 4003);
				}
			}
			else if (order[0].get_dish) { //�ֹ��� �޴��� �޾��� ��
				if (order[0].correct == true) { //�¾��� ��
					if (order[0].vari == 1) {
						if (order[0].c_06 % 2 == 0)
							order[0].face = order_face_03[3];
						else if (order[0].c_06 % 2 == 1) {
							order[0].face = order_face_03[4];
						}
					}
					else if (order[0].vari == 2) {
						if (order[0].c_06 % 2 == 0)
							order[0].face = villain_03[3];
						else if (order[0].c_06 % 2 == 1) {
							order[0].face = villain_03[4];
						}
					}
					order[0].score_y -= 10; //�󱼿� ���� ����ǥ ���
					if (order[0].c_06 == 4) { //4�� �Ʊ��۾Ʊ����ϰ� ����
						if (order[0].vari == 1) {
							order[0].c_06 = 0;
							order[0].face = order_face_03[0]; //ó�� ǥ������ ���������� ����
							order[0].face_count = 100; //�ٷ� �ϰ��ϵ���
							order[0].complete = 0; //�Ϸ� ó�� ���Ѱɷ� ����
							order[0].score_y = 300; //���� ���� ��ǥ���� ���
							//order[0].get_dish = false;
							order[0].correct = false;
							success_03++;
							if (success_03 >= 10 && stage_03 == 1) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							else if (success_03 >= 15 && stage_03 == 2) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							else if (success_03 >= 18 && stage_03 == 3) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							KillTimer(hWnd, 4003);
						}
						else if (order[0].vari == 2) {
							order[0].face = villain_03[0]; //ó�� ǥ��
							order[0].get_dish = false;
							order[0].correct = false;
							order[0].complete = 0;
							order[0].score_y = 300; //���� ���� ��ǥ���� ���
							KillTimer(hWnd, 4003);
						}
					}
					order[0].c_06++;
				}
				else { //�߸� ���� ��
					if (order[0].vari == 1) {
						if (order[0].c_06 % 2 == 0)
							order[0].face = order_face_03[7];
						else if (order[0].c_06 % 2 == 1) {
							order[0].face = order_face_03[8];
						}
					}
					else if (order[0].vari == 2) {
						if (order[0].c_06 % 2 == 0)
							order[0].face = villain_03[3];
						else if (order[0].c_06 % 2 == 1) {
							order[0].face = villain_03[4];
						}
					}
					order[0].c_06++;
					order[0].score_y -= 10; //�󱼿� ���� ����ǥ ���
					if (order[0].c_06 >= 4) {
						if (order[0].vari == 1) {
							order[0].c_06 = 0;
							order[0].face = order_face_03[2]; //ȭ�� ǥ������ ���������� ����
							order[0].face_count = 100; //�ٷ� �ϰ��ϵ���
							order[0].complete = 0; //�Ϸ� ó�� ���Ѱɷ� ����
							//order[0].get_dish = false;
							order[0].correct = false;
							order[0].score_y = 300; //���� ���� ��ǥ���� ���
							if (order[0].vari == 1) {
								life_03--;
							}
							KillTimer(hWnd, 4003);
						}
						else if (order[0].vari == 2) {
							order[0].face = villain_03[0]; //ó�� ǥ��
							order[0].get_dish = false;
							order[0].correct = false;
							order[0].score_y = 300; //���� ���� ��ǥ���� ���
							order[0].complete = 0;
							KillTimer(hWnd, 4003);
						}
					}
				}
			}
			break;
		}
		case 5003: {//2�� �մ� ���� ī��Ʈ
			if (order[1].vari == 3) {	//�������϶�
				order[1].face = master_03[order[1].c_06 + 3];
				order[1].c_06++;
				if (order[1].c_06 >= 6) {
					if (order[1].correct) {//������ �������� up
						stage_03++;
						if (stage_03 == 2) {
							channel->stop();
							channel1->stop();
							channel2->stop();
							channel3->stop();
							ssystem->playSound(stage2, 0, false, &channel);
							ssystem->playSound(scene_sound, 0, false, &channel3);
							channel->setVolume(0.5);
							channel3->setVolume(0.35);
						}
						else if (stage_03 == 3) {
							channel->stop();
							channel1->stop();
							channel2->stop();
							channel3->stop();
							ssystem->playSound(stage3, 0, false, &channel);
							channel->setVolume(0.5);
							ssystem->playSound(scene_sound, 0, false, &channel3);
							channel3->setVolume(0.35);
						}
						else if (stage_03 == 4) {
							channel->stop();
							channel1->stop();
							channel2->stop();
							channel3->stop();
							ssystem->playSound(ending, 0, false, &channel);
							channel->setVolume(0.3);

						}
						if (life_03 < 3)
							life_03++;
						P_top_03 = 0;	//���� �ʱ�ȭ
						for (int i = 0; i < 8; ++i) {//���� �ʱ�ȭ
							f_board_03[i].used = false;
						}
						for (int i = 0; i < 3; ++i) {//�մ� �ʱ� ����
							order[i].vari = 1;
							order[i].y = (i * 2000) + 500;
							order[i].x = (i * 322) + 38;
							order[i].score_x = (i * 322) + 45;
							order[i].score_y = 300;
							order[i].place = false;
							order[i].hit_pot = false;
							order[i].youangry = false;
							order[i].face = order_face_03[0];
							order[i].get_dish = false;
						}
						P_select_03 = 1;	//��ġ�� ����(Ű�Է����� �ٲ�)
						ladle_03.x = 670; ladle_03.y = 425;	//�ʱ� ���� ��ġ
						grab_pot_03 = false;
						v_percent_03 = 0;
						success_03 = 0;
						isTimer5003 = false;
						{
							falling_pot_03.crash = false;
							falling_pot_03.falling = false;
							falling_pot_03.x = falling_pot_03.y = -1000;
							falling_pot_03.pot = pot_03;
						}
						KillTimer(hWnd, 5003);
						if (stage_03 != 4) {
							SetTimer(hWnd, 1003, 200, NULL);//���� Ÿ�̸�
							SetTimer(hWnd, 2003, 1000, NULL);	//�� ���� ī��Ʈ
							SetTimer(hWnd, 1006, 40, NULL);	//�մ� �̵�
							SetTimer(hWnd, 2006, 1000, NULL);	//�մ��� ���ð�
						}
					}
					else {	//Ʋ���� ����
						stage_03 = 6;
						channel->stop();
						channel1->stop();
						channel2->stop();
						ssystem->playSound(s_bad_end, 0, false, &channel);
						channel->setVolume(0.5);
						KillTimer(hWnd, 1003);
						KillTimer(hWnd, 2003);
						KillTimer(hWnd, 3003);
						KillTimer(hWnd, 4003);
						KillTimer(hWnd, 5003);
						KillTimer(hWnd, 6003);

						KillTimer(hWnd, 1006);
						KillTimer(hWnd, 2006);
						KillTimer(hWnd, 3006);
					}
				}
			}
			else if (order[1].hit_pot) {
				order[1].score_y -= 10;
				if (order[1].vari == 1) {
					if (order[1].c_06 % 2 == 0)
						order[1].face = order_face_03[5];
					else if (order[1].c_06 % 2 == 1) {
						order[1].face = order_face_03[6];
					}
				}
				else if (order[1].vari == 2) {
					if (order[1].c_06 % 2 == 0)
						order[1].face = villain_03[1];
					else if (order[1].c_06 % 2 == 1) {
						order[1].face = villain_03[2];
					}
				}
				order[1].c_06++;
				if (order[1].vari != 3) {
					if (order[1].c_06 >= 4) {
						order[1].y += 40;
						order[1].face_count = 100;
						order[1].hit_pot = false;
						order[1].complete = 0;
						order[1].score_y = 300;
						order[1].youangry = true;
						if (order[1].vari == 1)
							life_03--;
						KillTimer(hWnd, 5003);
					}
				}
			}
			else if (order[1].get_dish) { //�ֹ��� �޴��� �޾��� ��
				if (order[1].correct == true) { //�¾��� ��
					if (order[1].vari == 1) {
						if (order[1].c_06 % 2 == 0)
							order[1].face = order_face_03[3];
						else if (order[1].c_06 % 2 == 1) {
							order[1].face = order_face_03[4];
						}
					}
					else if (order[1].vari == 2) {
						if (order[1].c_06 % 2 == 0)
							order[1].face = villain_03[3];
						else if (order[1].c_06 % 2 == 1) {
							order[1].face = villain_03[4];
						}
					}
					order[1].score_y -= 10; //�󱼿� ���� ����ǥ ���
					if (order[1].c_06 == 4) { //4�� �Ʊ��۾Ʊ����ϰ� ����
						if (order[1].vari == 1) {
							order[1].c_06 = 0;
							order[1].face = order_face_03[0]; //ó�� ǥ������ ���������� ����
							order[1].face_count = 100; //�ٷ� �ϰ��ϵ���
							order[1].complete = 0; //�Ϸ� ó�� ���Ѱɷ� ����
							order[1].score_y = 300; //���� ���� ��ǥ���� ���
							//order[1].get_dish = false;
							order[1].correct = false;
							success_03++;
							if (success_03 >= 10 && stage_03 == 1) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							else if (success_03 >= 15 && stage_03 == 2) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							else if (success_03 >= 18 && stage_03 == 3) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							KillTimer(hWnd, 5003);
						}
						else if (order[1].vari == 2) {
							order[1].face = villain_03[0]; //ó�� ǥ��
							order[1].get_dish = false;
							order[1].correct = false;
							order[1].complete = 0;
							order[1].score_y = 300; //���� ���� ��ǥ���� ���
							KillTimer(hWnd, 5003);
						}
					}
					order[1].c_06++;
				}
				else { //�߸� ���� ��
					if (order[1].vari == 1) {
						if (order[1].c_06 % 2 == 0)
							order[1].face = order_face_03[7];
						else if (order[1].c_06 % 2 == 1) {
							order[1].face = order_face_03[8];
						}
					}
					else if (order[1].vari == 2) {
						if (order[1].c_06 % 2 == 0)
							order[1].face = villain_03[3];
						else if (order[1].c_06 % 2 == 1) {
							order[1].face = villain_03[4];
						}
					}
					order[1].c_06++;
					order[1].score_y -= 10; //�󱼿� ���� ����ǥ ���
					if (order[1].c_06 >= 4) {
						if (order[1].vari == 1) {
							order[1].c_06 = 0;
							order[1].face = order_face_03[2]; //ȭ�� ǥ������ ���������� ����
							order[1].face_count = 100; //�ٷ� �ϰ��ϵ���
							order[1].complete = 0; //�Ϸ� ó�� ���Ѱɷ� ����
							//order[1].get_dish = false;
							order[1].correct = false;
							order[1].score_y = 300; //���� ���� ��ǥ���� ���
							life_03--;
							KillTimer(hWnd, 5003);
						}
						else if (order[1].vari == 2) {
							order[1].face = villain_03[0]; //ó�� ǥ��
							order[1].get_dish = false;
							order[1].correct = false;
							order[1].complete = 0;
							order[1].score_y = 300; //���� ���� ��ǥ���� ���
							KillTimer(hWnd, 5003);
						}
					}
				}
			}
			break;
		}
		case 6003: {//3�� �մ� ���� ī��Ʈ
			if (order[2].hit_pot) {
				order[2].score_y -= 10;
				if (order[2].vari == 1) {
					if (order[2].c_06 % 2 == 0)
						order[2].face = order_face_03[5];
					else if (order[2].c_06 % 2 == 1) {
						order[2].face = order_face_03[6];
					}
				}
				else if (order[2].vari == 2) {
					if (order[2].c_06 % 2 == 0)
						order[2].face = villain_03[1];
					else if (order[2].c_06 % 2 == 1) {
						order[2].face = villain_03[2];
					}
				}
				order[2].c_06++;
				if (order[2].c_06 >= 4) {
					order[2].hit_pot = false;
					order[2].c_06 = 0;
					if (order[2].vari != 5) {
						order[2].y += 40;
						order[2].face_count = 100;
						order[2].complete = 0;
						order[2].score_y = 300;
						order[2].youangry = true;
						if (order[2].vari == 1)
							life_03--;
					}
					KillTimer(hWnd, 6003);
				}
			}
			else if (order[2].get_dish) { //�ֹ��� �޴��� �޾��� ��
				if (order[2].correct == true) { //�¾��� ��
					if (order[2].vari == 1) {
						if (order[2].c_06 % 2 == 0)
							order[2].face = order_face_03[3];
						else if (order[2].c_06 % 2 == 1) {
							order[2].face = order_face_03[4];
						}
					}
					else if (order[2].vari == 2) {
						if (order[2].c_06 % 2 == 0)
							order[2].face = villain_03[3];
						else if (order[2].c_06 % 2 == 1) {
							order[2].face = villain_03[4];
						}
					}
					order[2].score_y -= 10; //�󱼿� ���� ����ǥ ���
					if (order[2].c_06 == 4) { //4�� �Ʊ��۾Ʊ����ϰ� ����
						if (order[2].vari == 1) {
							order[2].c_06 = 0;
							order[2].face = order_face_03[0]; //ó�� ǥ������ ���������� ����
							order[2].face_count = 100; //�ٷ� �ϰ��ϵ���
							order[2].complete = 0; //�Ϸ� ó�� ���Ѱɷ� ����
							//order[2].get_dish = false;
							order[2].correct = false;
							order[2].score_y = 300; //���� ���� ��ǥ���� ���
							success_03++;
							if (success_03 >= 10 && stage_03 == 1) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							else if (success_03 >= 15 && stage_03 == 2) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							else if (success_03 >= 18 && stage_03 == 3) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 5003);
									KillTimer(hWnd, 6003);
								}
							}
							KillTimer(hWnd, 6003);
						}
						else if (order[2].vari == 2) {
							order[2].face = villain_03[0]; //ó�� ǥ��
							order[2].get_dish = false;
							order[2].correct = false;
							order[2].complete = 0;
							order[2].score_y = 300; //���� ���� ��ǥ���� ���
							KillTimer(hWnd, 6003);
						}
					}
					order[2].c_06++;
				}
				else { //�߸� ���� ��
					if (order[2].vari == 1) {
						if (order[2].c_06 % 2 == 0)
							order[2].face = order_face_03[7];
						else if (order[2].c_06 % 2 == 1) {
							order[2].face = order_face_03[8];
						}
					}
					else if (order[2].vari == 2) {
						if (order[2].c_06 % 2 == 0)
							order[2].face = villain_03[3];
						else if (order[2].c_06 % 2 == 1) {
							order[2].face = villain_03[4];
						}
					}
					order[2].c_06++;
					order[2].score_y -= 10; //�󱼿� ���� ����ǥ ���
					if (order[2].c_06 >= 4) {
						if (order[2].vari == 1) {
							order[2].c_06 = 0;
							order[2].face = order_face_03[2]; //ȭ�� ǥ������ ���������� ����
							order[2].face_count = 100; //�ٷ� �ϰ��ϵ���
							order[2].complete = 0; //�Ϸ� ó�� ���Ѱɷ� ����
							order[2].score_y = 300; //���� ���� ��ǥ���� ���
							//order[2].get_dish = false;
							order[2].correct = false;
							life_03--;
							KillTimer(hWnd, 6003);
						}
						else if (order[2].vari == 2) {
							order[2].face = villain_03[0]; //ó�� ǥ��
							order[2].get_dish = false;
							order[2].correct = false;
							order[2].complete = 0;
							order[2].score_y = 300; //���� ���� ��ǥ���� ���
							KillTimer(hWnd, 6003);
						}
					}
				}
			}
			break;
		}

		case 1006: {//�մ��� ��� �� �ϰ�
			for (int i = 0; i < 3; ++i) {
				if (!order[i].place) {
					order[i].y -= 40;
					if (order[i].y <= 180) {//�մ��� �����ϸ� �ֹ��� �����޴´�
						if (order[i].y < 180)
							order[i].y = 180;
						order[i].face_count = 0;
						order[i].place = true;
						if (stage_03 == 1) {
							if (order[i].vari == 1 || order[i].vari == 2) {
								if (success_03 >= 10) {
									order[i].youangry = true;
									order[i].face_count = 100;
								}
							}
							if (order[i].vari == 1) {
								order[i].or_top = rand() % 2 + 1;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//�ֹ� ��Ʈ�ʳ��� �� ����?//�����ϸ� ��Ʈ�ʸ����� üũ ����(���� ����)
									case 1://��ġ
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://����
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://����
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://�޹�
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
							else if (order[i].vari == 3) {
								order[i].or_top = 4;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//�ֹ� ��Ʈ�ʳ��� �� ����?//�����ϸ� ��Ʈ�ʸ����� üũ ����(���� ����)
									case 1://��ġ
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://����
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://����
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://�޹�
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
						}
						else if (stage_03 == 2) {
							if (order[i].vari == 1 || order[i].vari == 2) {
								if (success_03 >= 15) {
									order[i].youangry = true;
									order[i].face_count = 100;
								}
							}
							if (order[i].vari == 1) {
								order[i].or_top = rand() % 2 + 2;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//�ֹ� ��Ʈ�ʳ��� �� ����?//�����ϸ� ��Ʈ�ʸ����� üũ ����(���� ����)
									case 1://��ġ
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://����
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://����
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://�޹�
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
							else if (order[i].vari == 3) {
								order[i].or_top = 5;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//�ֹ� ��Ʈ�ʳ��� �� ����?//�����ϸ� ��Ʈ�ʸ����� üũ ����(���� ����)
									case 1://��ġ
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://����
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://����
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://�޹�
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
						}
						else if (stage_03 == 3) {
							if (order[i].vari == 1 || order[i].vari == 2) {
								if (success_03 >= 18) {
									order[i].youangry = true;
									order[i].face_count = 100;
								}
							}
							if (order[i].vari == 1) {
								order[i].or_top = rand() % 3 + 2;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//�ֹ� ��Ʈ�ʳ��� �� ����?//�����ϸ� ��Ʈ�ʸ����� üũ ����(���� ����)
									case 1://��ġ
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://����
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://����
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://�޹�
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
							else if (order[i].vari == 3) {
								order[i].or_top = 6;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//�ֹ� ��Ʈ�ʳ��� �� ����?//�����ϸ� ��Ʈ�ʸ����� üũ ����(���� ����)
									case 1://��ġ
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://����
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://����
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://�޹�
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
						}
					}
				}
				else if (order[i].place && order[i].face_count >= 23) {//�մ��� �ö���� 23�ʰ� ��������
					if (order[i].vari == 3) {
						order[i].y++;
						order[i].c_06 = 0;
						order[i].correct = false;
						channel->stop();
						channel3->stop();
						SetTimer(hWnd, 5003, 800, NULL);
						KillTimer(hWnd, 1003);
						KillTimer(hWnd, 2003);
						KillTimer(hWnd, 3003);
						KillTimer(hWnd, 4003);
						KillTimer(hWnd, 6003);

						KillTimer(hWnd, 1006);
						KillTimer(hWnd, 2006);
					}
					else {
						if (order[i].get_dish == false && success_03 < 15 && order[i].youangry == false) {
							order[i].face_count = 0;
							order[i].correct = false;
							order[i].get_dish = true;
							order[i].complete = 1;
							order[i].score = minus_heart_03;
							order[i].y++;
							if (order[i].vari == 1) {
								ssystem->playSound(incorrect, 0, false, &channel1);
								channel1->setVolume(0.5);	//ȿ���� ä��
							}
							if (i == 0) {
								SetTimer(hWnd, 4003, 200, NULL);
							}
							else if (i == 1) {
								SetTimer(hWnd, 5003, 200, NULL);
							}
							else {
								SetTimer(hWnd, 6003, 200, NULL);
							}
						}
						if (order[i].face_count >= 20) {
							order[i].y += 40;
						}
					}
					if (order[i].y >= 500) {
						order[i].place = false;
						order[i].get_dish = false;
						order[i].youangry = false;
						order[i].c_06 = 0;
						order[i].score_y = 300; //���� ���� ��ǥ���� ���
						v_check_03 = rand() % 100 + 1;
						if (success_03 >= 10 && stage_03 == 1) {//���� Ƚ�� �ֹ� ���� �� ������	������������ �ٸ��� ������
							if (i == 0) {
								order[i].vari = 4;
								order[i].face = light_03;
							}
							if (i == 1) {
								order[i].vari = 3;
								order[i].face = master_03[0];
							}
							if (i == 2) {
								order[i].vari = 5;
								order[i].face = camera_03;
							}
						}
						else if (success_03 >= 15 && stage_03 == 2) {
							if (i == 0) {
								order[i].vari = 4;
								order[i].face = light_03;
							}
							if (i == 1) {
								order[i].vari = 3;
								order[i].face = master_03[0];
							}
							if (i == 2) {
								order[i].vari = 5;
								order[i].face = camera_03;
							}
						}
						else if (success_03 >= 18 && stage_03 == 3) {
							if (i == 0) {
								order[i].vari = 4;
								order[i].face = light_03;
							}
							if (i == 1) {
								order[i].vari = 3;
								order[i].face = master_03[0];
							}
							if (i == 2) {
								order[i].vari = 5;
								order[i].face = camera_03;
							}
						}
						else if (v_check_03 <= v_percent_03) {
							order[i].vari = 2;	//���ز�
							v_percent_03 = 0;
						}
						else {
							order[i].vari = 1;	//�մ��� ���� ����
						}
						switch (order[i].vari) {
						case 1://�մ�
							order[i].face = order_face_03[0];
							break;
						case 2://���ز�
							order[i].face = villain_03[0];
							break;
						}
					}
				}
				if (falling_pot_03.falling && !falling_pot_03.crash) {//������ �ϰ�
					falling_pot_03.y += 15;
					for (int i = 0; i < 3; ++i) {
						if (falling_pot_03.x == order[i].x && falling_pot_03.y + 50 >= order[i].y && order[i].place
							&& !order[i].hit_pot) {
							if (rand() % 100 + 1 <= 33) {	//33% Ȯ���� �ҹ�� ����
								ssystem->playSound(secret_hit, 0, false, &channel1);
							}
							else {
								ssystem->playSound(hit_sound, 0, false, &channel1);
								channel1->setVolume(0.5);
							}
							falling_pot_03.y += 50;
							falling_pot_03.falling = false;
							falling_pot_03.crash = true;
							falling_pot_03.pot = hit_pot_03;
							order[i].hit_pot = true;
							order[i].c_06 = 0;
							order[i].face_count = 0;
							if (order[i].vari == 1) {
								order[i].face = order_face_03[6];	//�մ� �ƴ� �ٸ�����϶� �ٲ����ڡ�
							}
							else if (order[i].vari == 2) {
								order[i].face = villain_03[2];
							}
							if (order[i].vari == 1) {
								order[i].complete = 1;
								order[i].score = minus_heart_03;
							}
							else if (order[i].vari == 2) {
								order[i].complete = 1;
								order[i].score = order_check_06[2];
								score_03 += 50;
							}
							SetTimer(hWnd, 3003, 200, NULL);
							if (i == 0) {
								SetTimer(hWnd, 4003, 200, NULL);
							}
							else if (i == 1) {
								if (order[i].vari == 3) {
									channel->stop();
									channel3->stop();
									order[i].y++;
									order[i].c_06 = 0;
									order[i].correct = false;
									isTimer5003 = true;
									KillTimer(hWnd, 1003);
									KillTimer(hWnd, 2003);
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 6003);

									KillTimer(hWnd, 1006);
									KillTimer(hWnd, 2006);
									SetTimer(hWnd, 5003, 800, NULL);
								}
								else {
									SetTimer(hWnd, 5003, 200, NULL);
								}
							}
							else if (i == 2) {
								SetTimer(hWnd, 6003, 200, NULL);
							}
							break;
						}
						else {
							if (falling_pot_03.y >= 440) {
								falling_pot_03.falling = false;
								falling_pot_03.x = falling_pot_03.y = -1000;
							}
						}
					}
				}
			}
			break;
		}
		case 2006: {//�մ��� �г� ī��Ʈ ����
			for (int i = 0; i < 3; ++i) {
				if (order[i].place && order[i].y <= 180) {
					if (order[i].vari == 1 && !order[i].get_dish) {
						order[i].face_count++;
						if (order[i].face_count == 13)
							order[i].face = order_face_03[1];
						else if (order[i].face_count == 18)
							order[i].face = order_face_03[2];
					}
					if (order[i].vari == 3 && !order[i].get_dish) {
						order[i].face_count++;
						if (order[i].face_count == 13)
							order[i].face = master_03[1];
						else if (order[i].face_count == 18)
							order[i].face = master_03[2];
					}
				}
			}
			if (stage_03 == 1) {
				v_percent_03 += 0.5;
			}
			else if (stage_03 == 2) {
				v_percent_03 += 1;
			}
			else if (stage_03 == 3) {
				v_percent_03 += 1.5;
			}
			break;
		}
		}
		if (life_03 == 0) {
			stage_03 = 5;
			channel->stop();
			channel3->stop();
			ssystem->playSound(s_bad_end, 0, false, &channel);
			channel->setVolume(0.5);
			KillTimer(hWnd, 1003);
			KillTimer(hWnd, 2003);
			KillTimer(hWnd, 3003);
			KillTimer(hWnd, 4003);
			KillTimer(hWnd, 5003);
			KillTimer(hWnd, 6003);

			KillTimer(hWnd, 1006);
			KillTimer(hWnd, 2006);
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_CHAR:
		if (stage_03 >= 1 && stage_03<=3) {
			exist_v_03 = false;
			for (int i = 0; i < 3; ++i) {
				if (order[i].vari == 2) {
					exist_v_03 = true;
					break;
				}
			}
			if (!exist_v_03 && !isTimer5003) {
				switch (wParam) {
				case 'Q':
				case 'q':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 1;	//��ġ�� ����
					ladle_03.x = 670; ladle_03.y = 425;
					break;
				case 'W':
				case 'w':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 2;	//���� ����
					ladle_03.x = 670; ladle_03.y = 495;
					break;
				case 'E':
				case 'e':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 3;	//������ ����
					ladle_03.x = 670; ladle_03.y = 560;
					break;
				case 'R':
				case 'r':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 4;	//�޹��� ����
					ladle_03.x = 670; ladle_03.y = 625;
					break;
				case 'o':	//���� �׽�Ʈ��
					score_03 += 1000;
					break;
				case 'p':
					score_03 += 100;
					break;
				case 'l':
					score_03 += 10;
					break;
				case ';':
					score_03 += 1;
					break;
				case '+':
					life_03++;
					break;
				case '-':
					life_03--;
					break;
				case '9':
					success_03++;
					break;
				}
			}
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_LBUTTONDOWN:
		mx_03 = LOWORD(lParam);
		my_03 = HIWORD(lParam);
		count_06 = 0;
		if (stage_03 == 0) {
			if (mx_03 > 714 && mx_03 < 975 && my_03 >612 && my_03 < 702 && !on_explain) {
				on_explain = true;
				c_hWnd = CreateWindow(L"ChildClass", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 100, 824, 568, hWnd, NULL, g_hInst, NULL);
			}
			else if(!on_explain) {
				stage_03++;
				SetTimer(hWnd, 1003, 200, NULL);//���� Ÿ�̸�
				SetTimer(hWnd, 2003, 1000, NULL);	//�� ���� ī��Ʈ
				SetTimer(hWnd, 1006, 40, NULL);	//�մ� �̵�
				SetTimer(hWnd, 2006, 1000, NULL);	//�մ��� ���ð�
				for (int i = 0; i < 3; ++i) {
					order[i].youangry = false;
				}
				channel->stop();
				channel3->stop();
				ssystem->playSound(stage1, 0, false, &channel);
				channel->setVolume(0.5);
				ssystem->playSound(scene_sound, 0, false, &channel3);
				channel3->setVolume(0.35);
			}
		}
		else if (stage_03 == 4 || stage_03 == 5 || stage_03 == 6) {
			stage_03 = 0;
			score_03 = 0;
			life_03 = 3;
			P_top_03 = 0;	//���� �ʱ�ȭ
			for (int i = 0; i < 8; ++i) {//���� �ʱ�ȭ
				f_board_03[i].used = false;
			}
			for (int i = 0; i < 3; ++i) {//�մ� �ʱ� ����
				order[i].vari = 1;
				order[i].y = (i * 2000) + 500;
				order[i].x = (i * 322) + 38;
				order[i].score_x = (i * 322) + 45;
				order[i].score_y = 300;
				order[i].place = false;
				order[i].hit_pot = false;
				order[i].face = order_face_03[0];
			}
			P_select_03 = 1;	//��ġ�� ����(Ű�Է����� �ٲ�)
			ladle_03.x = 670; ladle_03.y = 425;	//�ʱ� ���� ��ġ
			grab_pot_03 = false;
			v_percent_03 = 0;
			success_03 = 0;
			isTimer5003 = false;
			{
				falling_pot_03.crash = false;
				falling_pot_03.falling = false;
				falling_pot_03.x = falling_pot_03.y = -1000;
				falling_pot_03.pot = pot_03;
			}
			channel->stop();
			channel3->stop();
			ssystem->playSound(s_title, 0, false, &channel);
			channel->setVolume(0.3);
		}
		else if (!isTimer5003) {
			if (!grab_pot_03) {//���� ���� ���� ����
				for (int i = 0; i < 3; ++i) {//�ö�� �մ� Ŭ��
					if (mx_03 >= order[i].x && mx_03 <= order[i].x + 167
						&& my_03 >= order[i].y && my_03 <= order[i].y + 260) {
						if (order[i].place == true && order[i].y == 180) {
							if (P_top_03 > 0) {
								if (order[i].vari == 1 || order[i].vari == 3) {
									if (P_top_03 == order[i].or_top) { //���� ���� ���� �մ��� ���ϴ� ���� �� ��
										for (int j = 0; j < P_top_03; j++) {
											if (order[i].stack[j].pancake == Stack_03[j].pancake) {
												count_06++; //���� ������ �� � �´��� Ȯ��
											}
										}
										if (count_06 == P_top_03) { //���� ������ ���� �� ���� �� ���� ��ġ �Ҷ�
											order[i].correct = true;
											order[i].get_dish = true;
											P_top_03 = 0; //���ÿ� ���� �� ó��
											if (order[i].vari == 1) {
												order[i].complete = 1; //�Ϸ��ߴٰ� ó��
											}
											order[i].c_06 = 0;
											if (order[i].vari == 1) {
												ssystem->playSound(correct, 0, false, &channel1);
												channel1->setVolume(0.5);	//ȿ���� ä��
												if (0 < order[i].face_count && order[i].face_count <= 13) {//0~13�� ���̿��� 100��
													order[i].score = order_check_06[0];
													score_03 += 100;
												}
												else if (13 < order[i].face_count && order[i].face_count <= 18) {//13~18�� ���̿��� 80��
													order[i].score = order_check_06[1];
													score_03 += 80;
												}
												else {//18~23�� ���̿��� 50��
													order[i].score = order_check_06[2];
													score_03 += 50;
												}
											}
											if (i == 0) {
												SetTimer(hWnd, 4003, 200, NULL);
											}
											else if (i == 1) {
												if (order[i].vari == 3) {
													channel->stop();
													channel3->stop();
													ssystem->playSound(nurvous, 0, false, &channel);
													channel->setVolume(0.5);
													order[i].y++;
													isTimer5003 = true;
													SetTimer(hWnd, 5003, 800, NULL);
													KillTimer(hWnd, 1003);
													KillTimer(hWnd, 2003);
													KillTimer(hWnd, 3003);
													KillTimer(hWnd, 4003);
													KillTimer(hWnd, 6003);

													KillTimer(hWnd, 1006);
													KillTimer(hWnd, 2006);
												}
												else {
													SetTimer(hWnd, 5003, 200, NULL);
												}
											}
											else {
												SetTimer(hWnd, 6003, 200, NULL);
											}
										}
										else {
											order[i].correct = false;
											order[i].get_dish = true;
											P_top_03 = 0; //���ÿ� ���� �� ó��
											if (order[i].vari == 1) {
												ssystem->playSound(incorrect, 0, false, &channel1);
												channel1->setVolume(0.5);	//ȿ���� ä��
												order[i].complete = 1; //�Ϸ��ߴٰ� ó��
												order[i].score = minus_heart_03;
											}
											order[i].c_06 = 0;
											if (i == 0) {
												SetTimer(hWnd, 4003, 200, NULL);
											}
											else if (i == 1) {
												if (order[i].vari == 3) {
													channel->stop();
													channel3->stop();
													ssystem->playSound(nurvous, 0, false, &channel);
													channel->setVolume(0.5);
													order[i].y++;
													isTimer5003 = true;
													SetTimer(hWnd, 5003, 800, NULL);
													KillTimer(hWnd, 1003);
													KillTimer(hWnd, 2003);
													KillTimer(hWnd, 3003);
													KillTimer(hWnd, 4003);
													KillTimer(hWnd, 6003);

													KillTimer(hWnd, 1006);
													KillTimer(hWnd, 2006);
												}
												else
													SetTimer(hWnd, 5003, 200, NULL);
											}
											else {
												SetTimer(hWnd, 6003, 200, NULL);
											}
										}
									}
									else {
										order[i].correct = false;
										order[i].get_dish = true;
										P_top_03 = 0; //���ÿ� ���� �� ó��
										if (order[i].vari == 1) {
											ssystem->playSound(incorrect, 0, false, &channel1);
											channel1->setVolume(0.5);	//ȿ���� ä��
											order[i].complete = 1; //�Ϸ��ߴٰ� ó��
										}
										order[i].score = minus_heart_03;
										order[i].c_06 = 0;
										if (i == 0) {
											SetTimer(hWnd, 4003, 200, NULL);
										}
										else if (i == 1) {
											if (order[i].vari == 3) {
												channel->stop();
												channel3->stop();
												ssystem->playSound(nurvous, 0, false, &channel);
												channel->setVolume(0.5);
												order[i].y++;
												isTimer5003 = true;
												SetTimer(hWnd, 5003, 800, NULL);
												KillTimer(hWnd, 1003);
												KillTimer(hWnd, 2003);
												KillTimer(hWnd, 3003);
												KillTimer(hWnd, 4003);
												KillTimer(hWnd, 6003);

												KillTimer(hWnd, 1006);
												KillTimer(hWnd, 2006);
											}
											else
												SetTimer(hWnd, 5003, 200, NULL);
										}
										else {
											SetTimer(hWnd, 6003, 200, NULL);
										}
									}
								}
								else if (order[i].vari == 2) {
									order[i].correct = false;
									order[i].get_dish = true;
									order[i].complete = 0; //�Ϸ��ߴٰ� ó��
									P_top_03 = 0; //���ÿ� ���� �� ó��
									order[i].c_06 = 0;
									if (i == 0) {
										SetTimer(hWnd, 4003, 200, NULL);
									}
									else if (i == 1) {
										SetTimer(hWnd, 5003, 200, NULL);
									}
									else {
										SetTimer(hWnd, 6003, 200, NULL);
									}
								}
							}
						}
					}
				}
				for (int i = 0; i < 8; ++i) {
					if (mx_03 > f_board_03[i].range.left && mx_03 < f_board_03[i].range.right
						&& my_03 > f_board_03[i].range.top && my_03 < f_board_03[i].range.bottom) {
						if (f_board_03[i].used) {
							if (P_top_03 < 10) {
								ssystem->playSound(grab_pan, 0, false, &channel1);
								channel1->setVolume(0.5);
								Stack_03[P_top_03] = f_board_03[i];
								Stack_03[P_top_03].x = 760;
								Stack_03[P_top_03].y = 594 - (P_top_03 * 15);
								Stack_03[P_top_03].width = 220;
								Stack_03[P_top_03].height = 110;
								P_top_03++;
								f_board_03[i].used = false;
							}
						}
						else {
							ssystem->playSound(fried, 0, false, &channel1);
							channel1->setVolume(0.5);
							f_board_03[i].used = true;
							f_board_03[i].count = 0;
							f_board_03[i].vari = P_select_03;
							switch (f_board_03[i].vari) {
							case 1://��ġ
								f_board_03[i].pancake = pancake_03[0];
								break;
							case 2://����
								f_board_03[i].pancake = pancake_03[3];
								break;
							case 3://������
								f_board_03[i].pancake = pancake_03[6];
								break;
							case 4://�޹���
								f_board_03[i].pancake = pancake_03[9];
								break;
							}
						}
					}
				}
			}
			else if (grab_pot_03) {//���� ��������� Ŭ��
				grab_pot_03 = false;
				falling_pot_03.falling = true;
			}
			if (!grab_pot_03 && mx_03 > 740 && mx_03 < rt_03.right && my_03 > 440 && my_03 < 585 && !falling_pot_03.falling) {//���� Ŭ��
				grab_pot_03 = true;
				falling_pot_03.x = order[2].x;
				falling_pot_03.y = 50;
				falling_pot_03.pot = pot_03;
			}
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_MOUSEMOVE:
		if (grab_pot_03) {
			mx_03 = LOWORD(lParam);
			my_03 = HIWORD(lParam);
			for (int i = 0; i < 3; ++i) {
				if (mx_03 > order[i].x && mx_03 < order[i].x + 167) {
					falling_pot_03.x = order[i].x;
					falling_pot_03.y = 50;
					break;
				}
			}
		}
		break;
	case WM_RBUTTONDOWN:
		if (grab_pot_03) {
			falling_pot_03.x = falling_pot_03.y = -1000;
			grab_pot_03 = false;
		}
		else {
			P_top_03 = 0;
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_DESTROY:
		ssystem->close();
		ssystem->release();
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hWnd, Message, wParam, lParam));
}

LRESULT CALLBACK ChildProc(HWND c_hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	RECT rt;
	HDC hDC, mDC, mDC2;
	HBITMAP hBit, oldBit[2];
	PAINTSTRUCT ps;
	switch (Message) {
	case WM_CREATE:
		break;
	case WM_PAINT:
		GetClientRect(c_hWnd, &rt);
		hDC = BeginPaint(c_hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		mDC2 = CreateCompatibleDC(mDC);
		hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		oldBit[0] = (HBITMAP)SelectObject(mDC, hBit);
		oldBit[1] = (HBITMAP)SelectObject(mDC2, explain);
		TransparentBlt(mDC, 0, 0, rt.right, rt.bottom, mDC2, 0, 0, 1024, 768, RGB(34, 177, 76));
		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
		DeleteDC(mDC2);
		DeleteDC(mDC);
		EndPaint(c_hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		DestroyWindow(c_hWnd);
		on_explain = false;
		SetFocus(hWnd);
		break;
	case WM_DESTROY:
		DestroyWindow(c_hWnd);
		break;
	}
	return(DefWindowProc(c_hWnd, Message, wParam, lParam));
}