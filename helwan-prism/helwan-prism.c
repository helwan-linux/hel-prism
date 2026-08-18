#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
/*gcc `pkg-config --cflags gtk+-3.0` helwan-prism.c levels.c -o helwan_game `pkg-config --libs gtk+-3.0` -lm*/

void load_level(int level);
// --- Game Settings ---
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 750
#define MAX_BEAMS 100
#define MAX_OBJECTS 20
#define TARGET_RADIUS 25 
#define INITIAL_TIME 60 

typedef struct { double r, g, b; } Color;
typedef struct { double x1, y1, x2, y2; Color color; } Beam;
typedef enum { OBJ_SOURCE, OBJ_MIRROR, OBJ_TARGET, OBJ_WALL, OBJ_LENS, OBJ_SPLITTER, OBJ_SEMI_MIRROR, OBJ_MOVING_OBSTACLE } ObjType;

typedef struct {
    ObjType type;
    double x, y, angle, length;
    bool hit;
    bool draggable; 
} GameObject;

typedef struct {
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
    double global_time; // لدعم حركة العوائق الزمنية
} GameState;

GameState game;

// --- Physics Engine ---
bool line_intersection(double x1, double y1, double x2, double y2, 
                       double x3, double y3, double x4, double y4, double *ix, double *iy) {
    double s1_x = x2 - x1, s1_y = y2 - y1;
    double s2_x = x4 - x3, s2_y = y4 - y3;
    double d = (-s2_x * s1_y + s1_x * s2_y);
    if (fabs(d) < 0.0001) return false;
    double s = (-s1_y * (x1 - x3) + s1_x * (y1 - y3)) / d;
    double t = ( s2_x * (y1 - y3) - s2_y * (x1 - x3)) / d;
    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        if (ix) *ix = x1 + (t * s1_x); 
        if (iy) *iy = y1 + (t * s1_y);
        return true;
    }
    return false;
}

// دالة تتبع الأشعة مع دعم الانقسام (Recursive Beam Tracing) للـ Splitters و Semi-Mirrors
void trace_beam(double cur_x, double cur_y, double cur_angle, Color l_color, int bounce) {
    if (bounce > 8 || game.beam_count >= MAX_BEAMS) return;

    double next_x = cur_x + cos(cur_angle) * 3000;
    double next_y = cur_y + sin(cur_angle) * 3000;
    double closest_dist = 3000;
    int hit_idx = -1;
    bool hit_target = false;
    double ix, iy;

    // حساب موقع الحواجز المتحركة مؤقتاً أثناء الفحص
    double current_obs_x = 0, current_obs_y = 0, current_obs_angle = 0, current_obs_len = 0;

    for (int i = 1; i < game.obj_count; i++) {
        GameObject *obj = &game.objects[i];
        double ox = obj->x;
        double oy = obj->y;
        double o_angle = obj->angle;

        if (obj->type == OBJ_MOVING_OBSTACLE) {
            ox += sin(game.global_time * 2.0) * 50.0; // حركة دورية بسيطة
        }

        if (obj->type == OBJ_TARGET) {
            double dx = ox - cur_x;
            double dy = oy - cur_y;
            double proj = dx * cos(cur_angle) + dy * sin(cur_angle);
            if (proj > 0) {
                double cx = cur_x + cos(cur_angle) * proj;
                double cy = cur_y + sin(cur_angle) * proj;
                double dist = hypot(cx - ox, cy - oy);
                if (dist <= TARGET_RADIUS) {
                    double d_to_t = hypot(cx - cur_x, cy - cur_y);
                    if (d_to_t < closest_dist) {
                        closest_dist = d_to_t;
                        next_x = cx; next_y = cy;
                        hit_idx = i; hit_target = true;
                    }
                }
            }
        } else {
            double x3 = ox - cos(o_angle) * obj->length / 2;
            double y3 = oy - sin(o_angle) * obj->length / 2;
            double x4 = ox + cos(o_angle) * obj->length / 2;
            double y4 = oy + sin(o_angle) * obj->length / 2;
            if (line_intersection(cur_x, cur_y, next_x, next_y, x3, y3, x4, y4, &ix, &iy)) {
                double dist = hypot(ix - cur_x, iy - cur_y);
                if (dist < closest_dist && dist > 1.0) {
                    closest_dist = dist;
                    next_x = ix; next_y = iy;
                    hit_idx = i; hit_target = false;
                    current_obs_x = ox; current_obs_y = oy; current_obs_angle = o_angle; current_obs_len = obj->length;
                }
            }
        }
    }

    if (game.beam_count < MAX_BEAMS) {
        game.beams[game.beam_count++] = (Beam){cur_x, cur_y, next_x, next_y, l_color};
    }

    if (hit_idx != -1) {
        if (hit_target) {
            game.objects[hit_idx].hit = true;
            game.level_cleared = true;
        } else {
            ObjType type = game.objects[hit_idx].type;
            if (type == OBJ_MIRROR) {
                double refl_angle = 2 * game.objects[hit_idx].angle - cur_angle;
                trace_beam(next_x, next_y, refl_angle, l_color, bounce + 1);
            } else if (type == OBJ_SEMI_MIRROR) {
                // يعكس جزء ويمرر جزء
                double refl_angle = 2 * game.objects[hit_idx].angle - cur_angle;
                trace_beam(next_x, next_y, refl_angle, (Color){l_color.r, l_color.g, l_color.b}, bounce + 1);
                trace_beam(next_x, next_y, cur_angle, (Color){l_color.r * 0.7, l_color.g * 0.7, l_color.b * 0.7}, bounce + 1);
            } else if (type == OBJ_SPLITTER) {
                // يشتت الشعاع لثلاثة أشعة بزوايا مختلفة
                trace_beam(next_x, next_y, cur_angle - 0.3, (Color){1.0, 0.2, 0.2}, bounce + 1);
                trace_beam(next_x, next_y, cur_angle, (Color){0.2, 1.0, 0.2}, bounce + 1);
                trace_beam(next_x, next_y, cur_angle + 0.3, (Color){0.2, 0.2, 1.0}, bounce + 1);
            }
        }
    }
}

