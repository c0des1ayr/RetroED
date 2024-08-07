#include "libRSDK.hpp"

#include "chunks.hpp"
#include "RSDKv1/chunksv1.hpp"
#include "RSDKv2/chunksv2.hpp"
#include "RSDKv3/chunksv3.hpp"
#include "RSDKv4/chunksv4.hpp"

void FormatHelpers::Chunks::read(byte ver, QString filename)
{
    Reader reader(filename);
    filePath = filename;

    for (int c = 0; c < 0x200; ++c) {
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                chunks[c].tiles[y][x].direction   = 0;
                chunks[c].tiles[y][x].visualPlane = 0;
                chunks[c].tiles[y][x].tileIndex   = 0;
                chunks[c].tiles[y][x].solidityA   = 3;
                chunks[c].tiles[y][x].solidityB   = 3;
            }
        }
    }

    switch (ver) {
        default: break;
        case ENGINE_v1: {
            RSDKv1::Chunks *engineChunks = new RSDKv1::Chunks(reader);
            for (int c = 0; c < 0x100; ++c) {
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 8; ++x) {
                        chunks[c].tiles[y][x].direction =
                            engineChunks->chunkList[c].tiles[y][x].direction;
                        chunks[c].tiles[y][x].visualPlane =
                            engineChunks->chunkList[c].tiles[y][x].visualPlane;
                        chunks[c].tiles[y][x].tileIndex =
                            engineChunks->chunkList[c].tiles[y][x].tileIndex;
                        chunks[c].tiles[y][x].solidityA =
                            engineChunks->chunkList[c].tiles[y][x].solidityA;
                        chunks[c].tiles[y][x].solidityB =
                            engineChunks->chunkList[c].tiles[y][x].solidityB;
                    }
                }
            }

            delete engineChunks;
            break;
        }
        case ENGINE_v2: {
            RSDKv2::Chunks *engineChunks = new RSDKv2::Chunks(reader);
            for (int c = 0; c < 0x200; ++c) {
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 8; ++x) {
                        chunks[c].tiles[y][x].direction =
                            engineChunks->chunkList[c].tiles[y][x].direction;
                        chunks[c].tiles[y][x].visualPlane =
                            engineChunks->chunkList[c].tiles[y][x].visualPlane;
                        chunks[c].tiles[y][x].tileIndex =
                            engineChunks->chunkList[c].tiles[y][x].tileIndex;
                        chunks[c].tiles[y][x].solidityA =
                            engineChunks->chunkList[c].tiles[y][x].solidityA;
                        chunks[c].tiles[y][x].solidityB =
                            engineChunks->chunkList[c].tiles[y][x].solidityB;
                    }
                }
            }

            delete engineChunks;
            break;
        }
        case ENGINE_v3: {
            RSDKv3::Chunks *engineChunks = new RSDKv3::Chunks(reader);
            for (int c = 0; c < 0x200; ++c) {
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 8; ++x) {
                        chunks[c].tiles[y][x].direction =
                            engineChunks->chunkList[c].tiles[y][x].direction;
                        chunks[c].tiles[y][x].visualPlane =
                            engineChunks->chunkList[c].tiles[y][x].visualPlane;
                        chunks[c].tiles[y][x].tileIndex =
                            engineChunks->chunkList[c].tiles[y][x].tileIndex;
                        chunks[c].tiles[y][x].solidityA =
                            engineChunks->chunkList[c].tiles[y][x].solidityA;
                        chunks[c].tiles[y][x].solidityB =
                            engineChunks->chunkList[c].tiles[y][x].solidityB;
                    }
                }
            }

            delete engineChunks;
            break;
        }
        case ENGINE_v4: {
            RSDKv4::Chunks *engineChunks = new RSDKv4::Chunks(reader);
            for (int c = 0; c < 0x200; ++c) {
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 8; ++x) {
                        chunks[c].tiles[y][x].direction =
                            engineChunks->chunkList[c].tiles[y][x].direction;
                        chunks[c].tiles[y][x].visualPlane =
                            engineChunks->chunkList[c].tiles[y][x].visualPlane;
                        chunks[c].tiles[y][x].tileIndex =
                            engineChunks->chunkList[c].tiles[y][x].tileIndex;
                        chunks[c].tiles[y][x].solidityA =
                            engineChunks->chunkList[c].tiles[y][x].solidityA;
                        chunks[c].tiles[y][x].solidityB =
                            engineChunks->chunkList[c].tiles[y][x].solidityB;
                    }
                }
            }

            delete engineChunks;
            break;
        }
    }
}

