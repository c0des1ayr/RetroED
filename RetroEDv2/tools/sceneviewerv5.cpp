#include "includes.hpp"

static QVector3D rectVerticesv5[] = {
    QVector3D(-0.5f, -0.5f, -0.5f), QVector3D(0.5f, -0.5f, -0.5f), QVector3D(0.5f, 0.5f, -0.5f),
    QVector3D(0.5f, 0.5f, -0.5f),   QVector3D(-0.5f, 0.5f, -0.5f), QVector3D(-0.5f, -0.5f, -0.5f),
};

static QVector2D rectTexCoordsv5[] = {
    QVector2D(0.0f, 0.0f), QVector2D(1.0f, 0.0f), QVector2D(1.0f, 1.0f),
    QVector2D(1.0f, 1.0f), QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f),
};

SceneViewerv5::SceneViewerv5(QWidget *)
{
    setMouseTracking(true);

    this->setFocusPolicy(Qt::WheelFocus);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&SceneViewerv5::updateScene));
    timer->start(1000.0f / 60.0f);
}

SceneViewerv5::~SceneViewerv5()
{
    unloadScene();

    screenVAO.destroy();
    rectVAO.destroy();
}

void SceneViewerv5::loadScene(QString path)
{
    // unloading
    unloadScene();

    // loading
    QString pth      = path;
    QString basePath = pth.replace(QFileInfo(pth).fileName(), "");

    scene.read(path);

    tileconfig.read(basePath + "TileConfig.bin");
    stageConfig.read(basePath + "StageConfig.bin");

    bgColour    = Colour(scene.editorMetadata.backgroundColor1.red(),
                      scene.editorMetadata.backgroundColor1.green(),
                      scene.editorMetadata.backgroundColor1.blue());
    altBGColour = Colour(scene.editorMetadata.backgroundColor2.red(),
                         scene.editorMetadata.backgroundColor2.green(),
                         scene.editorMetadata.backgroundColor2.blue());

    objects.clear();
    entities.clear();

    QList<QString> objNames;
    QList<QString> varNames;
    objNames.append("Blank Object");

    if (stageConfig.loadGlobalObjects) {
        for (QString &obj : gameConfig.objects) {
            objNames.append(obj);
        }
    }

    for (QString &obj : stageConfig.objects) {
        objNames.append(obj);
    }

    int type = 0;
    for (RSDKv5::Scene::SceneObject &obj : scene.objects) {
        SceneObject object;
        object.name = obj.m_name.hashString();

        for (int i = 0; i < objNames.count(); ++i) {
            if (Utils::getMd5HashByteArray(objNames[i]) == obj.m_name.hash) {
                object.name = objNames[i];
                break;
            }
        }

        for (int v = 0; v < obj.variables.count(); ++v) {
            auto &var = obj.variables[v];
            VariableInfo variable;
            variable.name = var.m_name.hashString();
            variable.type = var.type;

            for (int i = 0; i < varNames.count(); ++i) {
                if (Utils::getMd5HashByteArray(varNames[i]) == var.m_name.hash) {
                    variable.name = varNames[i];
                    break;
                }
            }
            object.variables.append(variable);
        }

        for (RSDKv5::Scene::SceneEntity &ent : obj.entities) {
            SceneEntity entity;
            entity.slotID = ent.m_slotID;
            entity.type   = type;
            entity.pos.x  = ent.position.x / 65536.0f;
            entity.pos.y  = ent.position.y / 65536.0f;

            for (int v = 0; v < ent.variables.count(); ++v) {
                entity.variables.append(ent.variables[v]);
            }

            entities.append(entity);
        }
        objects.append(object);
        ++type;
    }

    if (QFile::exists(basePath + "16x16Tiles.gif")) {
        // setup tileset texture from png
        QImage tileset(basePath + "16x16Tiles.gif");
        m_tilesetTexture = createTexture(tileset);
        for (int i = 0; i < 0x400; ++i) {
            int tx         = ((i % (tileset.width() / 0x10)) * 0x10);
            int ty         = ((i / (tileset.width() / 0x10)) * 0x10);
            QImage tileTex = tileset.copy(tx, ty, 0x10, 0x10);

            tiles.append(tileTex);
        }

        // for (FormatHelpers::Chunks::Chunk &c : chunkset.chunks) {
        //    QImage img = c.getImage(tiles);
        //    chunks.append(img);
        //}
    }

    // objects
    objectSprites.clear();
    {
        TextureInfo tex;
        tex.name       = ":/icons/missing.png";
        missingObj     = QImage(tex.name);
        tex.texturePtr = createTexture(missingObj);
        objectSprites.append(tex);
    }

    m_rsPlayerSprite = createTexture(QImage(":/icons/player_v1.png"));
}

void SceneViewerv5::updateScene()
{
    this->repaint();

    if (statusLabel) {
        int mx = (int)((mousePos.x * invZoom()) + cam.pos.x);
        int my = (int)((mousePos.y * invZoom()) + cam.pos.y);
        statusLabel->setText(
            QString("Zoom: %1%, Mouse Position: (%2, %3), Chunk Position: (%4, %5), Selected Chunk: "
                    "%6, Selected Layer: %7 (%8), Selected Object: %9")
                .arg(zoom * 100)
                .arg(mx)
                .arg(my)
                .arg((int)mx / 0x10)
                .arg((int)my / 0x10)
                .arg(selectedTile)
                .arg(selectedLayer)
                .arg(selectedLayer >= 0 && selectedLayer < scene.layers.count()
                         ? scene.layers[selectedLayer].m_name
                         : "")
                .arg(selectedObject >= 0 && selectedObject < objects.count()
                         ? objects[selectedObject].name
                         : ""));
    }
}

