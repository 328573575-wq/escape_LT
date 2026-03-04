#include <string>
#include <vector>
#include <graphics.h>
#include <time.h>
const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "MSIMG32.LIB")
bool is_game_start = false;
enum class menu_choice {
	START = 0,
	END,
	RESTART,
	HOLD
};
menu_choice m_choice = menu_choice::HOLD;
bool running = true;
const double PI = 3.14159;
//子弹个数设置
int bullet_set_num = 1;
//敌人生成周期
double init_enemy_interval = 1200;
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
	const POINT& get_pos() const {
		return player_pos;
	}
	const int get_w() const{
		return PLAYER_W;
	}
	const int get_h() const{
		return PLAYER_H;
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
public:
     bullet() {
		 loadimage(&bullet_img, _T("img\\bullet.png"));
	}
	 const POINT& get_bul_pos() const {
		return bullet_pos;
	}
	 POINT& modify_pos() {
		 return bullet_pos;
	 }
	void DRAW() {
		putimage_alpha(bullet_pos.x, bullet_pos.y, &bullet_img);
	}
private:
	POINT bullet_pos = { 0 , 0 };
	IMAGE bullet_img;
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
		bool overlap_x = bul.get_bul_pos().x >= enemy_pos.x && bul.get_bul_pos().x <= enemy_pos.x + ENEMY_W;
		bool overlap_y = bul.get_bul_pos().y>= enemy_pos.y && bul.get_bul_pos().y <= enemy_pos.y + ENEMY_H;
		return overlap_x && overlap_y;
	}
	//与玩家碰撞函数
	bool touch_player(const player& man) {
		double t_x = enemy_pos.x + ENEMY_W / 2 ;
		double t_y = enemy_pos.y + ENEMY_H / 2 ;
		bool overlap_x = t_x >= man.get_x() && t_x <= man.get_x() + man.get_w();
		bool overlap_y = t_y >= man.get_y() && t_y <= man.get_y() + man.get_h();
		return overlap_x && overlap_y;
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
	void hurt() {
		alive = false;
	}
	bool is_alive() {
		return alive;
	}
	double& get_speed() {
		return ENEMYSPEED;
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
	double ENEMYSPEED = 2;
	//敌人的阴影宽度
	const int SHADOW_W = 80;
	//敌人图像的大小
	const int ENEMY_W = 100;
	const int ENEMY_H = 100;
	//敌人血量以及存活状态
	int blood = 0;
	bool alive = true;
private:
	IMAGE shadow_enemy;
	Animation* img_enemy;

};
//按键类
class button {
public:
	button(RECT rect,LPCTSTR path_idle ,LPCTSTR path_hovered,LPCTSTR path_pushed) 
		:region(rect)
	{
		loadimage(&img_idel , path_idle);
		loadimage(&img_hovered , path_hovered);
		loadimage(&img_pushed , path_pushed);
	}
	//修改按键状态为idle
	void modify_status_idle() {
		status = Status::Idle;
	}
	//处理鼠标信息
	void processEvent(const ExMessage& msg) {
		if (msg.message == WM_MOUSEMOVE) {
			if (status == Status::Idle && cursor(msg.x, msg.y)) {
				status = Status::Hovered;
			}
			else if (status == Status::Hovered && !cursor(msg.x, msg.y)) {
				status = Status::Idle;
			}
		}
		else if (msg.message == WM_LBUTTONDOWN) {
			if (cursor(msg.x, msg.y)) {
				status = Status::Pushed;
			}
		}
		else if (msg.message == WM_LBUTTONUP) {
			if (status == Status::Pushed) {
				OnClick();
			}
		}
	}
	//绘制按键
	void draw() {
		switch (status) {
		case Status::Idle:
			putimage(region.left, region.top, &img_idel);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		default:
			break;
		}

	}
	~button() {

	}
protected:
	virtual void OnClick() = 0;
private:
	enum class Status {
	Idle = 0,
	Hovered ,
	Pushed
	};
	//检测鼠标位置是否在按键上
	bool cursor(int x, int y) {
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
private:
	RECT region;
	IMAGE img_idel;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;
};

//开始按键
class start_button :public button {
public:
	start_button(RECT rect, LPCTSTR path_idle, LPCTSTR path_hovered, LPCTSTR path_pushed)
		: button(rect, path_idle, path_hovered, path_pushed) {}
protected:
	void OnClick() {
		m_choice = menu_choice::START;
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
	}
};
//结束按键
class end_button: public button {
public:
	end_button(RECT rect, LPCTSTR path_idle, LPCTSTR path_hovered, LPCTSTR path_pushed)
		: button(rect, path_idle, path_hovered, path_pushed) {}
protected:
	void OnClick() {
		m_choice = menu_choice::END;
	}
};
//重启按键
class restart_button :public button {
public:
	restart_button(RECT rect, LPCTSTR path_idle, LPCTSTR path_hovered, LPCTSTR path_pushed)
		: button(rect, path_idle, path_hovered, path_pushed) {
	}
protected:
	void OnClick() {
		m_choice = menu_choice::RESTART;
	}
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
	if (delta >= init_enemy_interval) {
		enemy_list.push_back(new enemy(1));
		return current_t;
	}
	return last_t;
}
//子弹运动函数
void bullet_move(const player& ply, std::vector<bullet>& list) {
	const double RADIAL_SPEED = 0.0045;	 //径向速度
	const double TANGENT_SPEED = 0.0045; //切向速度
	double interval = 2 * PI / list.size(); //获取一个子弹基础间隔
	double radius = 100 + 35 * sin(GetTickCount() * RADIAL_SPEED);//可变的半径
	const POINT plyaer_pos = ply.get_pos();
	for (int i = 0; i < list.size(); i++) {
		double radian = i * interval + GetTickCount()*TANGENT_SPEED; //获得子弹当前可变的弧度
		list[i].modify_pos().x = ply.get_x() + ply.get_w() / 2 + radius * cos(radian);
		list[i].modify_pos().y = ply.get_y() + ply.get_h() / 2 + radius * sin(radian);
	}
	
}
//分数等级函数
void laugh(int l_score) {
	if (l_score <= 50) {
		outtextxy(10, 30, _T("飞舞喵，还不到五十分"));
	}
	else if (l_score <= 100) {
		outtextxy(10, 30, _T("杂口，杂口"));
	}
	else if (l_score <= 150) {
		outtextxy(10, 30, _T("还算有点水平喵"));
	}
	else if (l_score <= 200) {
		outtextxy(10, 30, _T("?!强强！？，柚子勇者邹加昊重现荣光喵！"));
	}
	else {
		outtextxy(10, 30, _T("老资历，我给你跪了喵😭"));
	}
}
//菜单函数
void main_menu(ExMessage& msg , start_button& btn_start,end_button& btn_end,IMAGE& img_menu) {
	mciSendString(_T("play menu repeat from 0"), NULL, 0, NULL);
	BeginBatchDraw();
	while (true) {
		DWORD begintime = GetTickCount();

		if (peekmessage(&msg)) {
			btn_start.processEvent(msg);
			btn_end.processEvent(msg);
		}
		cleardevice();
		//绘图
		putimage(0, 0, &img_menu);//绘制主菜单
		btn_start.draw();
		btn_end.draw();
		FlushBatchDraw();

		if (m_choice == menu_choice::START) {
			is_game_start = true;
			mciSendString(_T("stop menu"), NULL, 0, NULL);
			return;
		}
		else if (m_choice == menu_choice::END) {
			is_game_start = false;
			return;
		}
		DWORD endtime = GetTickCount();
		DWORD deltatime = endtime - begintime;
		if (deltatime <= 1000 / 120) {
			Sleep(1000 / 120 - deltatime);
		}

	}
	EndBatchDraw();
	return;
}
//结束菜单
void end_menu(ExMessage& msg, restart_button& btn_restart, end_button& btn_end,IMAGE& img_menu) {
	mciSendString(_T("stop bgm "), NULL, 0, NULL);
	mciSendString(_T("play menu repeat from 0"), NULL, 0, NULL);
	BeginBatchDraw();
	while (true) {
		DWORD begintime = GetTickCount();

		if (peekmessage(&msg)) {
			btn_restart.processEvent(msg);
			btn_end.processEvent(msg);
		}
		cleardevice();
		//绘图
		putimage(0, 0, &img_menu);//绘制菜单
		btn_restart.draw();
		btn_end.draw();
		FlushBatchDraw();

		if (m_choice == menu_choice::RESTART) {
			is_game_start = true;
			mciSendString(_T("stop menu"), NULL, 0, NULL);
			return;
		}
		else if (m_choice == menu_choice::END) {
			is_game_start = false;
			return;
		}
		DWORD endtime = GetTickCount();
		DWORD deltatime = endtime - begintime;
		if (deltatime <= 1000 / 120) {
			Sleep(1000 / 120 - deltatime);
		}

	}
	EndBatchDraw();
	return;
}

//主函数
int main() {
	//初始化绘图窗口
	initgraph(1280, 720);
	const int WINDOW_WIDTH = getwidth();
	const int WINDOW_HEIGHT = getheight();
	//菜单界面按键设置
	RECT region_start, region_end ,region_restart;

	region_start.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_start.right = region_start.left + BUTTON_WIDTH;
	region_start.top = 430;
	region_start.bottom = region_start.top + BUTTON_HEIGHT;

	region_end.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_end.right = region_end.left + BUTTON_WIDTH;
	region_end.top = 550;
	region_end.bottom = region_end.top + BUTTON_HEIGHT;

	region_restart.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_restart.right = region_restart.left + BUTTON_WIDTH;
	region_restart.top = 430;
	region_restart.bottom = region_restart.top + BUTTON_HEIGHT;
	start_button btn_start(region_start, 
		_T("img\\ui_start_idle.png"), _T("img\\ui_start_hovered.png"), _T("img\\ui_start_pushed.png"));
	end_button btn_end(region_end
		, _T("img\\ui_quit_idle.png"), _T("img\\ui_quit_hovered.png"), _T("img\\ui_quit_pushed.png"));
	restart_button btn_restart(region_restart,
		_T("img\\ui_restart_idle.png"), _T("img\\ui_restart_hovered.png"), _T("img\\ui_restart_pushed.png"));
	//加载主菜单界面	
	IMAGE img_menu;
	loadimage(&img_menu, _T("img\\menu.png"));
	//加载结束界面
	IMAGE img_end_menu;
	loadimage(&img_end_menu, _T("img\\end_menu.png"));
	//加载失败界面
	IMAGE img_lost_menu;
	loadimage(&img_lost_menu, _T("img\\lost_menu.png"));
	//加载音乐
	mciSendString(_T("open mymus\\bgm_battle.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open mymus\\hit.mp3 alias hit"), NULL, 0, NULL);
	mciSendString(_T("open mus\\bgm.mp3 alias menu"), NULL, 0, NULL);
	srand(time(NULL));
	FLAG:
	bullet_set_num = 1;
	//加载背景图片
	IMAGE img_background;
	loadimage(&img_background,_T("img\\background.png"));
	//创建玩家1对象
	player player_1(1);
	//创建子弹列表
	std::vector<bullet> bullet_list(bullet_set_num);
	//玩家1移动的消息
	ExMessage move_msg_1;
	//菜单消息
	ExMessage menu_msg;
	//敌人列表
	std::vector<enemy*> enemy_list;
	//敌人生成时间
	DWORD enemy_t = GetTickCount();
	//玩家分数
	int score = 0;
	//主菜单逻辑
	main_menu(menu_msg,btn_start,btn_end,img_menu);
	//开始游戏
	if (is_game_start) {
		BeginBatchDraw();  //开启批量绘图
		while (running) {
			//游戏主循环
			DWORD begintime = GetTickCount();  //控制每秒帧数

			//player移动消息处理
			player_1.process_msg(move_msg_1);
			//player移动
			player_1.process_move();
			//子弹运动
			bullet_move(player_1, bullet_list);
			//生成敌人
			enemy_t = init_enemy(enemy_list, enemy_t);
			//让每一个敌人靠近玩家
			for (enemy* p : enemy_list) {
				p->move(player_1);
			}
			//检测敌人和玩家碰撞
			for (enemy* p : enemy_list) {
				if (p->touch_player(player_1)) {
					end_menu(menu_msg,btn_restart,btn_end,img_lost_menu);
					if (is_game_start) {
						btn_start.modify_status_idle();
						btn_restart.modify_status_idle();
						goto FLAG;
					}
					else {
						goto END;
					}
				}
			}
			//检测子弹和敌人碰撞
			for (enemy* e : enemy_list) {
				for (const bullet& b : bullet_list) {
					if (e->touch_bullet(b)) {
						mciSendString(_T("play hit from 400"), NULL, 0, NULL);
						e->hurt();
					}
				}
			}
			//处理死亡敌人
			for (int i = 0; i < enemy_list.size(); i++) {
				enemy* e = enemy_list[i];
				if (!e->is_alive()) {
					score++;
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete e;
					//每击杀一个敌人增加0.04移速,上限10
					if (enemy_list[0]->get_speed() <= 10) {
						for (enemy* e : enemy_list) {
							e->get_speed() += 0.04;
						}
					}
					if (init_enemy_interval >= 200) {
						init_enemy_interval -= 5;
					}
				}
			}

			cleardevice();
			//绘图
			putimage(0, 0, &img_background); //绘制背景
			player_1.player_move_anim();
			for (enemy* p : enemy_list) {
				p->draw();
			}
			for (bullet& b : bullet_list) {
				b.DRAW();
			}
			//在左上角显示分数
			TCHAR show_score[64];
			_stprintf_s(show_score, 64, _T("睡眠修女张哲铭提醒您分数喵: %d"), score);
			outtextxy(10, 10, show_score);
			outtextxy(10, 50, _T("每击杀敌人会增强敌人的速度和生成速度，但每击杀50个敌人你会升级喵"));
			//分数等级羞辱模块
			laugh(score);
			FlushBatchDraw();
			//子弹数升级
			if (bullet_set_num <= 5) {
				bullet_set_num = 1 + score / 50;
				bullet_list.resize(bullet_set_num);
			}
			//分数判定
			 if (score >= 250) {
				end_menu(menu_msg,btn_restart,btn_end,img_end_menu);
				if (is_game_start) {
					btn_start.modify_status_idle();
					btn_restart.modify_status_idle();
					goto FLAG;
				}
				else {
					break;
				}
			}
			DWORD endtime = GetTickCount();
			DWORD deltatime = endtime - begintime;
			if (deltatime <= 1000 / 60) {
				Sleep(1000 / 60 - deltatime);
			}
		}
		EndBatchDraw(); //结束批量绘图
	}
END:
	return 0;
}