void update_physics() {
    game.beam_count = 0;
    for(int i = 0; i < game.obj_count; i++) {
        if (game.objects[i].type == OBJ_TARGET) game.objects[i].hit = false;
    }

    if (game.game_over && !game.level_cleared) return;

    trace_beam(game.objects[0].x, game.objects[0].y, game.objects[0].angle, (Color){0.0, 1.0, 0.8}, 0);
}

// --- Timers & Demo ---
gboolean game_timer_cb(gpointer data) {
    if (!game.level_cleared && !game.game_over) {
        game.time_left--;
        game.global_time += 1.0;
        if (game.time_left <= 0) game.game_over = true;
        gtk_widget_queue_draw((GtkWidget*)data);
    }
    return TRUE;
}

gboolean demo_timer_cb(gpointer data) {
    if (!game.demo_mode) return FALSE;
    game.global_time += 0.1;
    for (int i = 1; i < game.obj_count; i++) {
        if (game.objects[i].type == OBJ_MIRROR || game.objects[i].type == OBJ_SEMI_MIRROR) {
            game.objects[i].angle += 0.02;
        }
    }
    update_physics();
    gtk_widget_queue_draw((GtkWidget*)data);
    return TRUE;
}

void on_demo_clicked(GtkWidget *widget, gpointer data) {
    game.demo_mode = !game.demo_mode;
    if (game.demo_mode) g_timeout_add(30, demo_timer_cb, data);
}

// --- UI ---
void show_about(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "Helwan Prism Ultimate\nLead Developer: Saeed Badreldin\nOS: Helwan Linux");
    gtk_dialog_run(GTK_DIALOG(dialog)); gtk_widget_destroy(dialog);
}

