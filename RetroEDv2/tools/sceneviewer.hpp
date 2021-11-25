#ifndef SCENEVIEWER_H
#define SCENEVIEWER_H

class SceneCamera
{
public:
    float m_near = 0.01f;
    float m_far  = 1000.0f;

    Vector3<float> pos = Vector3<float>(0.0f, 0.0f, 0.0f);

    Vector2<float> lastMousePos = Vector2<float>(0.0f, 0.0f);

    float aspectRatio = 0.0f;
};

enum SceneManagerTool {
    TOOL_MOUSE,
    TOOL_SELECT,
    TOOL_PENCIL,
    TOOL_ERASER,
    TOOL_ENTITY,
    TOOL_PARALLAX,
};

enum ScriptEvents {
    EVENT_RSDKDRAW,
    EVENT_RSDKLOAD,
    EVENT_RSDKEDIT,
};

class SceneViewer : public QOpenGLWidget
{
public:
    class VariableValue
    {
    public:
        QString name = "";
        int value    = 0;

        VariableValue() {}
    };

    class VariableInfo
    {
    public:
        QString name = "";
        QList<VariableValue> values;

        VariableInfo() {}
    };

    class ObjectInfo
    {
    public:
        QString name = "";
        QList<VariableInfo> variables;

        ObjectInfo() {}
    };

    class EntityInfo
    {
    public:
        ushort slotID      = 0;
        ushort prevSlotID  = 0;
        byte type          = 0;
        byte propertyValue = 0;
        Vector2<float> pos = Vector2<float>(0, 0);

        FormatHelpers::Scene::Object::VariableInfo variables[0x0F];
        QList<RSDKv5::Scene::VariableValue> customVars;

        Rect<int> box = Rect<int>(-0x10, -0x10, 0x10, 0x10); // selection box

        EntityInfo() {}
    };

    SceneViewer(QWidget *parent);
    ~SceneViewer();

    struct TextureInfo {
        TextureInfo() {}

        QString name               = "";
        QOpenGLTexture *texturePtr = nullptr;
    };

    void loadScene(QString path, byte ver);

    void updateScene();
    void drawScene();
    void unloadScene();

    // viewing properties
    float zoom = 1.0f;
    inline float invZoom() { return 1.0f / zoom; }

    byte gameType = ENGINE_NONE;

    QString dataPath = "";
    FormatHelpers::Gameconfig gameConfig;

    QList<ObjectInfo> objects;
    QList<EntityInfo> entities;

    FormatHelpers::Scene scene;
    FormatHelpers::Background background;
    FormatHelpers::Chunks chunkset;
    FormatHelpers::Stageconfig stageConfig;

    RSDKv4::TileConfig tileconfig;

    QString currentFolder = "Blank";

    QPoint reference;

    // General Editing
    byte curTool            = TOOL_MOUSE;
    bool selecting          = false;
    Vector2<float> mousePos = Vector2<float>(0.0f, 0.0f);

    // Layer Editing
    Vector2<float> tilePos = Vector2<float>(0.0f, 0.0f);
    Vector2<bool> tileFlip = Vector2<bool>(false, false);
    int selectedChunk      = -1;
    int selectedLayer      = -1;
    bool visibleLayers[9];

    // Collision
    bool showPlaneA = false;
    bool showPlaneB = false;

    // Entity Editing
    int selectedObject = -1; // placing
    int selectedEntity = -1; // viewing

    // Parallax Editing
    bool showParallax      = false;
    int selectedScrollInfo = -1;

    // Camera
    SceneCamera cam;

    QList<PaletteColour> tilePalette;
    QList<QImage> tiles;
    QList<QImage> chunks;
    QImage missingObj;

    bool showPixelGrid = false;
    bool showChunkGrid = false;
    bool showTileGrid  = false;

    Compilerv2 compilerv2;
    Compilerv3 compilerv3;
    Compilerv4 compilerv4;

