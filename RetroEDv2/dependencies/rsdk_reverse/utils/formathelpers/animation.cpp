#include "include.hpp"

void FormatHelpers::Animation::read(byte ver, QString filename)
{
    Reader reader(filename);
    filePath = filename;

    sheets.clear();
    animations.clear();
    hitboxes.clear();
    hitboxTypes.clear();

    switch (ver) {
        default: break;
        case ENGINE_v1: {
            RSDKv1::Animation animFile(filename);

            playerType = animFile.playerType;
            unknown2   = animFile.unknown;

            for (auto &s : animFile.sheets) sheets.append(s);

            for (auto &a : animFile.animations) {
                AnimationEntry animation;
                animation.name          = a.name;
                animation.speed         = a.speed;
                animation.loopIndex     = a.loopIndex;
                animation.rotationStyle = 1;

                for (auto &f : a.frames) {
                    Frame frame;
                    frame.sheet        = f.sheet;
                    frame.collisionBox = -1;
                    frame.duration     = 0x100;
                    frame.id           = 0;
                    frame.sprX         = f.sprX;
                    frame.sprY         = f.sprY;
                    frame.width        = f.width;
                    frame.height       = f.height;
                    frame.pivotX       = f.pivotX;
                    frame.pivotY       = f.pivotY;

                    Hitbox::HitboxInfo hitbox;
                    hitbox.left   = f.hitbox.left;
                    hitbox.top    = f.hitbox.top;
                    hitbox.right  = f.hitbox.right;
                    hitbox.bottom = f.hitbox.bottom;
                    frame.hitboxes.append(hitbox);

                    animation.frames.append(frame);
                }
                animations.append(animation);
            }

            hitboxTypes.append("Hitbox");
            hitboxes.append(Hitbox());
            break;
        }
        case ENGINE_v2: {
            RSDKv2::Animation animFile(filename);

            for (int i = 0; i < 5; ++i) unknown[i] = animFile.unknown[i];

            for (auto &s : animFile.sheets) sheets.append(s);

            for (auto &a : animFile.animations) {
                AnimationEntry animation;
                animation.name          = a.name;
                animation.speed         = a.speedMultiplyer;
                animation.loopIndex     = a.loopIndex;
                animation.rotationStyle = 1;

                for (auto &f : a.frames) {
                    Frame frame;
                    frame.sheet        = f.sheet;
                    frame.collisionBox = f.collisionBox;
                    frame.duration     = 0x100;
                    frame.id           = 0;
                    frame.sprX         = f.sprX;
                    frame.sprY         = f.sprY;
                    frame.width        = f.width;
                    frame.height       = f.height;
                    frame.pivotX       = f.pivotX;
                    frame.pivotY       = f.pivotY;

                    frame.hitboxes.clear();
                    animation.frames.append(frame);
                }
                animations.append(animation);
            }

            int hitboxID = 0;
            for (auto &h : animFile.hitboxes) {
                Hitbox hitboxInfo;
                for (int i = 0; i < 8; ++i) {
                    hitboxInfo.hitboxes[i].left   = h.hitboxes[i].left;
                    hitboxInfo.hitboxes[i].top    = h.hitboxes[i].top;
                    hitboxInfo.hitboxes[i].right  = h.hitboxes[i].right;
                    hitboxInfo.hitboxes[i].bottom = h.hitboxes[i].bottom;
                }
                hitboxes.append(hitboxInfo);
                hitboxTypes.append("Hitbox " + QString::number(hitboxID++));
            }
            break;
        }
        case ENGINE_v3: {
            RSDKv3::Animation animFile(filename);
            for (auto &s : animFile.sheets) sheets.append(s);

            for (auto &a : animFile.animations) {
                AnimationEntry animation;
                animation.name          = a.name;
                animation.speed         = a.speedMultiplyer;
                animation.loopIndex     = a.loopIndex;
                animation.rotationStyle = a.rotationFlags;

                for (auto &f : a.frames) {
                    Frame frame;
                    frame.sheet        = f.sheet;
                    frame.collisionBox = f.collisionBox;
                    frame.duration     = 0x100;
                    frame.id           = 0;
                    frame.sprX         = f.sprX;
                    frame.sprY         = f.sprY;
                    frame.width        = f.width;
                    frame.height       = f.height;
                    frame.pivotX       = f.pivotX;
                    frame.pivotY       = f.pivotY;

                    frame.hitboxes.clear();
                    animation.frames.append(frame);
                }
                animations.append(animation);
            }

            int hitboxID = 0;
            for (auto &h : animFile.hitboxes) {
                Hitbox hitboxInfo;
                for (int i = 0; i < 8; ++i) {
                    hitboxInfo.hitboxes[i].left   = h.hitboxes[i].left;
                    hitboxInfo.hitboxes[i].top    = h.hitboxes[i].top;
                    hitboxInfo.hitboxes[i].right  = h.hitboxes[i].right;
                    hitboxInfo.hitboxes[i].bottom = h.hitboxes[i].bottom;
                }
                hitboxes.append(hitboxInfo);
                hitboxTypes.append("Hitbox " + QString::number(hitboxID++));
            }
            break;
        }
        case ENGINE_v4: {
            RSDKv4::Animation animFile(filename);
            for (auto &s : animFile.sheets) sheets.append(s);

            for (auto &a : animFile.animations) {
                AnimationEntry animation;
                animation.name          = a.name;
                animation.speed         = a.speedMultiplyer;
                animation.loopIndex     = a.loopIndex;
                animation.rotationStyle = a.rotationFlags;

                for (auto &f : a.frames) {
                    Frame frame;
                    frame.sheet        = f.sheet;
                    frame.collisionBox = f.collisionBox;
                    frame.duration     = 0x100;
                    frame.id           = 0;
                    frame.sprX         = f.sprX;
                    frame.sprY         = f.sprY;
                    frame.width        = f.width;
                    frame.height       = f.height;
                    frame.pivotX       = f.pivotX;
                    frame.pivotY       = f.pivotY;

                    frame.hitboxes.clear();
                    animation.frames.append(frame);
                }
                animations.append(animation);
            }

            int hitboxID = 0;
            for (auto &h : animFile.hitboxes) {
                Hitbox hitboxInfo;
                for (int i = 0; i < 8; ++i) {
                    hitboxInfo.hitboxes[i].left   = h.hitboxes[i].left;
                    hitboxInfo.hitboxes[i].top    = h.hitboxes[i].top;
                    hitboxInfo.hitboxes[i].right  = h.hitboxes[i].right;
                    hitboxInfo.hitboxes[i].bottom = h.hitboxes[i].bottom;
                }
                hitboxes.append(hitboxInfo);
                hitboxTypes.append("Hitbox " + QString::number(hitboxID++));
            }
            break;
        }
        case ENGINE_v5: {
            RSDKv5::Animation animFile(filename);
            for (auto &s : animFile.sheets) sheets.append(s);

            for (auto &a : animFile.animations) {
                AnimationEntry animation;
                animation.name          = a.name;
                animation.speed         = a.speedMultiplyer;
                animation.loopIndex     = a.loopIndex;
                animation.rotationStyle = a.rotationFlags;

                for (auto &f : a.frames) {
                    Frame frame;
                    frame.sheet        = f.sheet;
                    frame.collisionBox = -1;
                    frame.duration     = f.delay;
                    frame.id           = f.id;
                    frame.sprX         = f.sprX;
                    frame.sprY         = f.sprY;
                    frame.width        = f.width;
                    frame.height       = f.height;
                    frame.pivotX       = f.pivotX;
                    frame.pivotY       = f.pivotY;

                    for (auto &h : f.hitboxes) {
                        Hitbox::HitboxInfo hitbox;
                        hitbox.left   = h.left;
                        hitbox.top    = h.top;
                        hitbox.right  = h.right;
                        hitbox.bottom = h.bottom;
                        frame.hitboxes.append(hitbox);
                    }
                    animation.frames.append(frame);
                }
                animations.append(animation);
            }

            for (auto &h : animFile.hitboxTypes) {
                Hitbox hitbox;
                hitboxes.append(hitbox);
                hitboxTypes.append(h);
            }
            break;
        }
    }
}

