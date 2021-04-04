#ifndef Compilerv4_H
#define Compilerv4_H

#define SCRIPTDATA_COUNT_v4 (0x100000)
#define JUMPTABLE_COUNT_v4  (0x10000)
#define FUNCTION_COUNT_v4   (0x200)

#define JUMPSTACK_COUNT_v4 (0x400)
#define FUNCSTACK_COUNT_v4 (0x400)
#define FORSTACK_COUNT_v4  (0x400)

#define SPRITEFRAME_COUNT_v4 (0x1000)

#define TYPEGROUP_COUNT (0x103)

class Compilerv4
{
public:
    Compilerv4();

public:
    struct ScriptPtr {
        int m_scriptCodePtr;
        int m_jumpTablePtr;
    };

    struct ObjectScript {
        ScriptPtr eventRSDKDraw;
        ScriptPtr eventRSDKLoad;
        byte spriteSheetID;
        int frameListOffset;
        byte spriteFrameCount;
    };

    struct ScriptEngine {
        int operands[10];
        int tempValue[8];
        int arrayPosition[9];
        int checkResult;
    };

    struct Entity {
        Entity()
        {
            memset(values, 0, 48 * sizeof(int));
            memset(flailing, 0, 5 * sizeof(byte));
        }
        int XPos      = 0;
        int YPos      = 0;
        int XVelocity = 0;
        int YVelocity = 0;
        int speed     = 0;
        int values[48];
        int state               = 0;
        int angle               = 0;
        int scale               = 0;
        int rotation            = 0;
        int alpha               = 0;
        int animationTimer      = 0;
        int animationSpeed      = 0;
        int camOffsetX          = 0;
        int lookPos             = 0;
        ushort typeGroup        = 0;
        byte type               = 0;
        byte propertyValue      = 0;
        byte priority           = 1;
        signed char drawOrder   = 3;
        byte direction          = 0;
        byte inkEffect          = 0;
        byte animation          = 0;
        byte prevAnimation      = 0;
        byte frame              = 0;
        byte collisionMode      = 0;
        byte collisionPlane     = 0;
        signed char controlMode = 0;
        byte controlLock        = 0;
        byte pushing            = 0;
        byte visible            = 1;
        byte tileCollisions     = 1;
        byte objectInteractions = 1;
        byte gravity            = 0;
        byte left               = 0;
        byte right              = 0;
        byte up                 = 0;
        byte down               = 0;
        byte jumpPress          = 0;
        byte jumpHold           = 0;
        byte trackScroll        = 0;
        byte flailing[5];
    };

    struct SpriteFrame {
        int sprX;
        int sprY;
        int width;
        int height;
        int pivotX;
        int pivotY;
        byte sheetID;
        byte hitboxID;
    };
    enum FlipFlags { FLIP_NONE, FLIP_X, FLIP_Y, FLIP_XY };
    enum InkFlags { INK_NONE, INK_BLEND, INK_ALPHA, INK_ADD, INK_SUB };
    enum DrawFXFlags { FX_SCALE, FX_ROTATE, FX_ROTOZOOM, FX_INK, FX_TINT, FX_FLIP };

    enum ScriptEvents { EVENT_RSDKDRAW = 0, EVENT_RSDKLOAD = 1 };

    ObjectScript m_objectScriptList[OBJECT_COUNT];
    ScriptPtr m_functionList[FUNCTION_COUNT_v4];

    int m_scriptData[SCRIPTDATA_COUNT_v4];
    int m_jumpTableData[JUMPTABLE_COUNT_v4];

    int m_jumpTableStack[JUMPSTACK_COUNT_v4];
    int m_jumpTableStackPos = 0;

    int m_foreachStack[FORSTACK_COUNT_v4];
    int m_foreachStackPos;

    QString m_scriptText = "";

    int m_scriptDataPos       = 0;
    int m_scriptDataOffset    = 0;
    int m_jumpTableDataPos    = 0;
    int m_jumpTableDataOffset = 0;
    int m_functionStackPos    = 0;

    int m_functionCount = 0;
    QString m_functionNames[FUNCTION_COUNT_v4];

    int m_publicAliasCount  = 0;
    int m_privateAliasCount = 0;

    int m_publicStaticVarCount  = 0;
    int m_privateStaticVarCount = 0;

    int m_publicTableCount  = 0;
    int m_privateTableCount = 0;

    int m_lineID = 0;

    int m_scriptCount = 0;

    int m_globalScriptCount     = 0;
    int m_globalScriptDataCount = 0;
    int m_globalJumpTableCount  = 0;

    bool m_scriptError = false;
    QString errorMsg   = "";
    QString errorPos   = "";
    QString m_errorScr = "";
    int m_errorLine    = 0;

    QString m_gamePlatform      = "STANDARD";
    QString m_gameRenderType    = "SW_RENDERING";
    QString m_gameHapticSetting = "USE_F_FEEDBACK";

    QList<QString> m_globalVariables;

    QString m_typeNames[OBJECT_COUNT];

    int findStringToken(QString &string, QString token, char stopID);

    void checkAliasText(QString &text);
    void checkStaticText(QString &text);
    void *checkTableText(QString &text);
    void convertArithmaticSyntax(QString &text);
    void convertIfWhileStatement(QString &text);
    void convertForeachStatement(QString &text);
    bool convertSwitchStatement(QString &text);
    void convertFunctionText(QString &text);
    void checkCaseNumber(QString &text);
    bool readSwitchCase(QString &text);
    void readTableValues(QString &text);
    void appendIntegerToSting(QString &text, int value);
    bool convertStringToInteger(QString &text, int *value);
    void copyAliasStr(QString &dest, QString text, bool arrayIndex);

    void parseScriptFile(QString scriptName, int scriptID);

    void clearScriptData();

    void writeBytecode(QString path);

    int m_objectEntityPos = 0;

    ScriptEngine m_scriptEng;
    Entity m_objectEntityList[ENTITY_COUNT];
    char scriptText[0x100];

    SpriteFrame scriptFrames[SPRITEFRAME_COUNT_v4];
    int scriptFrameCount = 0;

    int xScrollOffset = 0;
    int yScrollOffset = 0;

    void *m_viewer = nullptr;

    void processScript(int scriptCodePtr, int jumpTablePtr, byte scriptEvent);

    RSDKv4::Bytecode m_bytecode;

private:
    int sinVal512[0x200];
    int cosVal512[0x200];

    int sinVal256[0x100];
    int cosVal256[0x100];

    byte atanVal256[0x100 * 0x100];

    inline int sin512(int angle)
    {
        if (angle < 0)
            angle = 0x200 - angle;
        angle &= 0x1FFu;
        return sinVal512[angle];
    }

    inline int cos512(int angle)
    {
        if (angle < 0)
            angle = 0x200 - angle;
        angle &= 0x1FFu;
        return cosVal512[angle];
    }

    inline int sin256(int angle)
    {
        if (angle < 0)
            angle = 0x100 - angle;
        angle &= 0xFFu;
        return sinVal256[angle];
    }

    inline int cos256(int angle)
    {
        if (angle < 0)
            angle = 0x100 - angle;
        angle &= 0xFFu;
        return cosVal256[angle];
    }

    // Get Arc Tan value
    byte ArcTanLookup(int X, int Y);
};

#endif // Compilerv4_H