    // passed from main
    QLabel *statusLabel      = nullptr;
    QScrollBar *sbHorizontal = nullptr;
    QScrollBar *sbVertical   = nullptr;

    Colour bgColour    = Colour(0x20, 0x20, 0x20);
    Colour altBGColour = Colour(0x30, 0x30, 0x30);

    inline int layerWidth(byte layer)
    {
        int weight = scene.width;

        if (layer > 0) {
            weight = background.layers[layer - 1].width;
        }
        return weight;
    }

    inline int layerHeight(byte layer)
    {
        int height = scene.height;

        if (layer > 0) {
            height = background.layers[layer - 1].height;
        }
        return height;
    }

    inline QList<QList<ushort>> layerLayout(byte layer)
    {
        QList<QList<ushort>> layout = scene.layout;

        if (layer > 0) {
            layout = background.layers[layer - 1].layout;
        }
        return layout;
    }

    int prevStoredW = -1, prevStoredH = -1;
    int storedW, storedH;
    int sceneWidth, sceneHeight;

    int prevSprite = -1;

    int addGraphicsFile(QString sheetPath);
    void removeGraphicsFile(QString sheetPath, int slot);

    inline void getTileVerts(QVector2D *arr, int index, int tileIndex, byte direction)
    {
        float w = tilesetTexture->width(), h = tilesetTexture->height();

        float tx = 0.0f;
        float ty = tileIndex / h;
        float tw = 0x10 / w;
        float th = 0x10 / h;

        switch (direction) {
            case 0:
            default: {
                arr[index + 0].setX(tx);
                arr[index + 0].setY(ty);

                arr[index + 1].setX(tx + tw);
                arr[index + 1].setY(ty);

                arr[index + 2].setX(tx + tw);
                arr[index + 2].setY(ty + th);

                arr[index + 3].setX(tx + tw);
                arr[index + 3].setY(ty + th);

                arr[index + 4].setX(tx);
                arr[index + 4].setY(ty + th);

                arr[index + 5].setX(tx);
                arr[index + 5].setY(ty);
                break;
            }
            case 1: {
                arr[index + 0].setX(tx + tw);
                arr[index + 0].setY(ty);

                arr[index + 1].setX(tx);
                arr[index + 1].setY(ty);

                arr[index + 2].setX(tx);
                arr[index + 2].setY(ty + th);

                arr[index + 3].setX(tx);
                arr[index + 3].setY(ty + th);

                arr[index + 4].setX(tx + tw);
                arr[index + 4].setY(ty + th);

                arr[index + 5].setX(tx + tw);
                arr[index + 5].setY(ty);
                break;
            }
            case 2: {
                arr[index + 0].setX(tx);
                arr[index + 0].setY(ty + th);

                arr[index + 1].setX(tx + tw);
                arr[index + 1].setY(ty + th);

                arr[index + 2].setX(tx + tw);
                arr[index + 2].setY(ty);

                arr[index + 3].setX(tx + tw);
                arr[index + 3].setY(ty);

                arr[index + 4].setX(tx);
                arr[index + 4].setY(ty);

                arr[index + 5].setX(tx);
                arr[index + 5].setY(ty + th);
                break;
            }
            case 3: {
                arr[index + 0].setX(tx + tw);
                arr[index + 0].setY(ty + th);

                arr[index + 1].setX(tx);
                arr[index + 1].setY(ty + th);

                arr[index + 2].setX(tx);
                arr[index + 2].setY(ty);

                arr[index + 3].setX(tx);
                arr[index + 3].setY(ty);

                arr[index + 4].setX(tx + tw);
                arr[index + 4].setY(ty);

                arr[index + 5].setX(tx + tw);
                arr[index + 5].setY(ty + th);
                break;
            }
        }
    }

