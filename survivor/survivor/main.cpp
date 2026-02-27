#include <string>
#include <vector>
#include <graphics.h>
#include <time.h>
inline void putimage_alpha(int x, int y, IMAGE* img);

//动画类 加载画面，打印画面
class Animation {
public:
	Animation(int num, LPCTSTR path, DWORD interval,int code)
		:interval_ms(interval), current_time(0), last_time(GetTickCount()), anim_idx(0)
	{
		//临时储存图片文件路径
		TCHAR path_file[256];
		for (int i = 0; i < num; i++) {
			_stprintf_s(path_file,256, path, code, i);
			IMAGE* anim = new IMAGE();
			loadimage(anim, path_file);
			anim_list.push_back(anim);
		}
	}
	void play(int x, int y) {
		current_time = GetTickCount();
		int delta = current_time - last_time;
		if (delta >= interval_ms) {
			anim_idx = (anim_idx + 1) % anim_list.size();
			last_time = current_time;
		}
		putimage_alpha(x, y, anim_list[anim_idx]);
	}
	~Animation() {
		for (int i = 0; i < anim_list.size(); i++) {
			delete anim_list[i];
		}
	}


private:
	int anim_idx;
	DWORD current_time;
	DWORD last_time;
	DWORD interval_ms;
	std::vector<IMAGE*> anim_list;
};

//玩家类
class player {
public:
	player(int n)
		: code(n)
	{
		//加载阴影
		loadimage(&shadow_player, _T("img\\shadow_player.png"));
		//加载玩家动画
		img_player_left = new Animation(5, _T("img\\player%d_left%d.png"), 83,code);
		img_player_right = new Animation(5, _T("img\\player%d_right%d.png"), 83,code);
		img_player_stand = new Animation(2, _T("img\\player%d_stand%d.png"), 83,code);
	}
	~player() {
		delete img_player_left;
		delete img_player_right;
		delete img_player_stand;
	}
	//获取玩家坐标的接口
	int get_x() const {
		return player_pos.x;
	}
	int get_y() const {
		return player_pos.y;
	}
	//打印玩家动画函数
	void player_move_anim() {
		int pos_shadow_x = player_pos.x + (PLAYER_W / 2 - SHADOW_W / 2) - 10;
		int pos_shadow_y = player_pos.y + PLAYER_H - 25;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &shadow_player);

		if (is_left) {
			img_player_left->play(player_pos.x, player_pos.y);
		}
		else if (is_right) {
			img_player_right->play(player_pos.x, player_pos.y);
		}
		else {
			img_player_stand->play(player_pos.x, player_pos.y);
		}
	}
	//玩家移动消息处理函数
	void process_msg(ExMessage& move_msg) {
		//player移动消息处理
		while (peekmessage(&move_msg)) {
			if (move_msg.message == WM_KEYDOWN) {
				switch (move_msg.vkcode) {
				case VK_UP:
					is_up = true;
					break;
				case VK_DOWN:
					is_down = true;
					break;
				case VK_LEFT:
					is_left = true;
					break;
				case VK_RIGHT:
					is_right = true;
					break;
				}
			}
			else if (move_msg.message == WM_KEYUP) {
				switch (move_msg.vkcode) {
				case VK_UP:
					is_up = false;
					break;
				case VK_DOWN:
					is_down = false;
					break;
				case VK_LEFT:
					is_left = false;
					break;
				case VK_RIGHT:
					is_right = false;
					break;
				}
			}
		}
	}
	//玩家移动处理函数
	void process_move() {
		//利用向量，归一化处理，使得任意方向前进的长度一致
		double dx = 0.0, dy = 0.0;
		if (is_up == true) dy = -1.0;
		if (is_down == true) dy = 1.0;
		if (is_left == true) dx = -1.0;
		if (is_right == true) dx = 1.0;
		if (dx != 0 || dy != 0) {
			double len = sqrt(dx * dx + dy * dy);
			player_pos.x += (dx / len) * PLAYERSPEED;
			player_pos.y += (dy / len) * PLAYERSPEED;
		}
		if (player_pos.x < 0)player_pos.x = 0;
		if (player_pos.x + PLAYER_W > WINDOW_W) player_pos.x = WINDOW_W - PLAYER_W;
		if (player_pos.y < 0)player_pos.y = 0;
		if (player_pos.y + PLAYER_H > WINDOW_H) player_pos.y = WINDOW_H - PLAYER_H;
	}
private:
	//玩家编号
	int code;
	//窗口大小
	int WINDOW_W = getwidth();
	int WINDOW_H = getheight();
	//玩家位置结构体
	POINT player_pos = { 500 , 500 };
	//玩家速度
	const int PLAYERSPEED = 5;
	//玩家的阴影宽度
	const int SHADOW_W = 80;
	//玩家图像的大小
	const int PLAYER_W = 100;
	const int PLAYER_H = 100;