void SceneViewerv5::drawScene()
{
    if (!m_tilesetTexture)
        return;
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    f->glBlendEquation(GL_FUNC_ADD);

    // pre-render
    if ((cam.pos.x * zoom) < 0 * zoom)
        cam.pos.x = (0 * zoom);

    if ((cam.pos.y * zoom) < 0 * zoom)
        cam.pos.y = (0 * zoom);

    if ((cam.pos.x * zoom) + storedW > (sceneWidth * 0x10) * zoom)
        cam.pos.x = ((sceneWidth * 0x10) - (storedW * invZoom()));

    if ((cam.pos.y * zoom) + storedH > (sceneHeight * 0x10) * zoom)
        cam.pos.y = ((sceneHeight * 0x10) - (storedH * invZoom()));

    // draw bg colours
    primitiveShader.use();
    primitiveShader.setValue("colour", QVector4D(altBGColour.r / 255.0f, altBGColour.g / 255.0f,
                                                 altBGColour.b / 255.0f, 1.0f));
    primitiveShader.setValue("projection", getProjectionMatrix());
    primitiveShader.setValue("view", QMatrix4x4());
    rectVAO.bind();

    int bgOffsetY = 0x80;
    bgOffsetY -= (int)cam.pos.y % 0x200;
    for (int y = bgOffsetY; y < (storedH + 0x80) * (zoom < 1.0f ? invZoom() : 1.0f); y += 0x100) {
        int bgOffsetX = (((y - bgOffsetY) % 0x200 == 0) ? 0x100 : 0x00);
        bgOffsetX += 0x80;
        bgOffsetX -= (int)cam.pos.x % 0x200;
        for (int x = bgOffsetX; x < (storedW + 0x80) * (zoom < 1.0f ? invZoom() : 1.0f); x += 0x200) {
            QMatrix4x4 matModel;
            matModel.scale(0x100 * zoom, 0x100 * zoom, 1.0f);
            matModel.translate(x / 256.0f, y / 256.0f, -15.0f);
            primitiveShader.setValue("model", matModel);

            f->glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    spriteShader.use();
    spriteShader.setValue("useAlpha", false);
    spriteShader.setValue("alpha", 1.0f);
    spriteShader.setValue("transparentColour", QVector3D(1.0f, 0.0f, 1.0f));

    QMatrix4x4 matWorld;
    QMatrix4x4 matView;
    spriteShader.setValue("projection", matWorld = getProjectionMatrix());
    spriteShader.setValue("view", matView = QMatrix4x4());
    f->glActiveTexture(GL_TEXTURE0);

    rectVAO.bind();

    int prevChunk = -1;
    Vector3<float> camOffset(0.0f, 0.0f, 0.0f);

    QVector4D pixelSolidityClrs[5] = { QVector4D(1.0f, 1.0f, 1.0f, 1.0f),
                                       QVector4D(1.0f, 1.0f, 0.0f, 1.0f),
                                       QVector4D(1.0f, 0.0f, 0.0f, 1.0f),
                                       QVector4D(0.0f, 0.0f, 0.0f, 0.0f),
                                       QVector4D(0.0f, 0.0f, 1.0f, 1.0f) };
    bool showCLayers[2]            = { showPlaneA, showPlaneB };

    for (int l = scene.layers.count() - 1; l >= 0; --l) {
        // TILE LAYERS
        QVector<QVector<ushort>> layout = scene.layers[l].layout;
        int width                       = scene.layers[l].width;
        int height                      = scene.layers[l].height;

        spriteShader.use();
        spriteShader.setValue("useAlpha", false);
        spriteShader.setValue("alpha", 1.0f);
        spriteShader.setValue("transparentColour", QVector3D(1.0f, 0.0f, 1.0f));
        rectVAO.bind();
        // manage properties
        camOffset = Vector3<float>(0.0f, 0.0f, 0.0f);

        if (selectedLayer >= 0) {
            if (selectedLayer == l) {
                spriteShader.setValue("useAlpha", false);
                spriteShader.setValue("alpha", 1.0f);
            }
            else {
                spriteShader.setValue("useAlpha", true);
                spriteShader.setValue("alpha", 0.5f);
            }
        }

        // draw
        m_tilesetTexture->bind();
        spriteShader.setValue("flipX", false);
        spriteShader.setValue("flipY", false);

        QVector3D *vertsPtr  = new QVector3D[height * width * 0x10 * 6];
        QVector2D *tVertsPtr = new QVector2D[height * width * 0x10 * 6];
        int vertCnt          = 0;

        for (int y = 0; y < height; ++y) {
            bool yBreak = false;
            for (int x = 0; x < width; ++x) {
                ushort tile = layout[y][x];
                if (tile != 0xFFFF) {
                    float xpos = (x * 0x10) - (cam.pos.x + camOffset.x);
                    float ypos = (y * 0x10) - (cam.pos.y + camOffset.y);
                    float zpos = selectedLayer == l ? 8.5 : (8 - l);

                    Rect<int> check = Rect<int>();
                    check.x         = (int)((xpos + 0x10) * zoom);
                    check.y         = (int)((ypos + 0x10) * zoom);
                    check.w         = (int)((xpos - (0x10 / 2)) * zoom);
                    check.h         = (int)((ypos - (0x10 / 2)) * zoom);

                    if (check.x < 0 || check.y < 0)
                        continue;
                    if (check.w >= storedW)
                        break;
                    if (check.h >= storedH) {
                        yBreak = true;
                        break;
                    }

                    vertsPtr[vertCnt + 0].setX(0.0f + (xpos / 0x10));
                    vertsPtr[vertCnt + 0].setY(0.0f + (ypos / 0x10));
                    vertsPtr[vertCnt + 0].setZ(zpos);

                    vertsPtr[vertCnt + 1].setX(1.0f + (xpos / 0x10));
                    vertsPtr[vertCnt + 1].setY(0.0f + (ypos / 0x10));
                    vertsPtr[vertCnt + 1].setZ(zpos);

                    vertsPtr[vertCnt + 2].setX(1.0f + (xpos / 0x10));
                    vertsPtr[vertCnt + 2].setY(1.0f + (ypos / 0x10));
                    vertsPtr[vertCnt + 2].setZ(zpos);

                    vertsPtr[vertCnt + 3].setX(1.0f + (xpos / 0x10));
                    vertsPtr[vertCnt + 3].setY(1.0f + (ypos / 0x10));
                    vertsPtr[vertCnt + 3].setZ(zpos);

                    vertsPtr[vertCnt + 4].setX(0.0f + (xpos / 0x10));
                    vertsPtr[vertCnt + 4].setY(1.0f + (ypos / 0x10));
                    vertsPtr[vertCnt + 4].setZ(zpos);

                    vertsPtr[vertCnt + 5].setX(0.0f + (xpos / 0x10));
                    vertsPtr[vertCnt + 5].setY(0.0f + (ypos / 0x10));
                    vertsPtr[vertCnt + 5].setZ(zpos);

                    byte direction = Utils::getBit(tile, 10) | (Utils::getBit(tile, 11) << 1);
                    getTileVerts(tVertsPtr, vertCnt, (tile & 0x3FF) * 0x10, direction);
                    vertCnt += 6;
                }
            }
            if (yBreak)
                break;
        }

        // Draw Tiles
        {
            QOpenGLVertexArrayObject vao;
            vao.create();
            vao.bind();

            QOpenGLBuffer vVBO2D;
            vVBO2D.create();
            vVBO2D.setUsagePattern(QOpenGLBuffer::StaticDraw);
            vVBO2D.bind();
            vVBO2D.allocate(vertsPtr, vertCnt * sizeof(QVector3D));
            spriteShader.enableAttributeArray(0);
            spriteShader.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

            QOpenGLBuffer tVBO2D;
            tVBO2D.create();
            tVBO2D.setUsagePattern(QOpenGLBuffer::StaticDraw);
            tVBO2D.bind();
            tVBO2D.allocate(tVertsPtr, vertCnt * sizeof(QVector2D));
            spriteShader.enableAttributeArray(1);
            spriteShader.setAttributeBuffer(1, GL_FLOAT, 0, 2, 0);

            QMatrix4x4 matModel;
            matModel.scale(0x10 * zoom, 0x10 * zoom, 1.0f);
            spriteShader.setValue("model", matModel);

            f->glDrawArrays(GL_TRIANGLES, 0, vertCnt);

            vao.release();
            tVBO2D.release();
            vVBO2D.release();

            delete[] vertsPtr;
            delete[] tVertsPtr;
        }

        // Collision Previews
        /*for (int c = 0; c < 2; ++c) {
            if (showCLayers[c]) {
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        ushort chunkID = layout[y][x];
                        if (chunkID != 0x0) {
                            for (int ty = 0; ty < 8; ++ty) {
                                for (int tx = 0; tx < 8; ++tx) {
                                    FormatHelpers::Chunks::Tile &tile =
                                        chunkset.chunks[chunkID].tiles[ty][tx];

                                    float xpos = (x * 0x80) + (tx * 0x10) - cam.pos.x;
                                    float ypos = (y * 0x80) + (ty * 0x10) - cam.pos.y;

                                    Rect<int> check = Rect<int>();
                                    check.x         = (int)((xpos + 0x10) * zoom);
                                    check.y         = (int)((ypos + 0x10) * zoom);
                                    check.w         = (int)(xpos * zoom);
                                    check.h         = (int)(ypos * zoom);

                                    if (check.x < 0 || check.y < 0 || check.w >= storedW
                                        || check.h >= storedH) {
                                        continue;
                                    }

                                    float yStore = ypos;
                                    // draw pixel collision
                                    byte solidity = 0;
                                    RSDKv4::Tileconfig::CollisionMask &cmask =
                                        tileconfig.collisionPaths[c][tile.tileIndex];

                                    solidity = !c ? tile.solidityA : tile.solidityB;

                                    if (solidity == 3)
                                        continue;

                                    for (byte cx = 0; cx < 16; ++cx) {
                                        int hm = cx;
                                        if (Utils::getBit(tile.direction, 0))
                                            hm = 15 - cx;

                                        if (!cmask.collision[hm].solid)
                                            continue;

                                        byte cy = cmask.collision[hm].height;
                                        byte ch = 16 - cy;
                                        if (Utils::getBit(tile.direction, 1) && !cmask.flipY) {
                                            cy = 0;
                                            ch = 16 - cmask.collision[hm].height;
                                        }
                                        else if (!Utils::getBit(tile.direction, 1) && cmask.flipY) {
                                            cy = 0;
                                            ch = cmask.collision[hm].height + 1;
                                        }
                                        else if (Utils::getBit(tile.direction, 1) && cmask.flipY) {
                                            cy = 15 - cmask.collision[hm].height;
                                            ch = cmask.collision[hm].height + 1;
                                        }

                                        ypos = yStore + (ch / 2.0);

                                        drawRect((xpos + cx) * zoom, (ypos + cy) * zoom, 15.45,
                                                 1 * zoom, ch * zoom, pixelSolidityClrs[solidity],
                                                 primitiveShader);
                                    }
                                }
                            }
                            spriteShader.use();
                            rectVAO.bind();
                        }
                    }
                }
            }
        }*/

        // PARALLAX
        if (l == selectedLayer && l >= 0) {
            if (scene.layers[l].type == 0 || scene.layers[l].type == 1) {
                primitiveShader.use();
                primitiveShader.setValue("projection", getProjectionMatrix());
                primitiveShader.setValue("view", QMatrix4x4());
                primitiveShader.setValue("useAlpha", false);
                primitiveShader.setValue("alpha", 1.0f);
                primitiveShader.setValue("projection", matWorld = getProjectionMatrix());
                primitiveShader.setValue("view", matView = QMatrix4x4());
                QMatrix4x4 matModel;
                primitiveShader.setValue("model", matModel);

                QOpenGLVertexArrayObject colVAO;
                colVAO.create();
                colVAO.bind();

                QList<QVector3D> verts;
                if (showParallax) {
                    int id = 0;
                    for (RSDKv5::Scene::ScrollIndexInfo &info : scene.layers[l].scrollInfos) {
                        bool isSelected = selectedScrollInfo == id;

                        Vector4<float> clr(1.0f, 1.0f, 0.0f, 1.0f);
                        if (isSelected)
                            clr = Vector4<float>(0.0f, 0.0f, 1.0f, 1.0f);
                        float zpos = (isSelected ? 15.55f : 15.5f);

                        if (scene.layers[l].type == 0) {
                            int w = (width * 0x10) * zoom;
                            drawLine(0.0f * zoom, (info.startLine - cam.pos.y) * zoom, zpos,
                                     (w - cam.pos.x) * zoom, (info.startLine - cam.pos.y) * zoom, zpos,
                                     clr, primitiveShader);

                            drawLine(0.0f * zoom, ((info.startLine + info.length) - cam.pos.y) * zoom,
                                     zpos, (w - cam.pos.x) * zoom,
                                     ((info.startLine + info.length) - cam.pos.y) * zoom, zpos, clr,
                                     primitiveShader);
                        }
                        else if (scene.layers[l].type == 1) {
                            int h = (height * 0x10) * zoom;
                            drawLine((info.startLine - cam.pos.x) * zoom, 0.0f * zoom, zpos,
                                     (info.startLine - cam.pos.x) * zoom, (h - cam.pos.y) * zoom, zpos,
                                     clr, primitiveShader);

                            drawLine(((info.startLine + info.length) - cam.pos.x) * zoom, 0.0f * zoom,
                                     zpos, ((info.startLine + info.length) - cam.pos.x) * zoom,
                                     (h - cam.pos.y) * zoom, zpos, clr, primitiveShader);
                        }

                        ++id;
                    }
                }
            }
        }
    }

    // ENTITIES
    m_prevSprite = -1;
    spriteShader.use();
    rectVAO.bind();
    spriteShader.setValue("flipX", false);
    spriteShader.setValue("flipY", false);
    spriteShader.setValue("useAlpha", false);
    spriteShader.setValue("alpha", 1.0f);
    for (int o = 0; o < entities.count(); ++o) {
        /*auto &curObj = m_compilerv4.m_objectScriptList[scene.objects[o].type];

        if (curObj.eventRSDKDraw.m_scriptCodePtr != SCRIPTDATA_COUNT - 1) {
            m_compilerv4.m_objectEntityPos = o;
            m_compilerv4.processScript(curObj.eventRSDKDraw.m_scriptCodePtr,
                                       curObj.eventRSDKDraw.m_jumpTablePtr,
                                       Compilerv4::EVENT_RSDKDRAW);
            continue;
        }*/

        spriteShader.use();
        rectVAO.bind();
        // Draw Object
        float xpos = entities[o].pos.x - (cam.pos.x);
        float ypos = entities[o].pos.y - (cam.pos.y);
        float zpos = 10.0f;

        int w = objectSprites[0].texturePtr->width(), h = objectSprites[0].texturePtr->height();
        if (m_prevSprite) {
            objectSprites[0].texturePtr->bind();
            m_prevSprite = 0;
        }

        Rect<int> check = Rect<int>();
        check.x         = (int)(xpos + (float)w) * zoom;
        check.y         = (int)(ypos + (float)h) * zoom;
        check.w         = (int)(xpos - (w / 2.0f)) * zoom;
        check.h         = (int)(ypos - (h / 2.0f)) * zoom;
        if (check.x < 0 || check.y < 0 || check.w >= storedW || check.h >= storedH) {
            continue;
        }

        QMatrix4x4 matModel;
        matModel.scale(w * zoom, h * zoom, 1.0f);

        matModel.translate(xpos / (float)w, ypos / (float)h, zpos);
        spriteShader.setValue("model", matModel);

        f->glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // TILE PREVIEW
    spriteShader.use();
    rectVAO.bind();
    spriteShader.setValue("useAlpha", true);
    spriteShader.setValue("alpha", 0.75f);
    if (selectedTile >= 0 && selectedLayer >= 0 && isSelecting && curTool == TOOL_PENCIL) {
        m_tilesetTexture->bind();
        float tx = tilePos.x;
        float ty = tilePos.y;

        tx *= invZoom();
        ty *= invZoom();

        float tx2 = tx + fmodf(cam.pos.x, 0x10);
        float ty2 = ty + fmodf(cam.pos.y, 0x10);

        // clip to grid
        tx -= fmodf(tx2, 0x10);
        ty -= fmodf(ty2, 0x10);

        // Draw Selected Tile Preview
        float xpos = tx + cam.pos.x;
        float ypos = ty + cam.pos.y;
        float zpos = 15.0f;

        xpos -= (cam.pos.x + camOffset.x);
        ypos -= (cam.pos.y + camOffset.y);

        byte direction = 0; // Utils::getBit(tile, 10) | (Utils::getBit(tile, 11) << 1);
        drawTile(xpos, ypos, zpos, 0, selectedTile * 0x10, direction);
    }

    // ENT PREVIEW
    spriteShader.use();
    rectVAO.bind();
    spriteShader.setValue("flipX", false);
    spriteShader.setValue("flipY", false);
    spriteShader.setValue("useAlpha", true);
    spriteShader.setValue("alpha", 0.75f);
    if (selectedObject >= 0 && isSelecting && curTool == TOOL_ENTITY) {
        bool flag = false;
        float ex  = tilePos.x;
        float ey  = tilePos.y;

        ex *= invZoom();
        ey *= invZoom();

        float cx = cam.pos.x;
        float cy = cam.pos.y;

        /*auto &curObj = m_compilerv4.m_objectScriptList[selectedObject];

        if (curObj.eventRSDKDraw.m_scriptCodePtr != ENTITY_COUNT - 1) {
            m_compilerv4.m_objectEntityList[ENTITY_COUNT - 1].type = selectedObject;
            m_compilerv4.m_objectEntityList[ENTITY_COUNT - 1].XPos = (ex + cx) * 65536.0f;
            m_compilerv4.m_objectEntityList[ENTITY_COUNT - 1].YPos = (ey + cy) * 65536.0f;
            m_compilerv4.m_objectEntityPos                         = ENTITY_COUNT - 1;
            m_compilerv4.processScript(curObj.eventRSDKDraw.m_scriptCodePtr,
                                       curObj.eventRSDKDraw.m_jumpTablePtr,
                                       Compilerv4::EVENT_RSDKDRAW);
            flag = true;
        }*/

        if (!flag) {
            // Draw Selected Object Preview
            float xpos = ex;
            float ypos = ey;
            float zpos = 15.0f;

            int w = objectSprites[0].texturePtr->width(), h = objectSprites[0].texturePtr->height();
            objectSprites[0].texturePtr->bind();

            QMatrix4x4 matModel;
            matModel.scale(w * zoom, h * zoom, 1.0f);

            matModel.translate(xpos / (float)w, ypos / (float)h, zpos);
            spriteShader.setValue("model", matModel);

            f->glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    spriteShader.setValue("useAlpha", false);
    spriteShader.setValue("alpha", 1.0f);

    // Selected Ent Box
    rectVAO.bind();
    primitiveShader.use();
    primitiveShader.setValue("projection", getProjectionMatrix());
    primitiveShader.setValue("view", QMatrix4x4());
    primitiveShader.setValue("useAlpha", false);
    primitiveShader.setValue("alpha", 1.0f);
    QMatrix4x4 matModel;
    primitiveShader.setValue("model", matModel);
    if (selectedEntity >= 0) {
        SceneEntity &entity = entities[selectedEntity];
        int w = objectSprites[0].texturePtr->width(), h = objectSprites[0].texturePtr->height();
        objectSprites[0].texturePtr->bind();

        drawRect(((entity.pos.x - cam.pos.x) - (w / 2)) * zoom,
                 ((entity.pos.y - cam.pos.y) - (h / 2)) * zoom, 15.7f, w * zoom, h * zoom,
                 Vector4<float>(1.0f, 1.0f, 1.0f, 1.0f), primitiveShader, true);
    }

    if (showTileGrid) {
        rectVAO.bind();

        float camX = cam.pos.x;
        float camY = cam.pos.y;

        for (int y = camY - ((int)camY % 0x10); y < (camY + storedH) * (zoom < 1.0f ? invZoom() : 1.0f);
             y += 0x10) {
            drawLine((camX - camX) * zoom, (y - camY) * zoom, 15.6f,
                     (((camX + storedW * invZoom())) - camX) * zoom, (y - camY) * zoom, 15.6f,
                     Vector4<float>(1.0f, 1.0f, 1.0f, 1.0f), primitiveShader);
        }

        for (int x = camX - ((int)camX % 0x10); x < (camX + storedW) * (zoom < 1.0f ? invZoom() : 1.0f);
             x += 0x10) {
            drawLine((x + (zoom <= 1.0f ? 1.0f : 0.0f) - camX) * zoom, (camY - camY) * zoom, 15.6f,
                     (x + (zoom <= 1.0f ? 1.0f : 0.0f) - camX) * zoom,
                     (((camY + storedH * invZoom())) - camY) * zoom, 15.6f,
                     Vector4<float>(1.0f, 1.0f, 1.0f, 1.0f), primitiveShader);
        }
    }

    if (showPixelGrid && zoom >= 4.0f) {
        // f->glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        rectVAO.bind();
        QList<QVector3D> verts;
        primitiveShader.use();
        primitiveShader.setValue("colour", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));

        float camX = cam.pos.x;
        float camY = cam.pos.y;

        for (int y = camY; y < (camY + storedH) * (zoom < 1.0f ? invZoom() : 1.0f); ++y) {
            verts.append(QVector3D((camX - camX) * zoom, (y - camY) * zoom, 15.6f));
            verts.append(
                QVector3D((((camX + storedW * invZoom())) - camX) * zoom, (y - camY) * zoom, 15.6f));
        }

        for (int x = camX; x < (camX + storedW) * (zoom < 1.0f ? invZoom() : 1.0f); ++x) {
            verts.append(QVector3D((x + (zoom <= 1.0f ? 1.0f : 0.0f) - camX) * zoom,
                                   (camY - camY) * zoom, 15.6f));
            verts.append(QVector3D((x + (zoom <= 1.0f ? 1.0f : 0.0f) - camX) * zoom,
                                   (((camY + storedH * invZoom())) - camY) * zoom, 15.6f));
        }

        QVector3D *vertsPtr = new QVector3D[(uint)verts.count()];
        for (int v = 0; v < verts.count(); ++v) vertsPtr[v] = verts[v];

        QOpenGLVertexArrayObject gridVAO;
        gridVAO.create();
        gridVAO.bind();

        QOpenGLBuffer gridVBO;
        gridVBO.create();
        gridVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
        gridVBO.bind();
        gridVBO.allocate(vertsPtr, verts.count() * sizeof(QVector3D));
        primitiveShader.enableAttributeArray(0);
        primitiveShader.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

        f->glDrawArrays(GL_LINES, 0, verts.count());

        delete[] vertsPtr;

        gridVAO.release();
        gridVBO.release();
    }
}

void SceneViewerv5::unloadScene()
{
    // QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if (m_tilesetTexture) {
        m_tilesetTexture->destroy();
        delete m_tilesetTexture;
    }
    m_tilesetTexture = nullptr;
    tiles.clear();

    for (int o = 0; o < objectSprites.count(); ++o) {
        objectSprites[o].texturePtr->destroy();
        delete objectSprites[o].texturePtr;
        objectSprites[o].name = "";
    }
    objectSprites.clear();

    if (m_rsPlayerSprite) {
        m_rsPlayerSprite->destroy();
        delete m_rsPlayerSprite;
    }

    cam                = SceneCamerav5();
    selectedTile       = -1;
    selectedEntity     = -1;
    selectedLayer      = -1;
    selectedScrollInfo = -1;
    selectedObject     = -1;
    isSelecting        = false;
}

void SceneViewerv5::initializeGL()
{
    // QOpenGLFunctions::initializeOpenGLFunctions();

    // Set up the rendering context, load shaders and other resources, etc.
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    QOpenGLContext *glContext = QOpenGLContext::currentContext();
    QSurfaceFormat fmt        = glContext->format();
    qDebug() << "Widget Using OpenGL " + QString::number(fmt.majorVersion()) + "."
                    + QString::number(fmt.minorVersion());

    const unsigned char *vendor     = f->glGetString(GL_VENDOR);
    const unsigned char *renderer   = f->glGetString(GL_RENDERER);
    const unsigned char *version    = f->glGetString(GL_VERSION);
    const unsigned char *sdrVersion = f->glGetString(GL_SHADING_LANGUAGE_VERSION);
    const unsigned char *extensions = f->glGetString(GL_EXTENSIONS);

    QString vendorStr     = reinterpret_cast<const char *>(vendor);
    QString rendererStr   = reinterpret_cast<const char *>(renderer);
    QString versionStr    = reinterpret_cast<const char *>(version);
    QString sdrVersionStr = reinterpret_cast<const char *>(sdrVersion);
    QString extensionsStr = reinterpret_cast<const char *>(extensions);

    qDebug() << "OpenGL Details";
    qDebug() << "Vendor:       " + vendorStr;
    qDebug() << "Renderer:     " + rendererStr;
    qDebug() << "Version:      " + versionStr;
    qDebug() << "GLSL version: " + sdrVersionStr;
    qDebug() << "Extensions:   " + extensionsStr;
    qDebug() << (QOpenGLContext::currentContext()->isOpenGLES() ? "Using OpenGLES" : "Using OpenGL");
    qDebug() << (QOpenGLContext::currentContext()->isValid() ? "Is valid" : "Not valid");

    f->glEnable(GL_DEPTH_TEST);
    f->glDepthFunc(GL_LESS);
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    primitiveShader.loadShader(":/shaders/primitive.glv", QOpenGLShader::Vertex);
    primitiveShader.loadShader(":/shaders/primitive.glf", QOpenGLShader::Fragment);
    primitiveShader.link();
    primitiveShader.use();

    spriteShader.loadShader(":/shaders/sprite.glv", QOpenGLShader::Vertex);
    spriteShader.loadShader(":/shaders/sprite.glf", QOpenGLShader::Fragment);
    spriteShader.link();
    spriteShader.use();

    rectVAO.create();
    rectVAO.bind();

    QOpenGLBuffer vVBO2D;
    vVBO2D.create();
    vVBO2D.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vVBO2D.bind();
    vVBO2D.allocate(rectVerticesv5, 6 * sizeof(QVector3D));
    spriteShader.enableAttributeArray(0);
    spriteShader.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

    QOpenGLBuffer tVBO2D;
    tVBO2D.create();
    tVBO2D.setUsagePattern(QOpenGLBuffer::StaticDraw);
    tVBO2D.bind();
    tVBO2D.allocate(rectTexCoordsv5, 6 * sizeof(QVector2D));
    spriteShader.enableAttributeArray(1);
    spriteShader.setAttributeBuffer(1, GL_FLOAT, 0, 2, 0);

    // Release (unbind) all
    rectVAO.release();
    vVBO2D.release();
}

void SceneViewerv5::resizeGL(int w, int h)
{
    storedW             = w;
    storedH             = h;
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(0, 0, w, h);

    // m_sbHorizontal->setMaximum((m_scene.m_sceneConfig.m_camBounds.w * 0x10) - m_storedW);
    // m_sbVertical->setMaximum((m_scene.m_sceneConfig.m_camBounds.h * 0x10) - m_storedH);
}

void SceneViewerv5::paintGL()
{
    // Draw the scene:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(bgColour.r / 255.0f, bgColour.g / 255.0f, bgColour.b / 255.0f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene();
}

int SceneViewerv5::addGraphicsFile(char *sheetPath)
{
    QString path = dataPath + "/Sprites/" + sheetPath;
    if (!QFile::exists(path))
        return 0;

    for (int i = 1; i < objectSprites.count(); ++i) {
        if (QString(sheetPath) == objectSprites[i].name)
            return i;
    }

    int sheetID = -1;
    for (int i = 1; i < objectSprites.count(); ++i) {
        if (objectSprites[i].name == "")
            sheetID = i;
    }

    if (sheetID >= 0) {
        QImage sheet(path);
        TextureInfo tex;
        tex.name               = QString(sheetPath);
        tex.texturePtr         = createTexture(sheet);
        objectSprites[sheetID] = tex;
        return sheetID;
    }
    else {
        QImage sheet(path);
        int cnt = objectSprites.count();
        TextureInfo tex;
        tex.name       = QString(sheetPath);
        tex.texturePtr = createTexture(sheet);
        objectSprites.append(tex);
        return cnt;
    }
}

void SceneViewerv5::removeGraphicsFile(char *sheetPath, int slot)
{
    if (slot >= 0) {
        objectSprites[slot].texturePtr->destroy();
        delete objectSprites[slot].texturePtr;
        objectSprites[slot].name = "";
    }
    else {
        for (int i = 1; i < objectSprites.count(); ++i) {
            if (QString(sheetPath) == objectSprites[i].name) {
                objectSprites[slot].texturePtr->destroy();
                delete objectSprites[slot].texturePtr;
                objectSprites[slot].name = "";
            }
        }
    }
}

void SceneViewerv5::drawTile(float XPos, float YPos, float ZPos, int tileX, int tileY, byte direction)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    // Draw Sprite
    float w = m_tilesetTexture->width(), h = m_tilesetTexture->height();

    spriteShader.use();
    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    float tx = tileX / w;
    float ty = tileY / h;
    float tw = 0x10 / w;
    float th = 0x10 / h;

    QVector2D *texCoords = nullptr;
    switch (direction) {
        case 0:
        default: {
            QVector2D tc[] = {
                QVector2D(tx, ty),           QVector2D(tx + tw, ty), QVector2D(tx + tw, ty + th),
                QVector2D(tx + tw, ty + th), QVector2D(tx, ty + th), QVector2D(tx, ty),
            };
            texCoords = tc;
            break;
        }
        case 1: {
            QVector2D tc[] = {
                QVector2D(tx + tw, ty), QVector2D(tx, ty),           QVector2D(tx, ty + th),
                QVector2D(tx, ty + th), QVector2D(tx + tw, ty + th), QVector2D(tx + tw, ty),
            };
            texCoords = tc;
            break;
        }
        case 2: {
            QVector2D tc[] = {
                QVector2D(tx, ty + th), QVector2D(tx + tw, ty + th), QVector2D(tx + tw, ty),
                QVector2D(tx + tw, ty), QVector2D(tx, ty),           QVector2D(tx, ty + th),
            };
            texCoords = tc;
            break;
        }
        case 3: {
            QVector2D tc[] = {
                QVector2D(tx + tw, ty + th), QVector2D(tx, ty + th), QVector2D(tx, ty),
                QVector2D(tx, ty),           QVector2D(tx + tw, ty), QVector2D(tx + tw, ty + th),
            };
            texCoords = tc;
            break;
        }
    }

    QOpenGLBuffer vVBO2D;
    vVBO2D.create();
    vVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    vVBO2D.bind();
    vVBO2D.allocate(rectVerticesv5, 6 * sizeof(QVector3D));
    spriteShader.enableAttributeArray(0);
    spriteShader.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

    QOpenGLBuffer tVBO2D;
    tVBO2D.create();
    tVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    tVBO2D.bind();
    tVBO2D.allocate(texCoords, 6 * sizeof(QVector2D));
    spriteShader.enableAttributeArray(1);
    spriteShader.setAttributeBuffer(1, GL_FLOAT, 0, 2, 0);

    QMatrix4x4 matModel;
    matModel.scale(0x10 * zoom, 0x10 * zoom, 1.0f);

    matModel.translate((XPos + (0x10 / 2)) / (float)0x10, (YPos + (0x10 / 2)) / (float)0x10, ZPos);
    spriteShader.setValue("model", matModel);

    f->glDrawArrays(GL_TRIANGLES, 0, 6);

    vao.release();
    tVBO2D.destroy();
    vVBO2D.destroy();
    vao.destroy();
}

int sprDrawsv5 = 0;
void SceneViewerv5::drawSprite(int XPos, int YPos, int width, int height, int sprX, int sprY,
                               int sheetID)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if (sheetID != m_prevSprite)
        sprDrawsv5 = 0;

    // Draw Sprite
    float xpos = XPos - cam.pos.x;
    float ypos = YPos - cam.pos.y;
    float zpos = 10.0f + (sprDrawsv5 * 0.001f);

    if (sheetID != m_prevSprite) {
        objectSprites[sheetID].texturePtr->bind();
        m_prevSprite = sheetID;
        sprDrawsv5   = 0;
    }
    float w = objectSprites[sheetID].texturePtr->width(),
          h = objectSprites[sheetID].texturePtr->height();

    Rect<int> check = Rect<int>();
    check.x         = (int)(xpos + (float)w) * zoom;
    check.y         = (int)(ypos + (float)h) * zoom;
    check.w         = (int)(xpos - (w / 2.0f)) * zoom;
    check.h         = (int)(ypos - (h / 2.0f)) * zoom;
    if (check.x < 0 || check.y < 0 || check.w >= storedW || check.h >= storedH || !sheetID) {
        return;
    }

    spriteShader.use();
    spriteShader.setValue("flipX", false);
    spriteShader.setValue("flipY", false);
    spriteShader.setValue("useAlpha", false);
    spriteShader.setValue("alpha", 1.0f);
    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    double tx = sprX / w;
    double ty = sprY / h;
    double tw = (sprX + width) / w;
    double th = (sprY + height) / h;

    const QVector2D texCoords[] = {
        QVector2D(tx, ty), QVector2D(tw, ty), QVector2D(tw, th),
        QVector2D(tw, th), QVector2D(tx, th), QVector2D(tx, ty),
    };

    QOpenGLBuffer vVBO2D;
    vVBO2D.create();
    vVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    vVBO2D.bind();
    vVBO2D.allocate(rectVerticesv5, 6 * sizeof(QVector3D));
    spriteShader.enableAttributeArray(0);
    spriteShader.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

    QOpenGLBuffer tVBO2D;
    tVBO2D.create();
    tVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    tVBO2D.bind();
    tVBO2D.allocate(texCoords, 6 * sizeof(QVector2D));
    spriteShader.enableAttributeArray(1);
    spriteShader.setAttributeBuffer(1, GL_FLOAT, 0, 2, 0);

    QMatrix4x4 matModel;
    matModel.scale(width * zoom, height * zoom, 1.0f);

    matModel.translate((xpos + (width / 2)) / (float)width, (ypos + (height / 2)) / (float)height,
                       zpos);
    spriteShader.setValue("model", matModel);

    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    sprDrawsv5++;
}

void SceneViewerv5::drawSpriteFlipped(int XPos, int YPos, int width, int height, int sprX, int sprY,
                                      int direction, int sheetID)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if (sheetID != m_prevSprite)
        sprDrawsv5 = 0;

    // Draw Sprite
    float xpos = XPos - cam.pos.x;
    float ypos = YPos - cam.pos.y;
    float zpos = 10.0f + (sprDrawsv5 * 0.001f);

    if (sheetID != m_prevSprite) {
        objectSprites[sheetID].texturePtr->bind();
        m_prevSprite = sheetID;
    }
    float w = objectSprites[sheetID].texturePtr->width(),
          h = objectSprites[sheetID].texturePtr->height();

    Rect<int> check = Rect<int>();
    check.x         = (int)(xpos + (float)w) * zoom;
    check.y         = (int)(ypos + (float)h) * zoom;
    check.w         = (int)(xpos - (w / 2.0f)) * zoom;
    check.h         = (int)(ypos - (h / 2.0f)) * zoom;
    if (check.x < 0 || check.y < 0 || check.w >= storedW || check.h >= storedH || !sheetID) {
        return;
    }

    spriteShader.use();
    spriteShader.setValue("flipX", false);
    spriteShader.setValue("flipY", false);
    spriteShader.setValue("useAlpha", false);
    spriteShader.setValue("alpha", 1.0f);
    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    float tx = sprX / w;
    float ty = sprY / h;
    float tw = width / w;
    float th = height / h;

    QVector2D *texCoords = nullptr;
    switch (direction) {
        case 0:
        default: {
            QVector2D tc[] = {
                QVector2D(tx, ty),           QVector2D(tx + tw, ty), QVector2D(tx + tw, ty + th),
                QVector2D(tx + tw, ty + th), QVector2D(tx, ty + th), QVector2D(tx, ty),
            };
            texCoords = tc;
            break;
        }
        case 1: {
            QVector2D tc[] = {
                QVector2D(tx + tw, ty), QVector2D(tx, ty),           QVector2D(tx, ty + th),
                QVector2D(tx, ty + th), QVector2D(tx + tw, ty + th), QVector2D(tx + tw, ty),
            };
            texCoords = tc;
            break;
        }
        case 2: {
            QVector2D tc[] = {
                QVector2D(tx, ty + th), QVector2D(tx + tw, ty + th), QVector2D(tx + tw, ty),
                QVector2D(tx + tw, ty), QVector2D(tx, ty),           QVector2D(tx, ty + th),
            };
            texCoords = tc;
            break;
        }
        case 3: {
            QVector2D tc[] = {
                QVector2D(tx + tw, ty + th), QVector2D(tx, ty + th), QVector2D(tx, ty),
                QVector2D(tx, ty),           QVector2D(tx + tw, ty), QVector2D(tx + tw, ty + th),
            };
            texCoords = tc;
            break;
        }
    }

    QOpenGLBuffer vVBO2D;
    vVBO2D.create();
    vVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    vVBO2D.bind();
    vVBO2D.allocate(rectVerticesv5, 6 * sizeof(QVector3D));
    spriteShader.enableAttributeArray(0);
    spriteShader.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

    QOpenGLBuffer tVBO2D;
    tVBO2D.create();
    tVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    tVBO2D.bind();
    tVBO2D.allocate(texCoords, 6 * sizeof(QVector2D));
    spriteShader.enableAttributeArray(1);
    spriteShader.setAttributeBuffer(1, GL_FLOAT, 0, 2, 0);

    QMatrix4x4 matModel;
    matModel.scale(width * zoom, height * zoom, 1.0f);

    matModel.translate((xpos + (width / 2)) / (float)width, (ypos + (height / 2)) / (float)height,
                       zpos);
    spriteShader.setValue("model", matModel);

    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    sprDrawsv5++;
}