void FormatHelpers::Animation::write(byte ver, QString filename)
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
            RSDKv1::Animation animFile;

            animFile.playerType = playerType;
            animFile.unknown    = unknown2;

            for (auto &s : sheets) animFile.sheets.append(s);

            for (int aID = 0; aID < animations.count(); ++aID) {
                auto &a = animations[aID];
                RSDKv1::Animation::AnimationEntry animEntry;
                animEntry.speed     = a.speed;
                animEntry.loopIndex = a.loopIndex;

                for (auto &f : a.frames) {
                    RSDKv1::Animation::Frame frame;
                    frame.sheet  = f.sheet;
                    frame.sprX   = f.sprX;
                    frame.sprY   = f.sprY;
                    frame.width  = f.width;
                    frame.height = f.height;
                    frame.pivotX = f.pivotX;
                    frame.pivotY = f.pivotY;

                    if (f.hitboxes.count()) {
                        frame.hitbox.left   = f.hitboxes[0].left;
                        frame.hitbox.top    = f.hitboxes[0].top;
                        frame.hitbox.right  = f.hitboxes[0].right;
                        frame.hitbox.bottom = f.hitboxes[0].bottom;
                    }
                    else {
                        frame.hitbox.left   = 0;
                        frame.hitbox.top    = 0;
                        frame.hitbox.right  = 0;
                        frame.hitbox.bottom = 0;
                    }

                    animEntry.frames.append(frame);
                }
                animFile.animations.append(animEntry);
            }

            animFile.write(filename);
            break;
        }
        case ENGINE_v2: {
            RSDKv2::Animation animFile;

            for (auto &s : sheets) animFile.sheets.append(s);

            for (int aID = 0; aID < animations.count(); ++aID) {
                auto &a = animations[aID];
                RSDKv2::Animation::AnimationEntry animEntry;
                animEntry.speedMultiplyer = a.speed;
                animEntry.loopIndex       = a.loopIndex;

                for (auto &f : a.frames) {
                    RSDKv2::Animation::Frame frame;
                    frame.sheet        = f.sheet;
                    frame.collisionBox = f.collisionBox;
                    frame.sprX         = f.sprX;
                    frame.sprY         = f.sprY;
                    frame.width        = f.width;
                    frame.height       = f.height;
                    frame.pivotX       = f.pivotX;
                    frame.pivotY       = f.pivotY;

                    animEntry.frames.append(frame);
                }
                animFile.animations.append(animEntry);
            }

            for (auto &h : hitboxes) {
                RSDKv2::Animation::Hitbox hitboxInfo;
                for (int i = 0; i < 8; ++i) {
                    hitboxInfo.hitboxes[i].left   = h.hitboxes[i].left;
                    hitboxInfo.hitboxes[i].top    = h.hitboxes[i].top;
                    hitboxInfo.hitboxes[i].right  = h.hitboxes[i].right;
                    hitboxInfo.hitboxes[i].bottom = h.hitboxes[i].bottom;
                }
                animFile.hitboxes.append(hitboxInfo);
            }

            animFile.write(filename);
            break;
        }
        case ENGINE_v3: {
            RSDKv3::Animation animFile;

            for (auto &s : sheets) animFile.sheets.append(s);

            for (int aID = 0; aID < animations.count(); ++aID) {
                auto &a = animations[aID];
                RSDKv3::Animation::AnimationEntry animEntry;
                animEntry.name            = a.name;
                animEntry.speedMultiplyer = a.speed;
                animEntry.loopIndex       = a.loopIndex;
                animEntry.rotationFlags   = a.rotationStyle;

                for (auto &f : a.frames) {
                    RSDKv3::Animation::Frame frame;
                    frame.sheet        = f.sheet;
                    frame.collisionBox = f.collisionBox;
                    frame.sprX         = f.sprX;
                    frame.sprY         = f.sprY;
                    frame.width        = f.width;
                    frame.height       = f.height;
                    frame.pivotX       = f.pivotX;
                    frame.pivotY       = f.pivotY;

                    animEntry.frames.append(frame);
                }
                animFile.animations.append(animEntry);
            }

            for (auto &h : hitboxes) {
                RSDKv3::Animation::Hitbox hitboxInfo;
                for (int i = 0; i < 8; ++i) {
                    hitboxInfo.hitboxes[i].left   = h.hitboxes[i].left;
                    hitboxInfo.hitboxes[i].top    = h.hitboxes[i].top;
                    hitboxInfo.hitboxes[i].right  = h.hitboxes[i].right;
                    hitboxInfo.hitboxes[i].bottom = h.hitboxes[i].bottom;
                }
                animFile.hitboxes.append(hitboxInfo);
            }

            animFile.write(filename);
            break;
        }
        case ENGINE_v4: {
            RSDKv4::Animation animFile;

            for (auto &s : sheets) animFile.sheets.append(s);

            for (int aID = 0; aID < animations.count(); ++aID) {
                auto &a = animations[aID];
                RSDKv4::Animation::AnimationEntry animEntry;
                animEntry.name            = a.name;
                animEntry.speedMultiplyer = a.speed;
                animEntry.loopIndex       = a.loopIndex;
                animEntry.rotationFlags   = a.rotationStyle;

                for (auto &f : a.frames) {
                    RSDKv4::Animation::Frame frame;
                    frame.sheet        = f.sheet;
                    frame.collisionBox = f.collisionBox;
                    frame.sprX         = f.sprX;
                    frame.sprY         = f.sprY;
                    frame.width        = f.width;
                    frame.height       = f.height;
                    frame.pivotX       = f.pivotX;
                    frame.pivotY       = f.pivotY;

                    animEntry.frames.append(frame);
                }
                animFile.animations.append(animEntry);
            }

            for (auto &h : hitboxes) {
                RSDKv4::Animation::Hitbox hitboxInfo;
                for (int i = 0; i < 8; ++i) {
                    hitboxInfo.hitboxes[i].left   = h.hitboxes[i].left;
                    hitboxInfo.hitboxes[i].top    = h.hitboxes[i].top;
                    hitboxInfo.hitboxes[i].right  = h.hitboxes[i].right;
                    hitboxInfo.hitboxes[i].bottom = h.hitboxes[i].bottom;
                }
                animFile.hitboxes.append(hitboxInfo);
            }

            animFile.write(filename);
            break;
        }
        case ENGINE_v5: {
            RSDKv5::Animation animFile;

            for (auto &s : sheets) animFile.sheets.append(s);

            for (int aID = 0; aID < animations.count(); ++aID) {
                auto &a = animations[aID];
                RSDKv5::Animation::AnimationEntry animEntry;
                animEntry.name            = a.name;
                animEntry.speedMultiplyer = a.speed;
                animEntry.loopIndex       = a.loopIndex;
                animEntry.rotationFlags   = a.rotationStyle;

                for (auto &f : a.frames) {
                    RSDKv5::Animation::Frame frame;
                    frame.sheet  = f.sheet;
                    frame.delay  = f.duration;
                    frame.id     = f.id;
                    frame.sprX   = f.sprX;
                    frame.sprY   = f.sprY;
                    frame.width  = f.width;
                    frame.height = f.height;
                    frame.pivotX = f.pivotX;
                    frame.pivotY = f.pivotY;

                    for (int h = 0; h < hitboxTypes.count(); ++h) {
                        RSDKv5::Animation::Hitbox hitbox;
                        if (h >= f.hitboxes.count()) {
                            hitbox.left   = 0;
                            hitbox.top    = 0;
                            hitbox.right  = 0;
                            hitbox.bottom = 0;
                        }
                        else {
                            hitbox.left   = f.hitboxes[h].left;
                            hitbox.top    = f.hitboxes[h].top;
                            hitbox.right  = f.hitboxes[h].right;
                            hitbox.bottom = f.hitboxes[h].bottom;
                        }
                        frame.hitboxes.count();
                    }
                    animEntry.frames.append(frame);
                }
                animFile.animations.append(animEntry);
            }

            for (auto &h : hitboxTypes) animFile.hitboxTypes.append(h);

            animFile.write(filename);
            break;
        }
    }
}