private:
	//玩家阴影
	IMAGE shadow_player;
	//玩家动画
	Animation* img_player_left;
	Animation* img_player_right;
	Animation* img_player_stand;
	//玩家移动的按键是否按下的变量，用来优化player移动
	bool is_up = false;
	bool is_down = false;
	bool is_left = false;
	bool is_right = false;
};
//子弹类
class bullet {

};
//敌人类
class enemy {
public:
	enemy(int n)
		:code(n)
	{
		//加载敌人阴影
		loadimage(&shadow_enemy, _T("img\\shadow_enemy.png"));
		//加载敌人动画
		img_enemy = new Animation(5, _T("img\\enemy%d_%d.png"), 83, 1);
		//随机生成敌人
		init_enemy();
	}

	//随机生成敌人函数
	void init_enemy() {
		enum class Edge {
			Up,Down,Left,Right
		};
		Edge initedge = static_cast<Edge>(rand() % 4);
		switch (initedge) {
		case Edge::Up: 
			enemy_pos.x = rand() % WINDOW_W;
			enemy_pos.y = -ENEMY_H;
			break;
		case Edge::Down:
			enemy_pos.x = rand() % WINDOW_W;
			enemy_pos.y = WINDOW_H;
			break;
		case Edge::Left:
			enemy_pos.y = rand() % WINDOW_H;
			enemy_pos.x = -ENEMY_W;
			break;
		case Edge::Right:
			enemy_pos.y = rand() % WINDOW_H;
			enemy_pos.x = WINDOW_W;
			break;
		}
	}
	//与子弹碰撞函数
	bool touch_bullet(const bullet& bul) {
		return false;
	}
	//与玩家碰撞函数
	bool touch_player(const player& man) {
		return false;
	}
	//敌人自动索敌
	void move(const player& man) {
		double del_x = man.get_x() - enemy_pos.x;
		double del_y = man.get_y() - enemy_pos.y;
		double len = sqrt(del_x * del_x + del_y * del_y);
		if (len != 0) {
			enemy_pos.x += static_cast<int>((del_x / len) * ENEMYSPEED);
			enemy_pos.y += static_cast<int>((del_y / len) * ENEMYSPEED);
		}

	}
	//打印敌人动画函数
	void draw() {
		int pos_shadow_x = enemy_pos.x + (ENEMY_W / 2 - SHADOW_W / 2) - 10;
		int pos_shadow_y = enemy_pos.y + ENEMY_H - 25;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &shadow_enemy);

		img_enemy->play(enemy_pos.x,enemy_pos.y);
	}
	~enemy() {
		delete img_enemy;
	}
private:
	//敌人位置结构体
	POINT enemy_pos{ 0, 0};
	//敌人编号
	int code;
	//窗口大小
	int WINDOW_W = getwidth();
	int WINDOW_H = getheight();
	//敌人速度
	const int ENEMYSPEED = 2;
	//敌人的阴影宽度
	const int SHADOW_W = 80;
	//敌人图像的大小
	const int ENEMY_W = 100;
	const int ENEMY_H = 100;
private:
	IMAGE shadow_enemy;
	Animation* img_enemy;

};

//使用alpha通道的putimage,用于图形有透明度信息
inline void putimage_alpha(int x, int y, IMAGE* img) {
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA });
}
//生成敌人函数
DWORD init_enemy(std:: vector<enemy*>& enemy_list,DWORD last_t) {
	DWORD current_t = GetTickCount();
	DWORD delta = current_t - last_t;
	if (delta >= 1500) {
		enemy_list.push_back(new enemy(1));
		return current_t;
	}
	return last_t;
}


//主函数
int main() {
	srand(time(NULL));
	//初始化绘图窗口
	initgraph(1280, 720);
	//加载背景图片
	IMAGE img_background;
	loadimage(&img_background,_T("img\\background.png"));
	//创建玩家1对象
	player player_1(1);
	//玩家1移动的消息
	ExMessage move_msg_1;
	//敌人列表
	std::vector<enemy*> enemy_list;
	//敌人生成时间
	DWORD enemy_t = GetTickCount();

	bool running = true;
	BeginBatchDraw();  //开启批量绘图
	while (running) {
		//游戏主循环
		DWORD begintime = GetTickCount();  //控制每秒帧数
		
		//player移动消息处理
		player_1.process_msg(move_msg_1);
		//player移动
		player_1.process_move();
		//生成敌人
		enemy_t = init_enemy(enemy_list,enemy_t);
		//让每一个敌人靠近玩家
		for (enemy* p : enemy_list) {
			p->move(player_1);
		}
		cleardevice();
		//绘图
		putimage(0, 0, &img_background); //绘制背景
		player_1.player_move_anim();
		for (enemy* p : enemy_list) {
			p->draw();
		}
		
		// 调试：在左上角显示敌人数量，确认是否创建成功
		TCHAR dbg[64];
		_stprintf_s(dbg, 64, _T("Enemies: %d"), (int)enemy_list.size());
		outtextxy(10, 10, dbg);

		FlushBatchDraw();




		DWORD endtime = GetTickCount();
		DWORD deltatime = endtime - begintime;
		if (deltatime <= 1000 / 60) {
			Sleep(1000 / 60 - deltatime);
		}
	}
	EndBatchDraw(); //结束批量绘图
	return 0;
}