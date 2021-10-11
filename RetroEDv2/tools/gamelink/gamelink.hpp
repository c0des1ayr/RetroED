#ifndef GAMELINK_HPP
#define GAMELINK_HPP

typedef uint bool32;

enum Scopes {
    SCOPE_NONE,
    SCOPE_GLOBAL,
    SCOPE_STAGE,
};

enum InkEffects {
    INK_NONE,
    INK_BLEND,
    INK_ALPHA,
    INK_ADD,
    INK_SUB,
    INK_LOOKUP,
    INK_MASKED,
    INK_UNMASKED,
};

enum DrawFX { FX_NONE = 0, FX_FLIP = 1, FX_ROTATE = 2, FX_SCALE = 4 };

enum FlipFlags { FLIP_NONE, FLIP_X, FLIP_Y, FLIP_XY };

enum VarTypes {
    VAR_UINT8,
    VAR_UINT16,
    VAR_UINT32,
    VAR_INT8,
    VAR_INT16,
    VAR_INT32,
    VAR_ENUM,
    VAR_BOOL,
    VAR_STRING,
    VAR_VECTOR2,
    VAR_UNKNOWN,
    VAR_COLOUR,
};

struct GameObject {
    short objectID;
    byte active;
};

struct GameEntity {
    Vector2<int> position;
    Vector2<int> scale;
    Vector2<int> velocity;
    Vector2<int> updateRange;
    int angle;
    int alpha;
    int rotation;
    int groundVel;
    int depth;
    ushort group;
    ushort objectID;
    bool32 inBounds;
    bool32 isPermanent;
    bool32 tileCollisions;
    bool32 interaction;
    bool32 onGround;
    byte active;
    byte filter;
    byte direction;
    byte drawOrder;
    byte collisionLayers;
    byte collisionPlane;
    byte collisionMode;
    byte drawFX;
    byte inkEffect;
    byte visible;
    byte activeScreens;
};

struct GameEntityBase : GameEntity {
    void *data[0x100];
};

struct GameObjectInfo {
    uint hash[4];
    void (*update)(void);
    void (*lateUpdate)(void);
    void (*staticUpdate)(void);
    void (*draw)(void);
    void (*create)(void *data);
    void (*stageLoad)(void);
    void (*editorDraw)(void);
    void (*editorLoad)(void);
    void (*serialize)(void);
    GameObject **type;
    int entitySize;
    int objectSize;
    const char *name;
};

struct SceneInfo {
    GameEntity *entity;
    void *listData;
    void *listCategory;
    int timeCounter;
    int currentDrawGroup;
    int currentScreenID;
    ushort listPos;
    ushort entitySlot;
    ushort createSlot;
    ushort classCount;
    bool32 inEditor;
    bool32 effectGizmo;
    bool32 debugMode;
    bool32 useGlobalScripts;
    bool32 timeEnabled;
    byte activeCategory;
    byte categoryCount;
    byte state;
    byte filter;
    byte milliseconds;
    byte seconds;
    byte minutes;
};

struct EngineInfo {
    char engineInfo[0x40];
    char gameSubname[0x100];
    char version[0x10];
};

typedef struct {
    int platform;
    int language;
    int region;
} SKUInfo;

struct InputState {
    bool32 down;
    bool32 press;
    int keyMap;
};

struct ControllerState {
    InputState keyUp;
    InputState keyDown;
    InputState keyLeft;
    InputState keyRight;
    InputState keyA;
    InputState keyB;
    InputState keyC;
    InputState keyX;
    InputState keyY;
    InputState keyZ;
    InputState keyStart;
    InputState keySelect;
};

struct AnalogState {
    InputState keyUp;
    InputState keyDown;
    InputState keyLeft;
    InputState keyRight;
    InputState keyStick;
    float deadzone;
    float hDelta;
    float vDelta;
};

