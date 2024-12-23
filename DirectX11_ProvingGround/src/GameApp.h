﻿#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "D3DApp.h"


class GameApp : public D3DApp
{
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float deltaTime);
    void DrawScene();
};

#endif  // GAMEAPP_H