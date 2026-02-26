#include <string>
#include <graphics.h>
//动画帧索引，用来改变动画画面
int idx_current_anim = 0;
//储存动画图像
const int PLAYER_ANIM_NUM = 5;
IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
//加载动画函数
void load_animation() {
	for (int i = 0; i < PLAYER_ANIM_NUM; i++) {
		std::wstring path = L"img\\playerz_left" + std::to_wstring(i) + L".png";
		loadimage(&img_player_left[i], path.c_str());
	}
	for (int i = 0; i < PLAYER_ANIM_NUM; i++) {
		std::wstring path = L"img\\playerz_right" + std::to_wstring(i) + L".png";
		loadimage(&img_player_right[i], path.c_str());
	}
}

//可以使用透明底的putimage
inline void putimage_alpha(int x, int y, IMAGE* img) {
	int w = img->getwidth(); 
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA});
}

int main() {
	//初始化绘图窗口
	initgraph(1280, 720);
	//加载背景图片
	IMAGE img_background;
	loadimage(&img_background,_T("img\\background.png"));
	//加载动画
	load_animation();

	bool running = true;
	BeginBatchDraw();  //开启批量绘图
	while (running) {
		//游戏主循环
		DWORD begintime = GetTickCount();  //控制每秒帧数
		
		//计数器来计算经过了几帧，确定动画画面改变时机
		static int counter = 0;
		if (++counter >= 5) {
			counter = 0;
			idx_current_anim++;
		}
		//使动画索引循环
		idx_current_anim %= PLAYER_ANIM_NUM;

		cleardevice();
		//绘图
		putimage(0, 0, &img_background); //绘制背景
		putimage_alpha(500, 500, &img_player_left[idx_current_anim]);
		FlushBatchDraw();




		DWORD endtime = GetTickCount();
		DWORD deltatime = begintime - endtime;
		if (deltatime <= 1000 / 60) {
			Sleep(1000 / 60 - deltatime);
		}
	}
	EndBatchDraw(); //结束批量绘图

}