void FormatHelpers::Chunks::write(byte ver, QString filename)
{
    if (filename == "")
        filename = filePath;
    if (filename == "")
        return;
    Writer writer(filename);
    filePath = filename;

    switch (ver) {
        default: break;
        case ENGINE_v1: {
            RSDKv1::Chunks *engineChunks = new RSDKv1::Chunks;
            for (int c = 0; c < 0x100; ++c) {
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 8; ++x) {
                        engineChunks->chunkList[c].tiles[y][x].direction =
                            chunks[c].tiles[y][x].direction;
                        engineChunks->chunkList[c].tiles[y][x].visualPlane =
                            chunks[c].tiles[y][x].visualPlane;
                        engineChunks->chunkList[c].tiles[y][x].tileIndex =
                            chunks[c].tiles[y][x].tileIndex;
                        engineChunks->chunkList[c].tiles[y][x].solidityA =
                            chunks[c].tiles[y][x].solidityA;
                        engineChunks->chunkList[c].tiles[y][x].solidityB =
                            chunks[c].tiles[y][x].solidityB;

                        if (engineChunks->chunkList[c].tiles[y][x].solidityA == 4)
                            engineChunks->chunkList[c].tiles[y][x].solidityA = 1;
                        if (engineChunks->chunkList[c].tiles[y][x].solidityB == 4)
                            engineChunks->chunkList[c].tiles[y][x].solidityB = 1;
                    }
                }
            }
            engineChunks->write(writer);

            delete engineChunks;
            break;
        }
        case ENGINE_v2: {
            RSDKv2::Chunks *engineChunks = new RSDKv2::Chunks;
            for (int c = 0; c < 0x200; ++c) {
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 8; ++x) {
                        engineChunks->chunkList[c].tiles[y][x].direction =
                            chunks[c].tiles[y][x].direction;
                        engineChunks->chunkList[c].tiles[y][x].visualPlane =
                            chunks[c].tiles[y][x].visualPlane;
                        engineChunks->chunkList[c].tiles[y][x].tileIndex =
                            chunks[c].tiles[y][x].tileIndex;
                        engineChunks->chunkList[c].tiles[y][x].solidityA =
                            chunks[c].tiles[y][x].solidityA;
                        engineChunks->chunkList[c].tiles[y][x].solidityB =
                            chunks[c].tiles[y][x].solidityB;

                        if (engineChunks->chunkList[c].tiles[y][x].solidityA == 4)
                            engineChunks->chunkList[c].tiles[y][x].solidityA = 1;
                        if (engineChunks->chunkList[c].tiles[y][x].solidityB == 4)
                            engineChunks->chunkList[c].tiles[y][x].solidityB = 1;
                    }
                }
            }
            engineChunks->write(writer);

            delete engineChunks;
            break;
        }
        case ENGINE_v3: {
            RSDKv3::Chunks *engineChunks = new RSDKv3::Chunks;
            for (int c = 0; c < 0x200; ++c) {
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 8; ++x) {
                        engineChunks->chunkList[c].tiles[y][x].direction =
                            chunks[c].tiles[y][x].direction;
                        engineChunks->chunkList[c].tiles[y][x].visualPlane =
                            chunks[c].tiles[y][x].visualPlane;
                        engineChunks->chunkList[c].tiles[y][x].tileIndex =
                            chunks[c].tiles[y][x].tileIndex;
                        engineChunks->chunkList[c].tiles[y][x].solidityA =
                            chunks[c].tiles[y][x].solidityA;
                        engineChunks->chunkList[c].tiles[y][x].solidityB =
                            chunks[c].tiles[y][x].solidityB;

                        if (engineChunks->chunkList[c].tiles[y][x].solidityA == 4)
                            engineChunks->chunkList[c].tiles[y][x].solidityA = 1;
                        if (engineChunks->chunkList[c].tiles[y][x].solidityB == 4)
                            engineChunks->chunkList[c].tiles[y][x].solidityB = 1;
                    }
                }
            }
            engineChunks->write(writer);

            delete engineChunks;
            break;
        }
        case ENGINE_v4: {
            RSDKv4::Chunks *engineChunks = new RSDKv4::Chunks;
            for (int c = 0; c < 0x200; ++c) {
                for (int y = 0; y < 8; ++y) {
                    for (int x = 0; x < 8; ++x) {
                        engineChunks->chunkList[c].tiles[y][x].direction =
                            chunks[c].tiles[y][x].direction;
                        engineChunks->chunkList[c].tiles[y][x].visualPlane =
                            chunks[c].tiles[y][x].visualPlane;
                        engineChunks->chunkList[c].tiles[y][x].tileIndex =
                            chunks[c].tiles[y][x].tileIndex;
                        engineChunks->chunkList[c].tiles[y][x].solidityA =
                            chunks[c].tiles[y][x].solidityA;
                        engineChunks->chunkList[c].tiles[y][x].solidityB =
                            chunks[c].tiles[y][x].solidityB;
                    }
                }
            }
            engineChunks->write(writer);

            delete engineChunks;
            break;
        }
    }
}

QImage FormatHelpers::Chunks::Chunk::getImage(QList<QImage> tileList)
{
    QImage img(0x80, 0x80, QImage::Format::Format_ARGB32);

    for (int h = 0; h < 0x8; ++h) {
        for (int w = 0; w < 0x8; ++w) {
            QImage &tile = tileList[tiles[h][w].tileIndex];
            Vector2<bool> flip;
            flip.x = (tiles[h][w].direction & 1) == 1;
            flip.y = (tiles[h][w].direction & 2) == 2;

            for (int y = 0; y < 0x10; ++y) {
                for (int x = 0; x < 0x10; ++x) {
                    int xpos = x;
                    int ypos = y;
                    if (flip.y)
                        ypos = 0x0F - y;
                    if (flip.x)
                        xpos = 0x0F - x;

                    img.setPixelColor((w * 0x10) + x, (h * 0x10) + y,
                                      tile.pixelColor(xpos, ypos).rgb());
                }
            }
        }
    }

    return img;
}