void SceneViewerv5::drawBlendedSprite(int XPos, int YPos, int width, int height, int sprX, int sprY,
                                      int sheetID)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if (sheetID != m_prevSprite)
        sprDrawsv5 = 0;

    // Draw Sprite
    float xpos = XPos - cam.pos.x;
    float ypos = YPos - cam.pos.y;
    float zpos = 10.0f + (sprDrawsv5 * 0.001f);

    if (sheetID != m_prevSprite) {
        objectSprites[sheetID].texturePtr->bind();
        m_prevSprite = sheetID;
    }
    float w = objectSprites[sheetID].texturePtr->width(),
          h = objectSprites[sheetID].texturePtr->height();

    Rect<int> check = Rect<int>();
    check.x         = (int)(xpos + (float)w) * zoom;
    check.y         = (int)(ypos + (float)h) * zoom;
    check.w         = (int)(xpos - (w / 2.0f)) * zoom;
    check.h         = (int)(ypos - (h / 2.0f)) * zoom;
    if (check.x < 0 || check.y < 0 || check.w >= storedW || check.h >= storedH || !sheetID) {
        return;
    }

    spriteShader.use();
    spriteShader.setValue("flipX", false);
    spriteShader.setValue("flipY", false);
    spriteShader.setValue("useAlpha", true);
    spriteShader.setValue("alpha", 0.5f);
    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    float tx = sprX / w;
    float ty = sprY / h;
    float tw = width / w;
    float th = height / h;

    const QVector2D texCoords[] = {
        QVector2D(tx, ty),           QVector2D(tx + tw, ty), QVector2D(tx + tw, ty + th),
        QVector2D(tx + tw, ty + th), QVector2D(tx, ty + th), QVector2D(tx, ty),
    };

    QOpenGLBuffer vVBO2D;
    vVBO2D.create();
    vVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    vVBO2D.bind();
    vVBO2D.allocate(rectVerticesv5, 6 * sizeof(QVector3D));
    spriteShader.enableAttributeArray(0);
    spriteShader.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

    QOpenGLBuffer tVBO2D;
    tVBO2D.create();
    tVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    tVBO2D.bind();
    tVBO2D.allocate(texCoords, 6 * sizeof(QVector2D));
    spriteShader.enableAttributeArray(1);
    spriteShader.setAttributeBuffer(1, GL_FLOAT, 0, 2, 0);

    QMatrix4x4 matModel;
    matModel.scale(width * zoom, height * zoom, 1.0f);

    matModel.translate((xpos + (width / 2)) / (float)width, (ypos + (height / 2)) / (float)height,
                       zpos);
    spriteShader.setValue("model", matModel);

    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    sprDrawsv5++;
}

