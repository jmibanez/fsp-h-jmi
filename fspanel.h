typedef struct task
{
	struct task *next;
	Window win;
	Pixmap icon;
	Pixmap mask;
	char *name;
	int pos_x;
	int width;
//	unsigned long desktop; // me
	unsigned int focused:1;
	unsigned int iconified:1;
//	unsigned int shaded:1; // me
	unsigned int icon_copied:1;
	unsigned int demands_attention:1; // me
}
task;

typedef struct taskbar
{
	Window win;
	task *task_list;
	int num_tasks;
	int my_desktop;
	unsigned int hidden:1;
	unsigned int at_top:1;
}
taskbar;