struct TriggerState {
    InputState key1;
    InputState key2;
    float unknown1;
    float unknown2;
};

struct TouchMouseData {
    float x[0x10];
    float y[0x10];
    bool32 down[0x10];
    byte count;
};

struct UnknownInfo {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
};

#define SCREEN_XMAX  (1280)
#define SCREEN_YSIZE (240)
struct ScreenInfo {
    ushort frameBuffer[SCREEN_XMAX * SCREEN_YSIZE];
    Vector2<int> position;
    int width;
    int height;
    int centerX;
    int centerY;
    int pitch;
    int clipBound_X1;
    int clipBound_Y1;
    int clipBound_X2;
    int clipBound_Y2;
    int waterDrawPos;
};

struct Matrix {
    int values[4][4];
};

struct TextInfo {
    ushort *text;
    ushort textLength;
    ushort length;
};

struct Hitbox {
    short left;
    short top;
    short right;
    short bottom;
};

struct SpriteFrame {
    ushort sprX;
    ushort sprY;
    ushort width;
    ushort height;
    short pivotX;
    short pivotY;
    ushort delay;
    short id;
    byte sheetID;
    byte hitboxCnt;
    Hitbox hitboxes[8];
};

struct Animator {
    SpriteFrame *framePtrs;
    int frameID;
    short animationID;
    short prevAnimationID;
    short animationSpeed;
    short animationTimer;
    short frameDelay;
    short frameCount;
    byte loopIndex;
    byte rotationFlag;
};

struct ScrollInfo {
    int tilePos;
    int parallaxFactor;
    int scrollSpeed;
    int scrollPos;
    byte deform;
    byte unknown;
};

struct ScanlineInfo {
    Vector2<int> position;
    Vector2<int> deform;
};

struct TileLayer {
    byte behaviour;
    byte drawLayer[4];
    byte widthShift;
    byte heightShift;
    ushort width;
    ushort height;
    Vector2<int> position;
    int parallaxFactor;
    int scrollSpeed;
    int scrollPos;
    int deformationOffset;
    int deformationOffsetW;
    int deformationData[0x400];
    int deformationDataW[0x400];
    void (*scanlineCallback)(ScanlineInfo *);
    ushort scrollInfoCount;
    ScrollInfo scrollInfo[0x100];
    uint name[4];
    ushort *layout;
    byte *lineScroll;
};

struct GameInfo {
    void *functionPtrs;
    void *APIPtrs;
    SKUInfo *currentSKU;
    EngineInfo *engineInfo;
    SceneInfo *sceneInfo;
    ControllerState *controller;
    AnalogState *stickL;
    AnalogState *stickR;
    TriggerState *triggerL;
    TriggerState *triggerR;
    TouchMouseData *touchMouse;
    UnknownInfo *unknown;
    ScreenInfo *screenInfo;
    void *modPtrs;
};

extern byte *gameGlobalVariablesPtr;

namespace FunctionTable
{
void initGameOptions(void **options, int size);
} // namespace FunctionTable

extern SceneInfo sceneInfo;
extern EngineInfo engineInfo;
extern SKUInfo skuInfo;
extern ControllerState controller[5];
extern AnalogState stickL[5];
extern AnalogState stickR[5];
extern TriggerState triggerL[5];
extern TriggerState triggerR[5];
extern TouchMouseData touchMouse;
extern UnknownInfo unknownInfo;
extern ScreenInfo screens[4];

#include "gamestorage.hpp"
#include "gamemath.hpp"
#include "gameobjects.hpp"
#include "gamematrix.hpp"
#include "gametext.hpp"
#include "gamedraw.hpp"

class GameLink
{
public:
    GameLink();

    void Setup();
    void LinkGameObjects(QString gameName = "Game");

    GameObjectInfo *GetObjectInfo(QString name);
    GameObjectInfo *GetObjectInfo(int type);
};

extern GameLink gameLink;

#endif // GAMELINK_HPP