void SceneViewerv5::drawAlphaBlendedSprite(int XPos, int YPos, int width, int height, int sprX,
                                           int sprY, int alpha, int sheetID)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if (sheetID != m_prevSprite)
        sprDrawsv5 = 0;

    // Draw Sprite
    float xpos = XPos - cam.pos.x;
    float ypos = YPos - cam.pos.y;
    float zpos = 10.0f + (sprDrawsv5 * 0.001f);

    if (sheetID != m_prevSprite) {
        objectSprites[sheetID].texturePtr->bind();
        m_prevSprite = sheetID;
    }
    float w = objectSprites[sheetID].texturePtr->width(),
          h = objectSprites[sheetID].texturePtr->height();

    Rect<int> check = Rect<int>();
    check.x         = (int)(xpos + (float)w) * zoom;
    check.y         = (int)(ypos + (float)h) * zoom;
    check.w         = (int)(xpos - (w / 2.0f)) * zoom;
    check.h         = (int)(ypos - (h / 2.0f)) * zoom;
    if (check.x < 0 || check.y < 0 || check.w >= storedW || check.h >= storedH || !sheetID) {
        return;
    }

    spriteShader.use();
    spriteShader.setValue("flipX", false);
    spriteShader.setValue("flipY", false);
    spriteShader.setValue("useAlpha", true);
    spriteShader.setValue("alpha", (alpha > 0xFF ? 0xFF : alpha) / 255.0f);
    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    float tx = sprX / w;
    float ty = sprY / h;
    float tw = width / w;
    float th = height / h;

    const QVector2D texCoords[] = {
        QVector2D(tx, ty),           QVector2D(tx + tw, ty), QVector2D(tx + tw, ty + th),
        QVector2D(tx + tw, ty + th), QVector2D(tx, ty + th), QVector2D(tx, ty),
    };

    QOpenGLBuffer vVBO2D;
    vVBO2D.create();
    vVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    vVBO2D.bind();
    vVBO2D.allocate(rectVerticesv5, 6 * sizeof(QVector3D));
    spriteShader.enableAttributeArray(0);
    spriteShader.setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

    QOpenGLBuffer tVBO2D;
    tVBO2D.create();
    tVBO2D.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    tVBO2D.bind();
    tVBO2D.allocate(texCoords, 6 * sizeof(QVector2D));
    spriteShader.enableAttributeArray(1);
    spriteShader.setAttributeBuffer(1, GL_FLOAT, 0, 2, 0);

    QMatrix4x4 matModel;
    matModel.scale(width * zoom, height * zoom, 1.0f);

    matModel.translate((xpos + (width / 2)) / (float)width, (ypos + (height / 2)) / (float)height,
                       zpos);
    spriteShader.setValue("model", matModel);

    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    sprDrawsv5++;
}
