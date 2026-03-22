#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#define INITIAL_TIME 60
#define MAX_OBJECTS 20
#define MAX_BEAMS 100 // يجب تعريف الماكس هنا أيضاً

typedef enum { OBJ_SOURCE, OBJ_MIRROR, OBJ_TARGET, OBJ_WALL } ObjType;
typedef struct { double r, g, b; } Color;
typedef struct { double x1, y1, x2, y2; Color color; } Beam; // تعريف Beam بالكامل

typedef struct {
    ObjType type;
    double x, y, angle, length;
    bool hit;
    bool draggable; 
} GameObject;

extern struct {
    GameObject objects[MAX_OBJECTS];
    int obj_count;
    Beam beams[MAX_BEAMS]; // يجب أن تكون مصفوفة بنفس الحجم، وليس void*
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
        game.objects[0].angle = M_PI / 2; 
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
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 250, 250, 0, 250, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 850, 550, M_PI/2, 250, false, false};
        for(int i=0; i<4; i++)
             game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 100+i*200, 650, 0, 60, false, true};
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 900, 350, 0, 0, false, false};
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
        // جداران يشكلان نفقاً ضيقاً في المنتصف
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 200, 0, 400, false, false};
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 550, 0, 400, false, false};
        
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
	}
     else if (level >= 14) {
        // توليد مرحلة عشوائية "ذكية"
        game.time_left = 60;
        // وضع الهدف في مكان عشوائي في النصف اليمين من الشاشة
        game.objects[game.obj_count++] = (GameObject){OBJ_TARGET, 700 + rand()%200, 100 + rand()%500, 0, 0, false, false};
        
        // وضع 3 مرايا في أماكن عشوائية
        for(int i=0; i<3; i++) {
            game.objects[game.obj_count++] = (GameObject){OBJ_MIRROR, 200 + rand()%400, 100 + rand()%500, (rand()%360)*M_PI/180.0, 70, false, true};
        }
        
        // وضع حائط عائق في المنتصف
        game.objects[game.obj_count++] = (GameObject){OBJ_WALL, 500, 375, (rand()%2)*M_PI/2, 300, false, false};
    }
}
