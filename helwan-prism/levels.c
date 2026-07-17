#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#define INITIAL_TIME 60
#define MAX_OBJECTS 20
#define MAX_BEAMS 100

typedef enum { OBJ_SOURCE, OBJ_MIRROR, OBJ_TARGET, OBJ_WALL, OBJ_LENS } ObjType;
typedef struct { double r, g, b; } Color;
typedef struct { double x1, y1, x2, y2; Color color; } Beam;

typedef struct {
    ObjType type;
    double x, y, angle, length;
    bool hit;
    bool draggable; 
} GameObject;

extern struct {
    GameObject objects[MAX_OBJECTS];
    int obj_count;
    Beam beams[MAX_BEAMS];
    int beam_count;
    int dragged_obj_idx;
    bool is_rotating;
    int current_level;
    bool level_cleared;
    bool game_over;
    int time_left;
    bool demo_mode;
} game;

void load_level(int level) {
    game.obj_count = 0; 
    game.beam_count = 0; 
    game.level_cleared = false; 
    game.game_over = false; 
    game.time_left = INITIAL_TIME; 
    game.current_level = level; 
    game.demo_mode = false;
    
    // وضع مصدر الشعاع الافتراضي
    game.objects[game.obj_count++] = (GameObject){OBJ_SOURCE, 50, 350, 0, 0, false, false};

    if (level == 1) {
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 400, 350, M_PI/4, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 400, 100, 0, 0, false, false};
    } else if (level == 2) {
        game.objects[0].angle = M_PI / 7.8; 
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 400, 350, 0, 200, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 200, 500, 0, 80, false, true}; 
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 600, 500, 0, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 850, 350, 0, 0, false, false};
    } else if (level == 3) {
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300, 150, 0, 100, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 600, 550, 0, 100, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 300, 250, M_PI/2, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 650, -M_PI/6, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 700, 100, M_PI/4, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 600, 0, 0, false, false};
    } else if (level == 4) {
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 375, M_PI/2, 400, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 200, 100, M_PI/4, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 800, 100, -M_PI/4, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 800, 600, 0, 0, false, false};
    } else if (level == 5) {
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300, 200, 0, 300, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 700, 500, 0, 300, false, false};
        for(int i=0; i<3; i++) 
            game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 150+i*200, 400, 0, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 100, 0, 0, false, false};
    } else if (level == 6) {
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 0, M_PI/2, 600, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 100, 600, -M_PI/4, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 900, 600, M_PI/4, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 950, 50, 0, 0, false, false};
    } else if (level == 7) {
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 400, 350, M_PI/4, 150, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 600, 350, -M_PI/4, 150, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 100, 0, 100, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 500, 700, 0, 0, false, false};
    } else if (level == 8) {
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 300, 100, M_PI/6, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 700, 100, -M_PI/6, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 500, M_PI/2, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 50, 50, 0, 0, false, false};
    } else if (level == 9) {
        // حائط يقطع الطريق مباشرة أمام المصدر
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 200, 350, M_PI/2, 600, false, false};
        
        // المصدر
        game.objects[0].x = 100;
        game.objects[0].y = 350;
        game.objects[0].angle = 0;

        // الهدف في مكان آخر
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 350, 0, 0, false, false};

        // مرايا
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 300, 150, M_PI/4, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 700, 550, -M_PI/4, 60, false, true};
    } else if (level == 10) {
        game.time_left = 30; // وقت أقل للتحدي
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 375, 0, 800, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 375, M_PI/2, 600, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 100, 100, 0, 50, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 900, 100, 0, 50, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 100, 650, 0, 50, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 650, 0, 0, false, false};
    }
      else if (level == 11) {
        // مرحلة "نفق الاختبار"
        game.time_left = 45;
		game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 350, M_PI/2, 400, false, false};

		// الحائط الثاني متأخر عنه (عند x=600)
		game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 600, 350, M_PI/2, 400, false, false};        
        // مرايا للمناورة حول الجدران
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 200, 600, M_PI/4, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 800, 150, -M_PI/4, 60, false, true};
        
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 375, 0, 0, false, false};
	} else if (level == 12) {
        // مرحلة "الموشور المشتت"
        game.time_left = 50;
        // توزيع عوائق صغيرة عمودية
        for(int i=0; i<3; i++) {
            game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300 + i*200, 100 + i*200, M_PI/2, 150, false, false};
        }
        
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 100, 100, 0, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 700, M_PI/2, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 900, 400, 0, 70, false, true};
        
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 100, 650, 0, 0, false, false};
     } else if (level == 13) {
        // مرحلة "حلقة النار" - تحتاج دوران كامل للشعاع
        game.time_left = 60;
        // صندوق شبه مغلق حول الهدف
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 800, 300, 0, 200, false, false}; // سقف
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 800, 500, 0, 200, false, false}; // أرضية
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 900, 400, M_PI/2, 200, false, false}; // جدار خلفي
        
        // المرايا المطلوبة للالتفاف حول الصندوق
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 200, 100, M_PI/3, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 650, -M_PI/4, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 700, 400, M_PI/2, 60, false, true};
        
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 850, 400, 0, 0, false, false};
     } else if (level == 14) {
        // المصدر
        game.objects[0].x = 100;
        game.objects[0].y = 350;
        game.objects[0].angle = 0;

        // عائق عبقري يقطع المسار المباشر في المنتصف
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 350, M_PI/2, 200, false, false};

        // مرايا لتوجيه الشعاع حول العائق
        // المرآة الأولى تكسر الشعاع للأعلى
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 300, 350, M_PI/4, 60, false, true};
        // المرآة الثانية تكسر الشعاع لليمين فوق العائق
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 300, 150, -M_PI/4, 60, false, true};
        // المرآة الثالثة تنزله نحو العدسة
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 700, 150, M_PI/4, 60, false, true};

        // العدسة والهدف
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 700, 350, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 350, 0, 0, false, false};
    }
	else if (level == 15) {
		game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 300, M_PI/4, 500, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 300, 200, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 600, 500, M_PI/4, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 400, 300, M_PI/4, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 600, 100, 0, 0, false, false};
    } else if (level == 16) {
        game.time_left = 40;
        
        // الحائط في المنتصف
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 350, M_PI/2, 400, false, false};
        
        // العدسة (في البداية)
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 200, 350, 0, 0, false, true};
        
        // المرآة الأصلية
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 700, 150, M_PI/4, 60, false, true};
        
        // مرآة إضافية 1: لرفع الشعاع للأعلى في البداية
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 350, 350, M_PI/4, 60, false, true};
        
        // مرآة إضافية 2: لكسر الشعاع عند الوصول للحائط (تغيير المسار أفقياً)
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 150, 0, 60, false, true};
        
        // الهدف
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 850, 350, 0, 0, false, false};
    } else if (level == 17) {
        game.time_left = 35;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 400, 200, 0, 300, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 600, 600, 0, 300, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 200, 400, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 300, M_PI/3, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 100, 0, 0, false, false};
    } else if (level == 18) {
        game.time_left = 30;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 400, M_PI/2, 500, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 250, M_PI/6, 500, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 300, 300, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 600, 400, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 450, 500, M_PI/6, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 550, 400, M_PI/6, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 800, 200, 0, 0, false, false};
    } else if (level == 19) {
        game.time_left = 45;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 500, M_PI/2, 500, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 250, 250, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 750, 250, -M_PI/4, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 300, M_PI/3, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 750, 600, 0, 0, false, false};
    } else if (level == 20) {
        game.time_left = 25;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 200, 500, M_PI/2, 500, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 400, M_PI/2, 500, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 250, M_PI/6, 500, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 200, 100, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 200, 600, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 350, 0, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 450, 0, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 350, 0, 0, false, false};
    } else if (level == 21) {
        game.time_left = 50;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300, 300, M_PI/2, 200, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 150, 150, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 600, 600, M_PI/3, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 150, 0, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 250, 0, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 400, 150, 0, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 800, 50, 0, 0, false, false};
    } else if (level == 22) {
        game.time_left = 40;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300, 300, M_PI/2, 200, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 400, 400, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 200, 200, M_PI/4, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 600, 600, -M_PI/4, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 700, 100, 0, 0, false, false};
    } else if (level == 23) {
        game.time_left = 35;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 200, 0, 600, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300, 200, 0, 600, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 200, 500, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 800, 500, M_PI/2, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 400, 300, M_PI/2, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 350, 0, 0, false, false};
    } else if (level == 24) {
        game.time_left = 30;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300, 200, 0, 600, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 200, 0, 600, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 300, 200, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 300, 500, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 600, 350, M_PI/6, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 300, 250, M_PI/3, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 600, 0, 0, false, false};
    } else if (level == 25) {
        game.time_left = 45;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 400, 500, M_PI/2, 300, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 200, 300, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 200, M_PI/4, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 300, 100, M_PI/2, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 250, 300, M_PI/4, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 700, 700, 0, 0, false, false};
    } else if (level == 26) {
        game.time_left = 40;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 600, 500, M_PI/2, 300, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 400, 500, M_PI/2, 300, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 150, 350, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 450, 350, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 750, 350, M_PI/2, 70, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 550, 250, M_PI/2, 40, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 450, 150, M_PI/2, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 350, 0, 0, false, false};
    } else if (level == 27) {
        game.time_left = 35;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 600, 400, 0, 400, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 200, 500, 0, 200, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 300, 0, 400, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 300, 500, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 700, 200, -M_PI/6, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 200, -M_PI/2, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 400, 200, -M_PI/4, 20, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 750, 500, 0, 0, false, false};
    } else if (level == 28) {
        game.time_left = 30;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 700, 300, 0, 300, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300, 500, 0, 100, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 250, 400, 0, 500, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 200, 200, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 400, 400, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 100, 200, M_PI/3, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 400, 200, M_PI/3, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 200, 200, M_PI/3, 20, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 500, 0, 0, false, false};
    } else if (level == 29) {
        game.time_left = 45;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 300, 600, M_PI/2, 400, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 200, 400, M_PI/2, 300, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 150, 200, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 400, 150, M_PI/4, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 350, 200, M_PI/2, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 350, 200, M_PI/4, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 550, 450, 0, 0, false, false};
    } else if (level == 30) {
        game.time_left = 60;
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 100, 0, 800, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 800, 260, 0, 800, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 200, 400, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_LENS, 400, 600, 0, 0, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 500, 400, M_PI/2, 10, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 100, 400, M_PI/2, 30, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 300, 400, M_PI/2, 80, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 400, 250, 0, 0, false, false};
    }
}
