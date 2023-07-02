
#define MENU_TAB_DEFAULT 0
#define MENU_TAB_RIFLE 1
#define MENU_TAB_SHOTGUN 2
#define MENU_TAB_SMG 3
#define MENU_TAB_PISTOL 4
#define MENU_TAB_SNIPER 5

#define SELECT_BOXES_NORMAL 0
#define SELECT_BOXES_CORNER 1
#define SELECT_BOXES_THREED 2

static float smoothness1 = 4;
static float smoothness2 = 4;
static float smoothness3 = 4;
static float smoothness4 = 4;
static float smoothness5 = 4;

static float fov1 = 35;
static float fov2 = 35;
static float fov3 = 35;
static float fov4 = 35;
static float fov5 = 35;

bool DrawMenu = true;
bool WeaponConfigs = false;
bool Aimbot = true;
bool TargetLines = true;
bool triggerbot = false;
bool autofire = false;
bool Snaplines = true;
bool distance = false;
bool skell = false;
bool bhead = false;
bool freezepawns = false;
bool brainbow = false;
bool fill = false;
bool fastbullets = false;
bool MaGicBullEts = false;

int smooth = 7;
int delay = 1;
int offset_value = 1;
float fieldofview = 10;
float max_distance = 300;
float AimFOV = 180;

static int box_type = 0;

static const char* box[]
{
	"Normal",
	"Corner",
	"ThreeD"
};