void show_help(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "CONTROLS:\n- Left Click: Drag\n- Right Click: Rotate\n- SPACE: Next Level\n- ESC: Restart");
    gtk_dialog_run(GTK_DIALOG(dialog)); gtk_widget_destroy(dialog);
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    update_physics();

    cairo_set_source_rgb(cr, 0.01, 0.01, 0.05); 
    cairo_paint(cr);

    for (int i = 0; i < game.beam_count; i++) {
        cairo_set_source_rgba(cr, game.beams[i].color.r, game.beams[i].color.g, game.beams[i].color.b, 0.3);
        cairo_set_line_width(cr, 10.0);
        cairo_move_to(cr, game.beams[i].x1, game.beams[i].y1);
        cairo_line_to(cr, game.beams[i].x2, game.beams[i].y2);
        cairo_stroke(cr);

        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_set_line_width(cr, 2.0);
        cairo_move_to(cr, game.beams[i].x1, game.beams[i].y1);
        cairo_line_to(cr, game.beams[i].x2, game.beams[i].y2);
        cairo_stroke(cr);
    }

    for (int i = 0; i < game.obj_count; i++) {
        GameObject *obj = &game.objects[i];
        cairo_save(cr);
        
        double render_x = obj->x;
        double render_y = obj->y;
        if (obj->type == OBJ_MOVING_OBSTACLE) {
            render_x += sin(game.global_time * 2.0) * 50.0;
        }

        cairo_translate(cr, render_x, render_y);
        cairo_rotate(cr, obj->angle);

        if (obj->type == OBJ_SOURCE) {
            cairo_set_source_rgb(cr, 0.3, 0.3, 0.3); 
            cairo_rectangle(cr, -15, -15, 30, 30);
        } 
        else if (obj->type == OBJ_MIRROR) {
            cairo_set_source_rgb(cr, 0.7, 0.7, 1.0); 
            cairo_rectangle(cr, -obj->length/2, -5, obj->length, 10);
        } 
        else if (obj->type == OBJ_SEMI_MIRROR) {
            cairo_set_source_rgb(cr, 0.5, 0.9, 0.9); // لون مختلف للمرآة النصف عاكسة
            cairo_rectangle(cr, -obj->length/2, -4, obj->length, 8);
        }
        else if (obj->type == OBJ_SPLITTER) {
            cairo_set_source_rgb(cr, 0.9, 0.5, 0.9); // موشور مشتت
            cairo_rectangle(cr, -20, -20, 40, 40);
        }
        else if (obj->type == OBJ_MOVING_OBSTACLE) {
            cairo_set_source_rgb(cr, 0.9, 0.3, 0.1); // حاجز متحرك
            cairo_rectangle(cr, -obj->length/2, -10, obj->length, 20);
        }
        else if (obj->type == OBJ_TARGET) {
            if (obj->hit || game.level_cleared) {
                cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
            } else {
                cairo_set_source_rgb(cr, 0.8, 0.0, 0.0);
            }
            cairo_arc(cr, 0, 0, TARGET_RADIUS, 0, 2 * M_PI);
        } 
        else if (obj->type == OBJ_WALL) {
            cairo_set_source_rgb(cr, 0.4, 0.2, 0.1); 
            cairo_rectangle(cr, -obj->length/2, -12, obj->length, 24);
        }
        
        cairo_fill(cr);
        cairo_restore(cr);
    }

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_font_size(cr, 22);
    char HUD[64];
    sprintf(HUD, "LEVEL %d  |  TIME: %d", game.current_level, game.time_left);
    cairo_move_to(cr, 20, 40);
    cairo_show_text(cr, HUD);

    if (game.level_cleared) {
        cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        cairo_set_font_size(cr, 45);
        cairo_move_to(cr, SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2);
        cairo_show_text(cr, "WINNER! PRESS SPACE");
    } 
    else if (game.game_over) {
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_set_font_size(cr, 45);
        cairo_move_to(cr, SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2);
        cairo_show_text(cr, "GAME OVER! PRESS ESC");
    }

    return FALSE;
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Escape) { load_level(game.current_level); gtk_widget_queue_draw(widget); }
    if (event->keyval == GDK_KEY_space && game.level_cleared) {
        load_level(game.current_level + 1); gtk_widget_queue_draw(widget);
    }
    return TRUE;
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (game.game_over || game.level_cleared) return FALSE;
    for (int i = 1; i < game.obj_count; i++) {
        if (game.objects[i].draggable && hypot(event->x - game.objects[i].x, event->y - game.objects[i].y) < 45) {
            game.dragged_obj_idx = i; game.is_rotating = (event->button == 3); return TRUE;
        }
    }
    return FALSE;
}

static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    game.dragged_obj_idx = -1; return TRUE;
}

static gboolean on_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    if (game.dragged_obj_idx != -1) {
        if (game.is_rotating) {
            game.objects[game.dragged_obj_idx].angle = atan2(event->y - game.objects[game.dragged_obj_idx].y, event->x - game.objects[game.dragged_obj_idx].x);
        } else {
            game.objects[game.dragged_obj_idx].x = event->x; game.objects[game.dragged_obj_idx].y = event->y;
        }
        gtk_widget_queue_draw(widget);
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    load_level(1); game.dragged_obj_idx = -1; game.global_time = 0.0;

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Helwan Prism Ultimate");
    
    GdkPixbuf *icon = gdk_pixbuf_new_from_file("icon.png", NULL);
    if(icon) { gtk_window_set_icon(GTK_WINDOW(win), icon); g_object_unref(icon); }

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_demo = gtk_button_new_with_label("Demo");
    GtkWidget *btn_help = gtk_button_new_with_label("Help"), *btn_about = gtk_button_new_with_label("About");
    
    gtk_box_pack_start(GTK_BOX(toolbar), btn_demo, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(toolbar), btn_about, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(toolbar), btn_help, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 5);

    GtkWidget *area = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), area, TRUE, TRUE, 0);
    gtk_widget_add_events(area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    g_signal_connect(btn_demo, "clicked", G_CALLBACK(on_demo_clicked), area);
    g_signal_connect(btn_help, "clicked", G_CALLBACK(show_help), win);
    g_signal_connect(btn_about, "clicked", G_CALLBACK(show_about), win);
    g_signal_connect(area, "draw", G_CALLBACK(on_draw), NULL);
    g_signal_connect(area, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(area, "button-release-event", G_CALLBACK(on_button_release), NULL);
    g_signal_connect(area, "motion-notify-event", G_CALLBACK(on_motion), NULL);
    g_signal_connect(win, "key-press-event", G_CALLBACK(on_key_press), NULL);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    g_timeout_add(1000, game_timer_cb, area);

    gtk_window_set_default_size(GTK_WINDOW(win), SCREEN_WIDTH, SCREEN_HEIGHT);
    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