    inline QOpenGLTexture *createTexture(QImage src)
    {
        QOpenGLTexture *tex = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
        tex->create();
        tex->bind();
        tex->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::WrapMode::Repeat);
        tex->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::WrapMode::Repeat);
        tex->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
        tex->setFormat(QOpenGLTexture::RGBA8_UNorm);
        tex->setSize(src.width(), src.height());
        tex->setData(src, QOpenGLTexture::MipMapGeneration::GenerateMipMaps);
        return tex;
    }

    QOpenGLTexture *tilesetTexture = nullptr;

    void drawTile(float XPos, float YPos, float ZPos, int tileX, int tileY, byte direction);

    void drawRectangle(int x, int y, int width, int height, byte r, byte g, byte b, int a);

    void drawSprite(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID);
    void drawSpriteFlipped(int XPos, int YPos, int width, int height, int sprX, int sprY, int direction,
                           int sheetID);
    void drawSpriteScaled(int direction, int XPos, int YPos, int pivotX, int pivotY, int scaleX,
                          int scaleY, int width, int height, int sprX, int sprY, int sheetID);
    void drawSpriteRotated(int direction, int XPos, int YPos, int pivotX, int pivotY, int sprX,
                           int sprY, int width, int height, int rotation, int sheetID);
    void drawSpriteRotozoom(int direction, int XPos, int YPos, int pivotX, int pivotY, int sprX,
                            int sprY, int width, int height, int rotation, int scale, int sheetID);

    void drawBlendedSprite(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID);
    void drawAlphaBlendedSprite(int XPos, int YPos, int width, int height, int sprX, int sprY,
                                int alpha, int sheetID);
    void drawAdditiveBlendedSprite(int XPos, int YPos, int width, int height, int sprX, int sprY,
                                   int alpha, int sheetID);
    void drawSubtractiveBlendedSprite(int XPos, int YPos, int width, int height, int sprX, int sprY,
                                      int alpha, int sheetID);

    void callGameEvent(byte eventID, int entityID);

    bool objectsLoaded  = false;
    int activeVar       = -1;
    int activeVarObj    = -1;
    int variableID      = -1;
    int variableValue   = -1;
    bool returnVariable = false;

    void addEditorVariable(QString name);
    void setActiveVariable(QString name);
    void addEnumVariable(QString name, int value);

    void refreshResize();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    QSize sizeHint() const { return QSize(0, 0); }

private:
    QOpenGLVertexArrayObject screenVAO, rectVAO;
    QList<TextureInfo> objectSprites;

    QOpenGLTexture *rsPlayerSprite = nullptr;

    QTimer *updateTimer = nullptr;

    QMatrix4x4 matView;

    int shaderID = -1;
    Shader primitiveShader;
    Shader spriteShader;

    // void processParallax(Scene::SceneLayer &layer);

    inline QMatrix4x4 getProjectionMatrix()
    {
        QMatrix4x4 matWorld;
        cam.aspectRatio = storedW / (float)storedH;
        matWorld.ortho(0.0f, (float)storedW, (float)storedH, 0.0f, -16.0f, 16.0f);
        return matWorld;
    }

    inline void drawLine(float x1, float y1, float z1, float x2, float y2, float z2, float scale,
                         Vector4<float> colour, Shader &shader);

    inline void drawRect(float x, float y, float z, float w, float h, Vector4<float> colour,
                         Shader &shader, bool outline = false)
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

        shader.use();
        if (outline) {
            // top
            drawLine(x, y, z, x + w, y, z, zoom, colour, shader);
            // bottom
            drawLine(x, y + h, z, x + w, y + h, z, zoom, colour, shader);
            // left
            drawLine(x, y, z, x, y + h, z, zoom, colour, shader);
            // right
            drawLine(x + w, y, z, x + w, y + h, z, zoom, colour, shader);
        }
        else {
            shader.setValue("colour", QVector4D(colour.x, colour.y, colour.z, colour.w));

            rectVAO.bind();

            QMatrix4x4 matModel;
            matModel.scale(w, h, 1.0f);
            matModel.translate(x / w, y / h, z);
            shader.setValue("model", matModel);

            f->glDrawArrays(GL_TRIANGLES, 0, 6);

            rectVAO.release();
        }
    }
};

#endif // SCENEVIEWER_H
